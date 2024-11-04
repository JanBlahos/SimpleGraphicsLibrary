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

	if (USE_INCREMENTAL_ERROR) {
		int dx = std::abs(x2 - x1);
		int dy = -std::abs(y2 - y1);

		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;

		int err = dx + dy;

		while (true) {
			SetPixel(x1, y1, 0.0f);

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

void Context::BresenhamLine(int x1, int y1, float z1, int x2, int y2, float z2) {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}

	if (std::abs(y2 - y1) < std::abs(x2 - x1)) {
		if (x1 > x2) {
			PlotLineDepthX(x2, y2, z2, x1, y1, z1);
		}
		else {
			PlotLineDepthX(x1, y1, z1, x2, y2, z2);
		}
	}
	else {
		if (y1 > y2) {
			PlotLineDepthY(x2, y2, z2, x1, y1, z1);
		}
		else {
			PlotLineDepthY(x1, y1, z1, x2, y2, z2);
		}
	}
};

void Context::PlotLineX(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int D = (2 * dy) - dx;
	int y = y1;
	int two_dydx = 2 * (dy - dx);
	int two_dy = 2 * dy;

	for (int x = x1; x <= x2; ++x) {
		SetPixel(x, y, 0.0f);
		if (D > 0) {
			y += yi;
			D += two_dydx;
		} else {
			D += two_dy;
		}
	}
};

void Context::PlotLineDepthX(int x1, int y1, float z1, int x2, int y2, float z2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int D = (2 * dy) - dx;
	int y = y1;
	int two_dydx = 2 * (dy - dx);
	int two_dy = 2 * dy;

	float depth_step = (z2 - z1) / dx;
	float current_z = z1;

	for (int x = x1; x <= x2; ++x) {
		SetPixel(x, y, current_z);
		current_z += depth_step;

		if (D > 0) {
			y += yi;
			D += two_dydx;
		}
		else {
			D += two_dy;
		}
	}
}

void Context::PlotLineY(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}
	int D = (2 * dx) - dy;
	int x = x1;
	int two_dxdy = 2 * (dx - dy);
	int two_dx = 2 * dx;

	for (int y = y1; y <= y2; ++y) {
		SetPixel(x, y, 0.0f);
		if (D > 0) {
			x += xi;
			D += two_dxdy;
		}
		else {
			D += two_dx;
		}
	}
};

void Context::PlotLineDepthY(int x1, int y1, float z1, int x2, int y2, float z2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}
	int D = (2 * dx) - dy;
	int x = x1;
	int two_dxdy = 2 * (dx - dy);
	int two_dx = 2 * dx;

	float depth_step = (z2 - z1) / dy;
	float current_z = z1;

	for (int y = y1; y <= y2; ++y) {
		SetPixel(x, y, current_z);
		current_z += depth_step;

		if (D > 0) {
			x += xi;
			D += two_dxdy;
		}
		else {
			D += two_dx;
		}
	}
}

void Context::DrawSymmetrical(int centerx, int centery, int x, int y, bool lines, float depth) {
	int points_x[8] = { x + centerx, x - centerx, x + centerx, x - centerx, x + centery, x - centery, x + centery, x - centery};
	int points_y[8] = { y + centery, y + centery, y - centery, y - centery, y + centerx, y + centerx, y - centerx, y - centerx };
	if (lines) {
		//Draw lines between points with identical y
		// to fill the circle (similiar idea as scan-line)
		for (int i = 0; i < 7; i+= 2) {
			BresenhamLine(points_x[i], points_y[i], depth, points_x[i + 1], points_y[i + 1], depth);
		}
	}
	else {
		for (int i = 0; i < 8; i++) {
			SetPixel(points_x[i], points_y[i], depth);
		}
	}
}

