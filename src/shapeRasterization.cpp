//---------------------------------------------------------------------------
// shapeRasterization.cpp
// contains line, circle, ellipse and arc drawing algorithms
//---------------------------------------------------------------------------

#include "context.h"
#include "exceptions.h"

void Context::BresenhamLine(int x1, int y1, int x2, int y2) {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}
	//First aglorithm draws all octants and is easier to code, however should
	//be slower when both algorithms are optimized

	//Pseudocode can be found on Wikipedia: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

	if (use_incremental_error) {
		int dx = std::abs(x2 - x1);
		int dy = -std::abs(y2 - y1);

		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;

		int err = dx + dy;

		while (true) {
			SetPixel(x1, y1);

			if (x1 == x2 && y1 == y2) break;

			int err2 = 2 * err;

			if (err2 >= dy) {
				err += dy;
				x1 += sx;
			}
			if (err2 <= dx) {
				err += dx;
				y1 += sy;
			}
		}
		return;
	}
	if (std::abs(y2 - y1) < std::abs(x2 - x1)) {
		if (x1 > x2) {
			PlotLineX(x2, y2, x1, y1);
		}
		else {
			PlotLineX(x1, y1, x2, y2);
		}
	}
	else {
		if (y1 > y2) {
			PlotLineY(x2, y2, x1, y1);
		}
		else {
			PlotLineY(x1, y1, x2, y2);
		}
	}
}

/// <summary>
/// Draw line which has greater change
/// in x axis using the Bresenham algorithm
/// </summary>
/// <param name="x1">Start point x coordinate. Maker sure that x1 < x2 </param>
/// <param name="y1"> Start point y coordinate.</param>
/// <param name="x2"> End point x coordinate. Make sure that x2 > x1 </param>
/// <param name="y2"> End point y coordinate.</param>
void Context::PlotLineX(int x1, int y1, int x2, int y2) {
	int delta_y = y2 - y1;
	int delta_x = x2 - x1;
	int y_step = y2 >= y1 ? 1 : -1;
	int k1 = 2 * y_step * delta_y;
	int k2 = 2 * y_step * (delta_y - delta_x);
	int p = 2 * delta_y - delta_x;
	int cur_x = x1;
	int cur_y = y1;
	while (true) {
		//std::cout << "Setting pixel" << cur_x << " " << cur_y <<  " goal is " << x2 << " " << y2 << std::endl;
		SetPixel(cur_x, cur_y);
		if (cur_x == x2) {
			break;
		}
		cur_x += 1;
		if (p * y_step > 0) {
			cur_y += y_step;
			p += k2;
		}
		else {
			p += k1;
		}
	}
};

/// <summary>
/// Draw line which has greater change
/// in x axis using the Bresenham algorithm
/// </summary>
/// <param name="x1">Start point x coordinate. Maker sure that x1 < x2 </param>
/// <param name="y1"> Start point y coordinate.</param>
/// <param name="x2"> End point x coordinate. </param>
/// <param name="y2"> End point y coordinate.  Make sure that y2 > y1</param>
void Context::PlotLineY(int x1, int y1, int x2, int y2) {
	int delta_y = y2 - y1;
	int delta_x = x2 - x1;
	int x_step = x2 >= x1 ? 1 : -1;
	int k1 = 2 * x_step * delta_x;
	int k2 = 2 * x_step * (delta_x - delta_y);
	int p = delta_y - 2 * delta_x;
	int cur_x = x1;
	int cur_y = y1;
	while (true) {
		SetPixel(cur_x, cur_y);
		if (cur_y == y2) {
			break;
		}
		cur_y += 1;
		if (p * x_step > 0) {
			cur_x += x_step;
			p += k2;
		}
		else {
			p += k1;
		}
	}
};

