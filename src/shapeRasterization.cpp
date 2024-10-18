//---------------------------------------------------------------------------
// shapeRasterization.cpp
// contains line, circle, ellipse and arc drawing algorithms
//---------------------------------------------------------------------------

#include "context.h"
#include "exceptions.h"
#include <iostream>

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
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	BeginDrawing(SGL_POINTS);
	Vec4 v(x, y, z, 1);
	//Vec4::PrintVector(v);
	Vec4 transformed_vec = Matrix::Matmul(PVM_matrix, v);
	transformed_vec.PerspectiveDivide();
	Matrix mat = Matrix::Matmul(Vp_matrix, PVM_matrix);
	Vec4 vec_in_screen = Matrix::Matmul(Vp_matrix, transformed_vec);
	//TODO. Draw the center point if SGL_POINT fill mode was specified

	//square root of determinant of the upper left 2x2 part of the matrix
	// times radius is the new radius
	auto new_radius = radius * sqrt(mat(0, 0) * mat(1, 1) - (mat(1, 0) * mat(0, 1)));
	auto new_x = round(vec_in_screen.x);
	auto new_y = round(vec_in_screen.y);
	//TODO use the new_z for depth buffer
	auto new_z = vec_in_screen.z;
	// draw the first octant starting point
	int current_x = 0;
	int current_y = round(new_radius);
	//initialize the Bressenham algorithm constants
	int dvex = 3;
	int dvey = 2 * new_radius - 2;
	//initialize the decision constant
	int p = 1 - new_radius;
	while (current_x <= current_y) {
		//draw the 8 symmetrical vertices
		SetPixel(new_x + current_x, new_y + current_y);
		SetPixel(new_x - current_x, new_y + current_y);
		SetPixel(new_x + current_x, new_y - current_y);
		SetPixel(new_x - current_x, new_y - current_y);
		SetPixel(new_x + current_y, new_y + current_x);
		SetPixel(new_x - current_y, new_y + current_x);
		SetPixel(new_x + current_y, new_y - current_x);
		SetPixel(new_x - current_y, new_y - current_x);

		if (p > 0) {
			p = p - dvey;
			dvey = dvey - 2;
			current_y = current_y - 1;
		}
		p = p + dvex;
		dvex = dvex + 2;
		current_x = current_x + 1;

	}
	EndDrawing();
}

void Context::DrawArc(float x, float y, float z, float radius, float from, float to) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	BeginDrawing(SGL_LINE_STRIP);

	float total_angle = (abs(to - from)) / (2 * PI);
	int num_vertices = round(NUM_SEGMENTS * total_angle);

	for (int i = 0; i < num_vertices; i++) {
		float theta = from + (to - from) * static_cast<float>(i) / (num_vertices - 1);
		float x_pos = x + radius * cos(theta);
		float y_pos = y + radius * sin(theta);
		BufferVertex4f(x_pos, y_pos, 0, 1);
	}
	EndDrawing();
}

void Context::DrawEllipse(float x, float y, float z, float a, float b) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	BeginDrawing(SGL_LINE_LOOP);
	for (int i = 0; i < NUM_SEGMENTS; i++) {
		float theta = (2 * PI * i) / NUM_SEGMENTS;
		float x_pos = x + a * cos(theta);
		float y_pos = y + b * sin(theta);
		BufferVertex4f(x_pos, y_pos, 0, 1);
	}
	EndDrawing();
}