void Context::BresenhamCircle(float x, float y, float z, float radius) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	BeginDrawing(SGL_POINTS);
	Vec4 v(x, y, z, 1);
	Vec4 transformed_vec = Matrix::Matmul(PVM_matrix, v);
	transformed_vec.PerspectiveDivide();
	Matrix mat = Matrix::Matmul(Vp_matrix, PVM_matrix);
	Vec4 vec_in_screen = Matrix::Matmul(Vp_matrix, transformed_vec);

	//square root of determinant of the upper left 2x2 part of the matrix
	// times radius is the new radius
	auto new_radius = radius * sqrt(mat(0, 0) * mat(1, 1) - (mat(1, 0) * mat(0, 1)));
	 
	auto new_x = static_cast<int>(floor(vec_in_screen.x));
	auto new_y = static_cast<int>(floor(vec_in_screen.y));
	auto new_z = vec_in_screen.z;
	if (filling_mode == SGL_POINT) {
		//SetPixel(new_x, new_y, new_z);
		DrawPoint(new_x, new_y, new_z);
	}

	// draw the first octant starting point
	int current_x = 0;
	int current_y = static_cast<int>(round(new_radius));
	//initialize the Bressenham algorithm constants
	int dvex = 3;
	int dvey = static_cast<int>(2 * new_radius - 2);
	//initialize the decision constant
	int p = static_cast<int>(1 - new_radius);
	bool lines = filling_mode == SGL_FILL;
	while (current_x <= current_y) {
		DrawSymmetrical(current_x, current_y, new_x, new_y, lines, new_z);
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
	auto draw_mode = SGL_LINE_STRIP;
	switch (filling_mode) {
		case SGL_FILL:
			draw_mode = SGL_POLYGON;
			break;
		case SGL_POINT:
			draw_mode = SGL_POINTS;
			break;
		default:
			break;
	}
	BeginDrawing(draw_mode);

	//if difference is too big, keep subtracting 2pi
	while (to - from > 2 * PI) to -= 2 * PI;
	//fix drawing arc where to < from
	while (from > to) from -= 2 * PI;

	int num_vertices = static_cast<int>(std::round(NUM_SEGMENTS * std::abs(to - from) / (2 * PI)));

	/// Draw center if points or fill area mode are specified
	if (draw_mode == SGL_POLYGON || draw_mode == SGL_POINTS) {
		BufferVertex4f(x, y, z, 1.0f);
	}

	for (int i = 0; i < num_vertices; i++) {
		float theta = from + (to - from) * static_cast<float>(i) / (num_vertices - 1);
		float x_pos = x + radius * cos(theta);
		float y_pos = y + radius * sin(theta);
		//
		BufferVertex4f(x_pos, y_pos, z, 1.0f);
	}
	EndDrawing();
}

constexpr std::array<float, 22> precomputed_angles = {
	1.0f, 0.0f, 0.987688f, 0.156434f, 0.951057f, 0.309017f, 0.891007f,
	0.453991f, 0.809017f, 0.587785f, 0.707107f, 0.707107f, 0.587785f,
	0.809017f, 0.45399f, 0.891007f, 0.309017f, 0.951057f, 0.156434f,
	0.987688f, 0.0f, 1.0f
};


Vec4 Context::VertexToScreen(const Vec4& vertex, const Matrix& PVM, const Matrix& Vp) {

	Vec4 temp = Matrix::Matmul(PVM, vertex);
	temp.PerspectiveDivide();
	return Matrix::Matmul(Vp, temp);
}

