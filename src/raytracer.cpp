#include "context.h"
#include "exceptions.h"
#include <iostream>

void Context::BeginScene() {
	if (is_drawing) {
		throw SGLInvalidOperationException("BeginScene called inside Begin-End sequence");
	}

	is_setting_scene = true;
};

void Context::EndScene() {
	if (is_drawing) {
		throw SGLInvalidOperationException("EndScene called inside Begin-End sequence");
	}

	is_setting_scene = false;
};


void Context::SetMaterial(const float r,
	const float g,
	const float b,
	const float kd,
	const float ks,
	const float shine,
	const float T,
	const float ior) 
{
	if (is_drawing) {
		throw SGLInvalidOperationException("SetMaterial called inside Begin-End sequence");
	}

	assigning_emmisive_material = false;
	materials.emplace_back(Material{r, g, b, kd, ks, shine, T, ior});
};

void Context::CreatePointLight(const float x,
	const float y,
	const float z,
	const float r,
	const float g,
	const float b) 
{
	if (is_drawing) {
		throw SGLInvalidOperationException("CreatePointLight called inside Begin-End sequence");
	}
	else if (!is_setting_scene) {
		throw SGLInvalidOperationException("CreatePointLight called outside BeginScene-EndScene sequence");
	}
	point_lights.push_back(PointLight{x, y, z, r, g, b});
};

void Context::CreateSphere(const float x,
	const float y,
	const float z,
	const float radius)
{
	if (is_drawing) {
		throw SGLInvalidOperationException("CreateSphere called inside Begin-End sequence");
	}
	else if (!is_setting_scene) {
		throw SGLInvalidOperationException("CreateSphere called outside BeginScene-EndScene sequence");
	}
	sphere_buffer.push_back(Sphere{x, y, z, radius, materials.size() - 1});
}

void Context::SetEnvironmentMap(const int width,
	const int height,
	float* texels)
{
	if (is_drawing) {
		throw SGLInvalidOperationException("SetEnvironmentMap called inside Begin-End sequence");
	}

	//the user owns the array
	environment_map = EnvironmentMap{ width, height, texels };
	environment_map_set = true;
}

void Context::SetEmissiveMaterial(const float r,
	const float g,
	const float b,
	const float c0,
	const float c1,
	const float c2)
{
	if (is_drawing) {
		throw SGLInvalidOperationException("SetEmissiveMaterial called inside Begin-End sequence");
	}

	assigning_emmisive_material = true;
	emissive_materials.emplace_back(EmissiveMaterial{r, g, b, c0, c1, c2});
}

Vec4 Context::BilinearInterpolation(
	const Vec4& bl, const Vec4& br,
	const Vec4& tl, const Vec4& tr,
	float u, float v
) {
	Vec4 bottom = (1.0f - u) * bl + u * br;
	Vec4 top = (1.0f - u) * tl + u * tr;

	return (1.0f - v) * bottom + v * top;
}

