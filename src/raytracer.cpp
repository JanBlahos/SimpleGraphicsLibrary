#include "context.h"
#include "exceptions.h"
#include <iostream>

void Context::BeginScene() {
	if (is_drawing) {
		throw SGLInvalidOperationException("Called inside Begin-End sequence");
	}

	is_setting_scene = true;
};

void Context::EndScene() {
	if (is_drawing) {
		throw SGLInvalidOperationException("Called inside Begin-End sequence");
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
	//TODO throw exceptions
	materials.emplace_back(Material{r, g, b, kd, ks, shine, T, ior});
};

void Context::CreatePointLight(const float x,
	const float y,
	const float z,
	const float r,
	const float g,
	const float b) 
{
	//TODO throw exceptions
	point_lights.push_back(PointLight{x, y, z, r, g, b});
};

void Context::CreateSphere(const float x,
	const float y,
	const float z,
	const float radius)
{
	//TODO throw exceptions
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
	//TODO throw exceptions

	//fetch matrices, BeginScene() does this but isn't called for
	//sphere-only scenes
	const Matrix& VM = matrix_stack.GetViewModelMatrix();
	const Matrix& P = matrix_stack.GetProjectionMatrix();

	//std::cout << "viewmodel matrix:\n";
	//Matrix::PrintMatrix(VM);

	PVM_matrix = Matrix::Matmul(P, VM);
	Vp_matrix = matrix_stack.GetViewport();

	//TODO maybe top left should be (0, 0)
	Vec4 bottom_left = Vec4{0.0f, 0.0f, -1.0f, 1.0f};
	Vec4 bottom_right = Vec4{ static_cast<float>(win_width), 0.0f, -1.0f, 1.0f };
	Vec4 top_left = Vec4{ 0.0f, static_cast<float>(win_height), -1.0f, 1.0f };
	Vec4 top_right = Vec4{ static_cast<float>(win_width), static_cast<float>(win_height), -1.0f, 1.0f };

	//const auto& sp = sphere_buffer.back();
	//std::cout << "Sphere xyz and radius:" << sp.x << " " << sp.y << " " << sp.z << " " << sp.radius << "\n";

	std::cout << "corners of the screen in raster:\n";
	Vec4::PrintVector(bottom_left);
	Vec4::PrintVector(bottom_right);
	Vec4::PrintVector(top_left);
	Vec4::PrintVector(top_right);

	//TODO camera xyz wrong?
	Vec4 cam = Vec4{0.0f, 0.0f, 0.0f, 1.0f};

	//Matrix::PrintMatrix(Vp_matrix);

	Matrix PVM_inv, Vp_inv;
	std::cout << "VM_matrix" << std::endl;
	Matrix::PrintMatrix(VM); 
	Matrix::InvertMatrix(PVM_matrix, PVM_inv);
	/*std::cout << "PVM inversion" << std::endl;
	Matrix::PrintMatrix(PVM_inv);*/
	Matrix::InvertMatrix(Vp_matrix, Vp_inv);
	/*std::cout << "VP_matrix: " << std::endl;
	Matrix::PrintMatrix(Vp_matrix);
	std::cout << "Vp_inversion: " << std::endl;
	Matrix::PrintMatrix(Vp_inv);*/


	//transform camera
	//TODO dont use inverse projection?
	Matrix VM_inv;
	Matrix::InvertMatrix(VM, VM_inv);
	Vec4 cam_t = Matrix::Matmul(VM_inv, cam);
	//Vec4 cam_t = Matrix::Matmul(PVM_inv, cam);
	cam_t.PerspectiveDivide();

	Matrix PVM_Vp_inv = Matrix::Matmul(PVM_inv, Vp_inv);

	std::cout << "Camera xyzw: " << cam_t.x << " " << cam_t.y << " " << cam_t.z << " " << cam_t.w << "\n";


	//tranform raster corners
	Vec4 bl_t = Matrix::Matmul(PVM_Vp_inv, bottom_left); //tranformed bottom left
	bl_t.PerspectiveDivide();
	//bl_t.z -= 1;
	Vec4 br_t = Matrix::Matmul(PVM_Vp_inv, bottom_right); //tranformed bottom right
	br_t.PerspectiveDivide();
	//br_t.z -= 1;
	Vec4 tl_t = Matrix::Matmul(PVM_Vp_inv, top_left);; //tranformed top left
	tl_t.PerspectiveDivide();
	//tl_t.z -= 1;
	Vec4 tr_t = Matrix::Matmul(PVM_Vp_inv, top_right);; //transformed top right
	tr_t.PerspectiveDivide();
	//tr_t.z -= 1;

	std::cout << "corners of the screen in world:\n";
	Vec4::PrintVector(bl_t);
	Vec4::PrintVector(br_t);
	Vec4::PrintVector(tl_t);
	Vec4::PrintVector(tr_t);

	Vec4 ray_origin = cam_t;
	Vec4 ray_direction = Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };

	for (int r = 0; r < win_height; ++r) {
		for (int c = 0; c < win_width; ++c) {
			float u = static_cast<float>(c) / (win_width - 1);
			float v = static_cast<float>(r) / (win_height - 1);

			Vec4 pixel_in_world = BilinearInterpolation(bl_t, br_t, tl_t, tr_t, u, v);
			ray_direction = Vec4{ pixel_in_world.x - ray_origin.x, pixel_in_world.y - ray_origin.y, pixel_in_world.z - ray_origin.z, 0.0f };
			//std::cout << "Ray direction\n";
			//Vec4::PrintVector(ray_direction);
			ray_direction.normalize();

			Color color = ComputePixelColor(ray_origin, ray_direction);

			if (color.r == -1.0f) continue;

			SetPixelNoChecks(r, c, color);
		}
	}
};

