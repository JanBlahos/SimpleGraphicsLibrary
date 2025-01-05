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

	//raster corners
	Vec4 bottom_left = Vec4{ 0.0f, 0.0f, -1.0f, 1.0f };
	Vec4 bottom_right = Vec4{ static_cast<float>(win_width), 0.0f, -1.0f, 1.0f };
	Vec4 top_left = Vec4{ 0.0f, static_cast<float>(win_height), -1.0f, 1.0f };
	Vec4 top_right = Vec4{ static_cast<float>(win_width), static_cast<float>(win_height), -1.0f, 1.0f };

	Matrix PVM_inv, Vp_inv;
	Matrix VpPVM = Matrix::Matmul(Vp_matrix, PVM_matrix);

	//transform camera
	Vec4 cam = Vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
	Matrix VM_inv;
	Matrix::InvertMatrix(VM, VM_inv);
	Vec4 cam_t = Matrix::Matmul(VM_inv, cam);

	Matrix PVM_Vp_inv;
	Matrix::InvertMatrix(VpPVM, PVM_Vp_inv);

	//tranform raster corners
	Vec4 bl_t4 = Matrix::Matmul(PVM_Vp_inv, bottom_left); //tranformed bottom left
	Vec4 br_t4 = Matrix::Matmul(PVM_Vp_inv, bottom_right); //tranformed bottom right
	Vec4 tl_t4 = Matrix::Matmul(PVM_Vp_inv, top_left);; //tranformed top left
	//Vec4 tr_t4 = Matrix::Matmul(PVM_Vp_inv, top_right);; //transformed top right

	bl_t4.PerspectiveDivide();
	br_t4.PerspectiveDivide();
	tl_t4.PerspectiveDivide();
	//tr_t4.PerspectiveDivide();

	Vec3 bl_t(bl_t4);
	Vec3 br_t(br_t4);
	Vec3 tl_t(tl_t4);
	//Vec3 tr_t(tr_t4);

	Vec3 ray_origin(cam_t);

	Vec3 step_x = (br_t - bl_t) * (1.0f / (win_width - 1));
	Vec3 step_y = (tl_t - bl_t) * (1.0f / (win_height - 1));

	for (unsigned r = 0; r < win_height; ++r) {

		thread_pool.enqueue([&, r, bl_t, step_x, step_y, ray_origin]() {
			ResolveOneRow(r, bl_t, step_x, step_y, ray_origin);
		});
	}
	thread_pool.WaitUntilFinished();
};

void Context::ResolveOneRow(int r, const Vec3& bl_world, const Vec3& step_x, const Vec3& step_y, const Vec3& ray_origin) {
	Vec3 pixel_in_world = bl_world + (r * step_y);
	for (unsigned c = 0; c < win_width; ++c) {
		pixel_in_world += step_x;

		Vec3 ray_direction{ pixel_in_world.x - ray_origin.x, pixel_in_world.y - ray_origin.y, pixel_in_world.z - ray_origin.z };
		ray_direction.normalize();

		Color color;
		if (!ComputePixelColor(ray_origin, ray_direction, color)) continue;

		//setpixel uses x, y not row column, so column becomes x
		SetPixelNoChecks(c, r, color);
	}
}

bool Context::ComputePixelColor(const Vec3& ray_origin, const Vec3& ray_direction, Color& fragment_color) {
	
	int nearest_polygon = -1;
	int nearest_sphere = -1;
	float smallest_dist = std::numeric_limits<float>::max();
	Vec3 nearest_intersection;
	
	float new_dist;
	Vec3 intersection;

	//sphere objects
	for (unsigned long i = 0; i < sphere_buffer.size(); ++i) {
		const auto& sphere = sphere_buffer[i];

		if (!RaySphereIntersection(ray_origin, ray_direction, sphere, intersection)) continue;

		new_dist = intersection.Distance2(ray_origin);

		if (new_dist < smallest_dist) {
			smallest_dist = new_dist;
			nearest_sphere = i;
			nearest_intersection = intersection;
		}
	}

	//triangles
	for (unsigned long i = 0; i < primitive_buffer.size(); ++i) {
		const auto& primitive = primitive_buffer[i];

		if (!RayTriangleIntersection(ray_origin, ray_direction, primitive, intersection)) continue;

		new_dist = intersection.Distance2(ray_origin);

		if (new_dist < smallest_dist) {
			smallest_dist = new_dist;
			nearest_polygon = i;
			nearest_intersection = intersection;
			nearest_sphere = -1; //don't consider spheres anymore
		}
	}

	if (nearest_sphere == -1 && nearest_polygon == -1) { //no intersection
		return false;
	} else if (nearest_polygon == -1) { //sphere
		const auto& intersected_sphere = sphere_buffer[nearest_sphere];
		//need at least intersection point, primitive material, surface normal
		// (cross for triangle or subtract center for sphere, normalize!!!)
		fragment_color = ComputeLighting(ray_origin, nearest_intersection, materials.at(intersected_sphere.mat_idx), GetNormalizedNormal(intersected_sphere, nearest_intersection));
	} else { //triangle
		const auto& intersected_triangle = primitive_buffer[nearest_polygon];
		fragment_color = ComputeLighting(ray_origin, nearest_intersection, materials.at(intersected_triangle.mat_idx), intersected_triangle.normal);
	}

	return true;
}