void Context::RayTraceScene() {

	if (is_drawing) {
		throw SGLInvalidOperationException("RayTraceScene called inside Begin-End sequence");
	}
	else if (is_setting_scene) {
		throw SGLInvalidOperationException("RayTraceScene called inside BeginScene-EndScene sequence");
	}

	//fetch matrices, BeginScene() does this but isn't called for
	//sphere-only scenes
	const Matrix& VM = matrix_stack.GetViewModelMatrix();
	const Matrix& P = matrix_stack.GetProjectionMatrix();

	PVM_matrix = Matrix::Matmul(P, VM);
	Vp_matrix = matrix_stack.GetViewport();

	Vec4 bottom_left = Vec4{ 0.0f, 0.0f, -1.0f, 1.0f };
	Vec4 bottom_right = Vec4{ static_cast<float>(win_width), 0.0f, -1.0f, 1.0f };
	Vec4 top_left = Vec4{ 0.0f, static_cast<float>(win_height), -1.0f, 1.0f };
	Vec4 top_right = Vec4{ static_cast<float>(win_width), static_cast<float>(win_height), -1.0f, 1.0f };

	Vec4 cam = Vec4{0.0f, 0.0f, 0.0f, 1.0f};

	Matrix PVM_inv, Vp_inv;
	Matrix VpPVM = Matrix::Matmul(Vp_matrix, PVM_matrix);

	//transform camera
	Matrix VM_inv;
	Matrix::InvertMatrix(VM, VM_inv);
	Vec4 cam_t = Matrix::Matmul(VM_inv, cam);

	Matrix PVM_Vp_inv;
	Matrix::InvertMatrix(VpPVM, PVM_Vp_inv);

	//tranform raster corners
	Vec4 bl_t4 = Matrix::Matmul(PVM_Vp_inv, bottom_left); //tranformed bottom left
	Vec4 br_t4 = Matrix::Matmul(PVM_Vp_inv, bottom_right); //tranformed bottom right
	Vec4 tl_t4 = Matrix::Matmul(PVM_Vp_inv, top_left);; //tranformed top left
	Vec4 tr_t4 = Matrix::Matmul(PVM_Vp_inv, top_right);; //transformed top right

	bl_t4.PerspectiveDivide();
	br_t4.PerspectiveDivide();
	tl_t4.PerspectiveDivide();
	tr_t4.PerspectiveDivide();

	Vec3 bl_t(bl_t4);
	Vec3 br_t(br_t4);
	Vec3 tl_t(tl_t4);
	Vec3 tr_t(tr_t4);

	Vec3 ray_origin(cam_t);

	Vec3 step_x = (br_t - bl_t) * (1.0f / (win_width - 1));
	Vec3 step_y = (tl_t - bl_t) * (1.0f / (win_height - 1));

	for (unsigned r = 0; r < win_height; ++r) {
		#ifdef THREADING
		thread_pool.enqueue([&, r, bl_t, step_x, step_y, ray_origin]() {
			ResolveOneRow(r, bl_t, step_x, step_y, ray_origin);
		});
		#endif // THREADING

		#ifndef THREADING
		ResolveOneRow(r, bl_t, step_x, step_y, ray_origin);
		#endif // THREADING		
	}

	#ifdef THREADING
	thread_pool.WaitUntilFinished();
	#endif // THREADING		
};

void Context::ResolveOneRow(int r, const Vec3& bl_world, const Vec3& step_x, const Vec3& step_y, const Vec3& ray_origin) {
	for (unsigned c = 0; c < win_width; ++c) {
		Vec3 pixel_in_world = bl_world + (c * step_x) + (r * step_y);
		Vec3 ray_direction = Vec3{ pixel_in_world.x - ray_origin.x, pixel_in_world.y - ray_origin.y, pixel_in_world.z - ray_origin.z };
		ray_direction.normalize();

		Ray ray { ray_origin, ray_direction };
		Vec3 color = TraceRay(ray, 0);

		//handle no intersection in first pass
		if (color.x == -1.0f) continue;

		//setpixel uses x, y not row column, so column becomes x
		SetPixelNoChecks(c, r, Color{color.x, color.y, color.z});
	}
}