Color Context::ComputePixelColor(Vec4 ray_origin, Vec4 ray_direction) {
	
	int nearest_polygon = -1;
	int nearest_sphere = -1;
	float smallest_dist = std::numeric_limits<float>::max();
	Vec4 nearest_intersection = Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < sphere_buffer.size(); ++i) {
		auto& sphere = sphere_buffer[i];

		Vec4 intersection = RaySphereIntersection(ray_origin, ray_direction, sphere);
		if (intersection.w == -1) continue;

		float new_dist = intersection.Distance(ray_origin);

		if (new_dist > 0.0f && new_dist < smallest_dist) {
			smallest_dist = new_dist;
			nearest_sphere = i;
			nearest_intersection = intersection;
		}
	}

	for (int i = 0; i < primitive_buffer.size(); ++i) {
		auto& primitive = primitive_buffer[i];
		
		if (primitive.points.size() != 3) {
			throw SGLInvalidOperationException("Primitives other than triangles"
				"are currently not supported");
		}

		Vec4 intersection = RayTriangleIntersection(ray_origin, ray_direction, primitive);
		if (intersection.w == -1) continue;

		float new_dist = intersection.Distance(ray_origin);

		if (new_dist > 0.0f && new_dist < smallest_dist) {
			smallest_dist = new_dist;
			nearest_polygon = i;
			nearest_intersection = intersection;
			nearest_sphere = -1; //don't consider spheres anymore
		}
	}

	Color fragment_color;

	if (nearest_sphere == -1 && nearest_polygon == -1) { //no intersection
		//return invalid color
		//fragment_color = clear_color;
		fragment_color = Color{-1.0f, -1.0f, -1.0f};

	} else if (nearest_polygon == -1) { //sphere
		const auto& intersected_sphere = sphere_buffer[nearest_sphere];

		//need at least intersection point, primitive material, surface normal
		// (cross for triangle or subtract center for sphere, normalize!!!)
		fragment_color = ComputeLighting(nearest_intersection, materials.at(intersected_sphere.mat_idx), GetNormalizedNormal(intersected_sphere, nearest_intersection));
	} else { //triangle
		const auto& intersected_triangle = primitive_buffer[nearest_polygon];
		fragment_color = ComputeLighting(nearest_intersection, materials.at(intersected_triangle.mat_idx), GetNormalizedNormal(intersected_triangle));
	}

	return fragment_color;
}

Vec4 Context::GetNormalizedNormal(const Sphere& sphere, const Vec4& intersection) {
	Vec4 center = Vec4{sphere.x, sphere.y, sphere.z, 1.0f};
	Vec4 normal = intersection - center;
	normal.normalize();
	return normal;
}