void Context::DrawEllipse(float x, float y, float z, float a, float b) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}

	auto draw_mode = SGL_LINE_STRIP;
	switch (filling_mode) {
	case SGL_FILL:
		draw_mode = SGL_POLYGON;
		break;
	case SGL_LINE:
		draw_mode = SGL_POLYGON;
		break;
	case SGL_POINT:
		draw_mode = SGL_LINES;
		break;
	default:
		break;
	}

	if (filling_mode == SGL_POINT) {
		Matrix PVM = Matrix::Matmul(matrix_stack.GetProjectionMatrix(), matrix_stack.GetViewModelMatrix());
		const Matrix& Vp = matrix_stack.GetViewport();
		Vec4 transformed_center = VertexToScreen(Vec4{x, y, z, 1.0f}, PVM, Vp);
		DrawPoint(transformed_center.x, transformed_center.y, transformed_center.z);
	}

	BeginDrawing(draw_mode);

	/// Buffer center if points or fill area mode are specified
	/*if (draw_mode == SGL_POLYGON) {
		BufferVertex4f(x, y, z, 1.0f);
	}*/

	for (int i = 0; i < NUM_SEGMENTS; i++) {
		float theta = (2 * PI) * static_cast<float>(i) / (NUM_SEGMENTS - 1);
		float x_pos = x + a * cos(theta);
		float y_pos = y + b * sin(theta);
		//
		BufferVertex4f(x_pos, y_pos, z, 1.0f);
	}
	EndDrawing();
	//if (is_drawing) {
	//	throw SGLInvalidOperationException("Cannot call this function while drawing.");
	//}
	//is_drawing = true;

	////draw by quadrants, use precomputed angles
	//Matrix PVM = Matrix::Matmul(matrix_stack.GetProjectionMatrix(), matrix_stack.GetViewModelMatrix());
	//const Matrix& Vp = matrix_stack.GetViewport();

	////TODO hw02
	///*if (area_mode == sglEAreaMode::SGL_POINT) {
	//	...
	//}*/

	//PVM = Matrix::Matmul(PVM, Matrix::Translation3D(x, y, z));

	//float x_pos = a * precomputed_angles[0];
	//float y_pos = b * precomputed_angles[1];

	//Vec4 v1 = VertexToScreen(Vec4{x_pos, y_pos, 0.0f, 1.0f}, PVM, Vp),
	//	 v2 = VertexToScreen(Vec4{ -x_pos, y_pos, 0.0f, 1.0f }, PVM, Vp),
	//	 v3 = VertexToScreen(Vec4{ x_pos, -y_pos, 0.0f, 1.0f }, PVM, Vp),
	//	 v4 = VertexToScreen(Vec4{ -x_pos, -y_pos, 0.0f, 1.0f }, PVM, Vp);

	//for (int i = 1; i < QUADRANT_SEGMENTS+1; i++) {
	//	int j = 2 * i;
	//	x_pos = a * precomputed_angles[j];
	//	y_pos = b * precomputed_angles[j+1];

	//	Vec4 u1 = VertexToScreen(Vec4{ x_pos, y_pos, 0.0f, 1.0f }, PVM, Vp),
	//		u2 = VertexToScreen(Vec4{ -x_pos, y_pos, 0.0f, 1.0f }, PVM, Vp),
	//		u3 = VertexToScreen(Vec4{ x_pos, -y_pos, 0.0f, 1.0f }, PVM, Vp),
	//		u4 = VertexToScreen(Vec4{ -x_pos, -y_pos, 0.0f, 1.0f }, PVM, Vp);

	//	BresenhamLine(static_cast<int>(v1.x), static_cast<int>(v1.y), static_cast<int>(u1.x), static_cast<int>(u1.y));
	//	BresenhamLine(static_cast<int>(v2.x), static_cast<int>(v2.y), static_cast<int>(u2.x), static_cast<int>(u2.y));
	//	BresenhamLine(static_cast<int>(v3.x), static_cast<int>(v3.y), static_cast<int>(u3.x), static_cast<int>(u3.y));
	//	BresenhamLine(static_cast<int>(v4.x), static_cast<int>(v4.y), static_cast<int>(u4.x), static_cast<int>(u4.y));

	//	v1 = u1;
	//	v2 = u2;
	//	v3 = u3;
	//	v4 = u4;
	//}

	//is_drawing = false;
}