Vec3 Context::TraceRay(const Ray& ray, int depth) {
	IntersectionData intersection = FindIntersection(ray);

	//for depth 0 (primary rays), find intersection with area lights,
	// if closer than intersection with object, return its color
	if (depth == 0) {
		IntersectionData light_intersection = FindLightIntersection(ray);
		if (light_intersection.valid) {
			const auto& material = emissive_materials[light_intersection.mat_idx];
			if (intersection.valid) {
				float d1 = intersection.point.Distance2(ray.origin);
				float d2 = light_intersection.point.Distance2(ray.origin);
				if (d2 < d1) {
					return Vec3{ material.r, material.g, material.b };
				}
			} else {
				return Vec3{ material.r, material.g, material.b };
			}
		}
	}

	if (!intersection.valid) {
		if (environment_map_set) {
			return RayEnvironmentMapColor(ray);
		} else {
			if (depth == 0) {
				//first pass no intersection -> dont write to color buffer
				return Vec3{ -1.0f, 0.0f, 0.0f };
			}
			else {
				return Vec3{ 0.0f, 0.0f, 0.0f };
			}
		}
	}

	Vec3 direct_lighting = ComputeDirectLight(intersection, ray.origin);

	Vec3 reflection_color{ 0.0f, 0.0f, 0.0f };
	Vec3 refraction_color{ 0.0f, 0.0f, 0.0f };
	const Material& mat = materials[intersection.mat_idx];
	
	if (depth < MAX_RECURSION_DEPTH) {
		//reflected ray
		if (mat.ks > 0.0f) {
			Vec3 dir_towards_origin = ray.origin - intersection.point;
			dir_towards_origin.normalize();
			Vec3 reflected_dir = Reflect(intersection.normal, dir_towards_origin);
			Ray reflected_ray{ intersection.point, reflected_dir };
			reflection_color = TraceRay(reflected_ray, depth + 1);
		}

		//refracted ray
		if (mat.T > 0.0f) {
			Vec3 refracted_dir = Refract(ray.direction, intersection.normal, mat.ior);
			if (refracted_dir.dot(refracted_dir) > 0.0f) {
				Ray refracted_ray{ intersection.point, refracted_dir };
				refraction_color = TraceRay(refracted_ray, depth + 1);
			}
		}
	}

	Vec3 color{ 0.0f, 0.0f, 0.0f };
	color += direct_lighting;
	color += mat.ks * reflection_color;
	color += mat.T * refraction_color;

	//clamping here is apparently not correct and produces a different
	// image for environment maps

	//color.x = std::clamp(color.x, 0.0f, 1.0f);
	//color.y = std::clamp(color.y, 0.0f, 1.0f);
	//color.z = std::clamp(color.z, 0.0f, 1.0f);

	return color;
};

IntersectionData Context::FindIntersection(const Ray& ray) {
	bool valid = false;
	Vec3 point{ 0.0f, 0.0f, 0.0f };
	Vec3 normal{ 0.0f, 0.0f, 0.0f };
	unsigned mat_idx = 0;

	int nearest_polygon = -1;
	int nearest_sphere = -1;
	float smallest_dist = std::numeric_limits<float>::max();
	Vec3 nearest_intersection;

	float new_dist;
	Vec3 intersection;

	for (unsigned long i = 0; i < sphere_buffer.size(); ++i) {
		const auto& sphere = sphere_buffer[i];

		if (!RaySphereIntersection(ray.origin, ray.direction, sphere, intersection)) continue;

		new_dist = intersection.Distance2(ray.origin);

		if (new_dist < smallest_dist && new_dist > SELF_INTERSECTION_TOLERANCE_DISTANCE2) {
			smallest_dist = new_dist;
			nearest_sphere = i;
			nearest_intersection = intersection;
		}
	}

	for (unsigned long i = 0; i < primitive_buffer.size(); ++i) {
		const auto& primitive = primitive_buffer[i];

		if (!RayTriangleIntersection(ray.origin, ray.direction, primitive, intersection)) continue;

		//backface culling
		normal = GetNormalizedNormal(primitive, intersection);
		if ((intersection - ray.origin).dot(normal) > 0.0f) {
			continue;
		}

		new_dist = intersection.Distance2(ray.origin);

		if (new_dist < smallest_dist && new_dist > SELF_INTERSECTION_TOLERANCE_SPHERE) {
			smallest_dist = new_dist;
			nearest_polygon = i;
			nearest_intersection = intersection;
			nearest_sphere = -1; //don't consider spheres anymore
		}
	}

	if (nearest_sphere == -1 && nearest_polygon == -1) { //no intersection
	}
	else if (nearest_polygon == -1) { //sphere
		const auto& intersected_sphere = sphere_buffer[nearest_sphere];

		valid = true;
		point = nearest_intersection;
		mat_idx = intersected_sphere.mat_idx;
		normal = GetNormalizedNormal(intersected_sphere, nearest_intersection);
	}
	else { //triangle
		const auto& intersected_triangle = primitive_buffer[nearest_polygon];

		valid = true;
		point = nearest_intersection;
		mat_idx = intersected_triangle.mat_idx;
		normal = GetNormalizedNormal(intersected_triangle, ray.origin);
	}

	return IntersectionData{valid, point, normal, mat_idx};
};