Vec4 Context::GetNormalizedNormal(const Polygon& polygon) {
	const Vec4& p0 = polygon.points[0];
	const Vec4& p1 = polygon.points[1];
	const Vec4& p2 = polygon.points[2];
	Vec4 normal = Vec4::Cross3D(p1 - p0, p1 - p2);

	//flip if facing away, no polygon orientation defined (ccw/cw)
	Vec4 dir_towards_camera = Vec4{ 0.0f, 0.0f, 1.0f, 0.0f };
	normal.normalize();
	if (normal.dot(dir_towards_camera) < 0) {
		normal = Vec4{-normal.x, -normal.y, -normal.z, normal.w};
	}

	//normal.normalize();
	return normal;
}

Vec4 Context::RayTriangleIntersection(const Vec4& ray_origin, const Vec4& ray_direction, const Polygon& primitive) {
	Vec4 failed_intersection = Vec4{0.0f, 0.0f, 0.0f, -1.0f};

	// Moller-Trumbore intersection algorithm implementation
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

	// 1.0f minus the next representable value, essentially this is just 0
	// used because of float numerical inprescision
	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	Vec4 e1 = primitive.points[1] - primitive.points[0];
	Vec4 e2 = primitive.points[2] - primitive.points[0];
	Vec4 ray_cross_e2 = Vec4::Cross3D(ray_direction, e2);

	// these represent determinants of the
	// linear equation system
	// because Moller-Trumbore algorithm
	// uses Cramer´s rule to find solutions

	float det = e1.dot(ray_cross_e2);

	if (std::abs(det) < epsilon) { // parallel
		return failed_intersection;
	}

	float inv_det = 1.0f / det;
	Vec4 s = ray_origin - primitive.points[0];
	// the u parameter representing the coefficent of edge 1
	float u = inv_det * s.dot(ray_cross_e2);

	if ((u < 0 && std::abs(u) > epsilon) || (u > 1 && std::abs(u - 1) > epsilon)) {
		return failed_intersection;
	}

	Vec4 s_cross_e1 = Vec4::Cross3D(s, e1);
	// the v parameter representing the coefficient of edge 2
	// these two need to be a convex combination of the edges to be within the triangle
	float v = inv_det * ray_direction.dot(s_cross_e1);

	if ((v < 0 && std::abs(v) > epsilon) || (u + v > 1 && std::abs(u + v - 1) > epsilon)) {
		return failed_intersection;
	}
	// t parameter representing the coefficient of ray direction
	float t = inv_det * e2.dot(s_cross_e1);

	if (t <= epsilon) {
		return failed_intersection;
	}

	return ray_origin + (Vec4(ray_direction.x * t, ray_direction.y * t, ray_direction.z * t, 0.0f));
};

Vec4 Context::RaySphereIntersection(const Vec4& ray_origin, const Vec4& ray_direction, const Sphere& sphere) {
	Vec4 failed_intersection = Vec4{ 0.0f, 0.0f, 0.0f, -1.0f };

	// implementation of geometric solution found here:
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
	// check Figure 1 for better understanding of the code

	float t0, t1;

	Vec4 center = Vec4{sphere.x, sphere.y, sphere.z, 1.0f};
	Vec4 L = center - ray_origin;
	float tca = L.dot(ray_direction);

	if (tca < 0) {
		return failed_intersection;
	}

	//compare squared dist to avoid sqrt computation
	float d2 = L.dot(L) - tca*tca;

	if (d2 > sphere.radius * sphere.radius) {
		return failed_intersection;
	}

	float thc = std::sqrt(sphere.radius*sphere.radius - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1) {
		std::swap(t0, t1);
	}

	float t;
	if (t0 < 0) {
		t = t1;
		if (t < 0) {
			return failed_intersection;
		};
	} else {
		t = t0;
	}

	return ray_origin + (Vec4(ray_direction.x * t, ray_direction.y * t, ray_direction.z * t, 0.0f));
};

Color Context::ComputeLighting(const Vec4& intersection, const Material& material, const Vec4& surface_normal) {

	Color color = Color{ material.r, material.g, material.b };

	//TODO Phong, utilize points_lights vector

	return color;
};