void Context::BresenhamCircle(float x, float y, float z, float radius) {
	//Note: this exception was changed as specified in sgl.h
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	Vec4 v(x, y, z, 1);
	Vec4 transformed_vec = Matrix::Matmul(PVM_matrix, v);
	transformed_vec.PerspectiveDivide();
	Vec4 vec_in_screen = Matrix::Matmul(Vp_matrix, transformed_vec);
	Matrix mat = Matrix::Matmul(Vp_matrix, PVM_matrix);

	//square root of determinant of the upper left 2x2 part of the matrix
	// times radius is the new radius
	auto new_radius = radius * sqrt(mat(0, 0) * mat(1, 1) - (mat(1, 0) * mat(0, 1)));
	auto new_x = round(v.x);
	auto new_y = round(v.y);
	//TODO use the new_z for depth buffer
	auto new_z = v.z;
	// draw the first octant starting point
	int current_x = new_x;
	//if y == 0 is at the bottom of the screen otherwise it would be subtracted
	int current_y = round(new_y + new_radius);
	//initialize the Bressenham algorithm constants
	int dvex = 3 + 2 * new_x;
	int dvey = 2 * current_y - 2;
	//starting decision constant if you put current_x and 
	// current_y into the parametric circle equation
	int p = (current_x * current_x) + 2 * current_x + 1
		+ (new_y * new_y) + 2 * (new_y * new_radius)
		- new_y - new_radius;
	// int p =  (current_x * current_x) + 2 * current_x + 1 
	// +(new_y * new_y) - 2 * (new_y * new_radius)
	// - new_y + new_radius; if y == 0 is at the top of the screen
	while (current_x <= current_y) {
		//draw the 8 symmetrical vertices
		//for flipping current x y based on
		// the starting axes
		int x_distance = (current_x - new_x);
		int y_distance = (current_y - new_y);
		//for drawing the vertices that are 
		// center symmetrical with current (x, y)
		int switched_x = (new_x + y_distance);
		int switched_y = (new_y + x_distance);
		SetPixel(current_x, current_y);
		SetPixel(new_x - x_distance, current_y);
		SetPixel(current_x, new_y - y_distance);
		SetPixel(new_x - x_distance, new_y - y_distance);
		SetPixel(switched_x, switched_y);
		SetPixel(new_x - y_distance, switched_y);
		SetPixel(switched_x, new_y - x_distance);
		SetPixel(new_x - y_distance, new_y - x_distance);
		if (p > 0) {
			p = p - dvey;
			dvey = dvey - 2;
			current_y = current_y - 1;
		}
		p = p + dvex;
		dvex = dvex + 2;
		current_x = current_x + 1;

	}
}

void Context::DrawArc(float x, float y, float z, float radius, float from, float to) {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}
	float total_angle = (abs(to - from)) / (2 * PI);
	auto num_vertices = round(NUM_SEGMENTS * total_angle);
	auto angle_step = total_angle / num_vertices;
	auto start_vec = Vec4(x + radius, y, z, 1);
	float cur_angle = from;
	int vertices = 0;
	while (vertices < num_vertices) {
		auto rotation_matrix = Matrix::RotateAroundCenter(x, y, cur_angle);
		auto temp_vec = Matrix::Matmul(rotation_matrix, start_vec);
		BufferVertex4f(temp_vec.x, temp_vec.y, temp_vec.z, temp_vec.w);
		cur_angle += angle_step;
		vertices++;
	}

}

void Context::DrawEllipse(float x, float y, float z, float a, float b) {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}
	auto angle_step = (2 * PI) / NUM_SEGMENTS;
	auto start_vec = Vec4(x + a, y, z, 1);
	float cur_angle = 0;
	int vertices = 0;
	//auto x_scaling = a / b;
	//auto y_scaling = b / a;
	while (vertices < NUM_SEGMENTS) {
		auto rotation_matrix = Matrix::RotateAroundCenter(x, y, cur_angle);
		auto temp_vec = Matrix::Matmul(rotation_matrix, start_vec);
		//save the unnormalized z and get rid of it for normalization
		float unnormalized_z = temp_vec.z;
		temp_vec.z = 0;
		//to get position on a unit circle 
		temp_vec.normalize();
		// multiply the position on unit circle by the appropriate scaling
		BufferVertex4f(x + (temp_vec.x * a), y + (temp_vec.y * b)
			, unnormalized_z, temp_vec.w);
		cur_angle += angle_step;
		vertices++;
	}
}