IntersectionData Context::FindLightIntersection(const Ray& ray) {
	bool valid = false;
	Vec3 point{ 0.0f, 0.0f, 0.0f };
	Vec3 normal{ 0.0f, 0.0f, 0.0f };
	unsigned mat_idx = 0;

	int nearest_polygon = -1;
	float smallest_dist = std::numeric_limits<float>::max();
	Vec3 nearest_intersection;

	float new_dist;
	Vec3 intersection;

	for (unsigned long i = 0; i < area_lights.size(); ++i) {
		const auto& area_light = area_lights[i];
		Polygon primitive;
		primitive.mat_idx = area_light.mat_idx;
		primitive.points = area_light.points;

		if (!RayTriangleIntersection(ray.origin, ray.direction, primitive, intersection)) continue;

		//backface culling
		normal = GetNormalizedNormal(primitive, intersection);
		if ((intersection - ray.origin).dot(normal) > 0.0f) {
			continue;
		}

		new_dist = intersection.Distance2(ray.origin);

		if (new_dist < smallest_dist && new_dist > SELF_INTERSECTION_TOLERANCE_SPHERE) {
			smallest_dist = new_dist;
			nearest_polygon = i;
			nearest_intersection = intersection;
		}
	}

	if (nearest_polygon == -1) { //no intersection
	}else { //triangle
		const auto& intersected_triangle = primitive_buffer[nearest_polygon];

		valid = true;
		point = nearest_intersection;
		mat_idx = intersected_triangle.mat_idx;
		normal = GetNormalizedNormal(intersected_triangle, ray.origin);
	}

	return IntersectionData{ valid, point, normal, mat_idx };
};

Vec3 Context::ComputeDirectLight(const IntersectionData& intersection, const Vec3& ray_origin) {
	//Phong
	Vec3 color{ 0.0f, 0.0f, 0.0f };
	const Vec3& N = intersection.normal;
	Vec3 E = ray_origin - intersection.point;
	E.normalize();

	const Material& material = materials[intersection.mat_idx];

	//point lights
	for (const auto& light : point_lights) {
		//cast shadow ray from intersection point to the light, if an object is between the
		//two points, continue
		Vec3 light_pos{ light.x, light.y, light.z };
		if (CastShadowRay(light_pos, intersection.point)) {
			continue;
		}

		//diffuse reflection
		Vec3 L = light_pos - intersection.point;
		L.normalize();
		float cos_alpha = L.dot(N);
		cos_alpha = std::max(cos_alpha, 0.0f);

		//specular reflection
		Vec3 R = (2 * cos_alpha * N) - L;
		float cos_beta_sh = std::pow(std::max(R.dot(E), 0.0f), material.shine);

		//combine the components together
		color.x += (light.r * material.r * material.kd * cos_alpha) + (light.r * material.ks * cos_beta_sh);
		color.y += (light.g * material.g * material.kd * cos_alpha) + (light.g * material.ks * cos_beta_sh);
		color.z += (light.b * material.b * material.kd * cos_alpha) + (light.b * material.ks * cos_beta_sh);
	}

	//area lights
	for (const auto& area_light : area_lights) {

		const auto& light_material = emissive_materials[area_light.mat_idx];

		for (int i = 0; i < AREA_LIGHT_SAMPLES; ++i) {
			Vec3 light_pos = SampleTriangle(area_light.points);

			if (CastShadowRay(light_pos, intersection.point)) {
				continue;
			}

			float d = light_pos.Distance(intersection.point);
			Vec3 light_to_inter_dir = intersection.point - light_pos;
			light_to_inter_dir.normalize();
			float cos_fi = light_to_inter_dir.dot(area_light.normal);
			cos_fi = std::max(cos_fi, 0.0f);
			float coef = cos_fi * (area_light.area / AREA_LIGHT_SAMPLES) /
				(light_material.c0 + light_material.c1 * d + light_material.c2 * d * d);

			Color light_intensity = {
				light_material.r * coef,
				light_material.g * coef,
				light_material.b * coef };

			//diffuse reflection
			Vec3 L = light_pos - intersection.point;
			L.normalize();
			float cos_alpha = L.dot(N);
			cos_alpha = std::max(cos_alpha, 0.0f);

			//specular reflection
			Vec3 R = (2 * cos_alpha * N) - L;
			float cos_beta_sh = std::pow(std::max(R.dot(E), 0.0f), material.shine);

			//combine the components together
			color.x += (light_intensity.r * material.r * material.kd * cos_alpha) + (light_intensity.r * material.ks * cos_beta_sh);
			color.y += (light_intensity.g * material.g * material.kd * cos_alpha) + (light_intensity.g * material.ks * cos_beta_sh);
			color.z += (light_intensity.b * material.b * material.kd * cos_alpha) + (light_intensity.b * material.ks * cos_beta_sh);
		}
	}

	return color;
};