Vec3 Context::GetNormalizedNormal(const Sphere& sphere, const Vec3& intersection) {
	Vec3 center = Vec3{sphere.x, sphere.y, sphere.z};
	Vec3 normal = intersection - center;
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
	if (t0 < 0) {
		t0 = t1;
		if (t0 < 0) {
			return false;
		};
	}
	t = t0;

	intersection = ray_origin + (Vec3(ray_direction.x * t, ray_direction.y * t, ray_direction.z * t));
	return true;
};

float ComputeTan(const Vec3& N, const Vec3& H) { 
	float n_dot_h = N.dot(H);
	float cos_theta = std::max(n_dot_h, 0.0f);
	float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
	// avoid division by zero
	return (cos_theta > 0.0f) ? (sin_theta / cos_theta) : std::numeric_limits<float>::max();
}

Color Context::ComputeLighting(const Vec3& ray_origin, const Vec3& intersection, const Material& material, const Vec3& surface_normal) {

#ifdef PHONG_LIGHTING

	//Phong
	Color color = Color{ 0.0f, 0.0f, 0.0f };
	const Vec3& N = surface_normal;
	Vec3 E = ray_origin - intersection;
	E.normalize();

	for (auto& light : point_lights) {
		//diffuse reflection
		Vec3 light_pos = Vec3{ light.x, light.y, light.z };
		Vec3 L = light_pos - intersection;
		L.normalize();
		float cos_alpha = L.dot(N);
		cos_alpha = std::max(cos_alpha, 0.0f);

		//specular reflection
		Vec3 R = (2 * cos_alpha * N) - L;
		float cos_beta_sh = std::pow(std::max(R.dot(E), 0.0f), material.shine);

		//combine the components together
		color.r += (light.r * material.r * material.kd * cos_alpha) + (light.r * material.ks * cos_beta_sh);
		color.g += (light.g * material.g * material.kd * cos_alpha) + (light.g * material.ks * cos_beta_sh);
		color.b += (light.b * material.b * material.kd * cos_alpha) + (light.b * material.ks * cos_beta_sh);
	}

#endif // PHONG_LIGHTING

#ifndef PHONG_LIGHTING

	// attempt at Cook-Torrance microfacet model,
	// specular component doesn't seem to work

	// uncomment PHONG_LIGHTING in context.h, material roughness can
	// also be set there since it is not given

	Vec3 color_v = Vec3 { 0.0f, 0.0f, 0.0f };
	const Vec3& N = surface_normal;
	Vec3 material_color = Vec3{ material.r, material.g, material.b };
	Vec3 V = ray_origin - intersection;
	V.normalize();
	for (auto& light : point_lights) {
		Vec3 light_pos = Vec3{ light.x, light.y, light.z };
		Vec3 light_color = Vec3{ light.r, light.g, light.b };
		Vec3 L = light_pos - intersection;
		L.normalize();
		//half-way vector between viewer and light
		Vec3 H = L + V;
		H.normalize();

		float n_dot_v = N.dot(V);
		float v_dot_h = V.dot(H);
		float h_dot_n = H.dot(N);
		float n_dot_l = N.dot(L);

		n_dot_v = std::max(0.0f, n_dot_v);
		v_dot_h = std::max(0.0f, v_dot_h);
		h_dot_n = std::max(0.0f, h_dot_n);
		n_dot_l = std::max(0.0f, n_dot_l);

		if (n_dot_l <= 0.0f || n_dot_v <= 0.0f) {
			continue;
		}

		//distribution, geometric attenuation and fresnel
		float D, G, F;

		//D
		float alpha = COOK_TORRANCE_ROUGHNESS;
		D = std::pow(EulerConstant, -(std::pow(ComputeTan(N, H) / alpha, 2)) ) / (PI * alpha * alpha * std::pow(h_dot_n, 4));

		//G
		float two_hn_vh = 2 * h_dot_n * (1.0f / v_dot_h);
		G = std::min(1.0f, std::min(two_hn_vh * n_dot_l, two_hn_vh * n_dot_v));

		//F
		float F0 = (material.ior - 1.0f) / (material.ior + 1.0f);
		F0 *= F0;
		F = F0 + (1.0f - F0) * std::pow((1.0f - v_dot_h), 5);

		//fr
		float fr = (D * G * F) / (4.0f * n_dot_l * n_dot_v);

		float specular = material.ks * fr;
		Vec3 diffuse = material.kd * material_color * (1.0f / PI);

		color_v += light_color * n_dot_l * (diffuse + Vec3{specular, specular, specular});
	}

	Color color = Color{color_v.x, color_v.y, color_v.z};

#endif // !PHONG_LIGHTING

	return color;
};