Vec3 Context::SampleTriangle(const std::array<Vec3, 3>& points) {
	float r1, r2, u, v;

	//random r1, r2 from [0, 1]
	r1 = RandomFloat01();
	r2 = RandomFloat01();

	if (r1 + r2 > 1.0f) {
		u = 1.0f - r1;
		v = 1.0f - r2;
	} else {
		u = r1;
		v = r2;
	}

	Vec3 e1 = points[1] - points[0];
	Vec3 e2 = points[2] - points[0];

	return points[0] + u * e1 + v * e2;
}

float Context::RandomFloat01() {
	return unifrom_real_distribution(rng);
};

Vec3 Context::RayEnvironmentMapColor(const Ray& ray) {
	Vec3 color{ 0.0f, 0.0f, 0.0f };

	Vec3 dir = ray.direction;
	dir.normalize();

	float d, r, u, v;
	d = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	r = d > 0 ? std::acos(dir.z)/(2*PI*d) : 0.0f;
	u = 0.5 + dir.x * r;
	v = 0.5 + dir.y * r;
	
	/* flip the v since texture is stored top - down while texture
	coordinates start in bottom left corner */
	int x, y;
	x = static_cast<int>(std::floor(u * (environment_map.width)));
	y = static_cast<int>(std::floor((1.0f - v) * (environment_map.height)));

	x = std::clamp(x, 0, environment_map.width - 1);
	y = std::clamp(y, 0, environment_map.height - 1);

	int idx = (y * environment_map.width + x) * 3;

	color.x = environment_map.texels[idx];
	color.y = environment_map.texels[idx+1];
	color.z = environment_map.texels[idx+2];

	return color;
};

bool Context::CastShadowRay(const Vec3& light_pos, const Vec3& intersection_point) {
	float t = 0.0f;

	for (unsigned long i = 0; i < sphere_buffer.size(); ++i) {
		const auto& sphere = sphere_buffer[i];
		

		//NOTE sphere intersection works only for normalized direction
		Vec3 L = light_pos - intersection_point;
		float L_norm = sqrt(L.dot(L));

		L.normalize();

		if (!RaySphereIntersection(intersection_point, L, sphere, t)) continue;

		t = t / L_norm;

		if (SHADOW_RAY_TOLERANCE < t && t <= 1.0f - SHADOW_RAY_TOLERANCE) return true;
	}

	for (unsigned long i = 0; i < primitive_buffer.size(); ++i) {
		const auto& primitive = primitive_buffer[i];

		if (!RayTriangleIntersection(intersection_point, light_pos - intersection_point, primitive, t)) continue;

		if (SHADOW_RAY_TOLERANCE < t && t <= 1.0f - SHADOW_RAY_TOLERANCE) return true;
	}

	return false;
};

float Context::Fresnel(const Vec3& ray_direction, const Vec3& intersection_normal, float ior) {
	float F0 = (ior - 1.0f) / (ior + 1.0f);
	F0 *= F0;
	float r_dot_n = std::max(ray_direction.dot(intersection_normal), 0.0f);
	float F = F0 + (1.0f - F0) * std::pow((1.0f - r_dot_n), 5);
	return F;
};

Vec3 Context::Reflect(const Vec3& N, const Vec3& L) {
	float cos_alpha = L.dot(N);
	cos_alpha = std::max(cos_alpha, 0.0f);
	Vec3 R = (2 * cos_alpha * N) - L;
	return R;
}

Vec3 Context::Refract(const Vec3& I, const Vec3& N, float ior) {
	float cos_theta_I = std::clamp(-N.dot(I), -1.0f, 1.0f); // cosine of the incident angle
	float eta_I = 1.0f; // refractive index of the incident medium (air = 1.0)
	float eta_T = ior;  // refractive index of the transmitted medium
	Vec3 normal = N;

	// if the ray is exiting the medium, swap eta_I and eta_T and invert the normal
	if (cos_theta_I < 0) {
		cos_theta_I = -cos_theta_I;
		std::swap(eta_I, eta_T);
		normal = Vec3{-N.x, -N.y, -N.z};
	}

	float eta = eta_I / eta_T;
	float sin_theta_T2 = eta * eta * (1.0f - cos_theta_I * cos_theta_I);

	// check for total internal reflection
	if (sin_theta_T2 > 1.0f) {
		return Vec3{0.0f, 0.0f, 0.0f}; // no refraction, return a zero vector
	}

	float cos_theta_T = std::sqrt(1.0f - sin_theta_T2);
	return eta * I + (eta * cos_theta_I - cos_theta_T) * normal;
};

Vec3 Context::GetNormalizedNormal(const Sphere& sphere, const Vec3& intersection) {
	Vec3 center = Vec3{sphere.x, sphere.y, sphere.z};
	Vec3 normal = intersection - center;
	normal.normalize();
	return normal;
}

Vec3 Context::GetNormalizedNormal(const Polygon& polygon, const Vec3& ray_origin) {
	const Vec3& p0 = polygon.points[0];
	const Vec3& p1 = polygon.points[1];
	const Vec3& p2 = polygon.points[2];
	Vec3 normal = Vec3::Cross3D(p1 - p0, p2 - p1);

	normal.normalize();

	return normal;
}

bool Context::RayTriangleIntersection(const Vec3& ray_origin, const Vec3& ray_direction, const Polygon& primitive, Vec3& intersection) {

	// Moller-Trumbore intersection algorithm implementation
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

	// 1.0f minus the next representable value, essentially this is just 0
	// used because of float numerical imprescision
	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	Vec3 e1 = primitive.points[1] - primitive.points[0];
	Vec3 e2 = primitive.points[2] - primitive.points[0];
	Vec3 ray_cross_e2 = Vec3::Cross3D(ray_direction, e2);

	// these represent determinants of the
	// linear equation system
	// because Moller-Trumbore algorithm
	// uses Cramer´s rule to find solutions

	float det = e1.dot(ray_cross_e2);

	if (std::abs(det) < epsilon) { // parallel
		return false;
	}

	float inv_det = 1.0f / det;
	Vec3 s = ray_origin - primitive.points[0];
	// the u parameter representing the coefficent of edge 1
	float u = inv_det * s.dot(ray_cross_e2);

	if ((u < 0 && std::abs(u) > epsilon) || (u > 1 && std::abs(u - 1) > epsilon)) {
		return false;
	}

	Vec3 s_cross_e1 = Vec3::Cross3D(s, e1);
	// the v parameter representing the coefficient of edge 2
	// these two need to be a convex combination of the edges to be within the triangle
	float v = inv_det * ray_direction.dot(s_cross_e1);

	if ((v < 0 && std::abs(v) > epsilon) || (u + v > 1 && std::abs(u + v - 1) > epsilon)) {
		return false;
	}
	// t parameter representing the coefficient of ray direction
	float t = inv_det * e2.dot(s_cross_e1);

	if (t <= epsilon) {
		return false;
	}

	intersection = ray_origin + (Vec3(ray_direction.x * t, ray_direction.y * t, ray_direction.z * t));
	return true;
};

bool Context::RaySphereIntersection(const Vec3& ray_origin, const Vec3& ray_direction, const Sphere& sphere, Vec3& intersection) {
	// implementation of geometric solution found here:
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
	// check Figure 1 for better understanding of the code

	float t0, t1;

	Vec3 center = Vec3{sphere.x, sphere.y, sphere.z};
	Vec3 L = center - ray_origin;
	float tca = L.dot(ray_direction);

	if (tca < 0) {
		return false;
	}

	//compare squared dist to avoid sqrt computation
	float d2 = L.dot(L) - tca*tca;

	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	float thc = std::sqrt(sphere.radius*sphere.radius - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1) {
		std::swap(t0, t1);
	}

	float t;
	if (t0 < 0.0f || t0 < SELF_INTERSECTION_TOLERANCE_SPHERE) {
		t0 = t1;
		if (t0 < 0.0f || t0 < SELF_INTERSECTION_TOLERANCE_SPHERE) {
			return false;
		};
	}
	t = t0;

	intersection = ray_origin + (Vec3(ray_direction.x * t, ray_direction.y * t, ray_direction.z * t));
	return true;
};

bool Context::RayTriangleIntersection(const Vec3& ray_origin, const Vec3& ray_direction, const Polygon& primitive, float& t) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	Vec3 e1 = primitive.points[1] - primitive.points[0];
	Vec3 e2 = primitive.points[2] - primitive.points[0];
	Vec3 ray_cross_e2 = Vec3::Cross3D(ray_direction, e2);

	// these represent determinants of the
	// linear equation system
	// because Moller-Trumbore algorithm
	// uses Cramer´s rule to find solutions

	float det = e1.dot(ray_cross_e2);

	if (std::abs(det) < epsilon) { // parallel
		return false;
	}

	float inv_det = 1.0f / det;
	Vec3 s = ray_origin - primitive.points[0];
	// the u parameter representing the coefficent of edge 1
	float u = inv_det * s.dot(ray_cross_e2);

	if ((u < 0 && std::abs(u) > epsilon) || (u > 1 && std::abs(u - 1) > epsilon)) {
		return false;
	}

	Vec3 s_cross_e1 = Vec3::Cross3D(s, e1);
	// the v parameter representing the coefficient of edge 2
	// these two need to be a convex combination of the edges to be within the triangle
	float v = inv_det * ray_direction.dot(s_cross_e1);

	if ((v < 0 && std::abs(v) > epsilon) || (u + v > 1 && std::abs(u + v - 1) > epsilon)) {
		return false;
	}
	// t parameter representing the coefficient of ray direction
	float t_ = inv_det * e2.dot(s_cross_e1);

	if (t_ <= epsilon) {
		return false;
	}

	t = t_;
	return true;
};

bool Context::RaySphereIntersection(const Vec3& ray_origin, const Vec3& ray_direction, const Sphere& sphere, float& t) {
	// implementation of geometric solution found here:
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
	// check Figure 1 for better understanding of the code

	float t0, t1;

	Vec3 center = Vec3{ sphere.x, sphere.y, sphere.z };
	Vec3 L = center - ray_origin;
	float tca = L.dot(ray_direction);

	if (tca < 0) {
		return false;
	}

	//compare squared dist to avoid sqrt computation
	float d2 = L.dot(L) - tca * tca;

	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	float thc = std::sqrt(sphere.radius * sphere.radius - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1) {
		std::swap(t0, t1);
	}

	if (t0 < 0) {
		t0 = t1;
		if (t0 < 0) {
			return false;
		};
	}
	t = t0;

	return true;
};
