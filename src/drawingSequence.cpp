//---------------------------------------------------------------------------
// drawingSequence.cpp
// Implements methods from context.h related to Begin() and End()
// sequence
//---------------------------------------------------------------------------


#include "context.h"
#include "exceptions.h"
#include <iostream>

void Context::BeginDrawing(sglEElementType mode) {
	if (mode >= SGL_LAST_ELEMENT_TYPE) {
		throw SGLInvalidEnumException("This drawing mode doesn't exist.");
	}
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	is_drawing = true;
	drawing_mode = mode;
	first_point.x = -1;
	first_point.y = -1;

	//get viewport and PVM matrices

	auto current_mode = matrix_stack.GetMode();
	matrix_stack.SetMode(SGL_MODELVIEW);
	const Matrix& VM = matrix_stack.Top();

	matrix_stack.SetMode(SGL_PROJECTION);
	const Matrix& P = matrix_stack.Top();
	matrix_stack.SetMode(current_mode);

	//Matrix::PrintMatrix(VM);
	//Matrix::PrintMatrix(P);

	PVM_matrix = Matrix::Matmul(P, VM);

	Vp_matrix = matrix_stack.GetViewport();
};

void Context::EndDrawing() {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}

	// TODO
	// - close line loop, last triangle, etc. depending on mode

	if (drawing_mode == SGL_LINE_LOOP) {
		BresenhamLine(first_point.x, first_point.y, vertex_buffer.back().x, vertex_buffer.back().y);
	}
	is_drawing = false;

	//clear the vertex buffer
	vertex_buffer.clear();
};

void Context::DrawVertex(int x1, int y1) {
	switch (drawing_mode) {

	case SGL_POINTS:
		DrawPoint(x1, y1);
		break;

	case SGL_LINES:
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.front().x, vertex_buffer.front().y, x1, y1);
			vertex_buffer.clear();
		}
		else {
			vertex_buffer.push_back(Point2D{ x1, y1 });
		}
		break;

	case SGL_LINE_STRIP: //same as lines but doesn't clear buffer, just pops front
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.front().x, vertex_buffer.front().y, x1, y1);
			vertex_buffer.pop_front();
		}
		//always push new vertex
		vertex_buffer.push_back(Point2D{ x1, y1 });
		break;

	case SGL_LINE_LOOP: //same as strip but needs to connect first and last when End() is called
		if (first_point.x == -1) {
			first_point.x = x1;
			first_point.y = y1;
		}
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.back().x, vertex_buffer.back().y, x1, y1);
			vertex_buffer.pop_front();
		}
		vertex_buffer.push_back(Point2D{ x1, y1 });
		break;
		//TODO triangles, polygon, etc. (from sglEElementType)
	default:
		break;
	}
}

void Context::BufferVertex4f(float x, float y, float z, float w) {
	//tranform to screen
	Vec4 v(x, y, z, w);
	//Vec4::PrintVector(v);
	Vec4 transformed_vec = Matrix::Matmul(PVM_matrix, v);
	transformed_vec.PerspectiveDivide();
	//Matrix::PrintMatrix(PVM_matrix);
	//Matrix::PrintMatrix(Vp_matrix);
	Vec4 vec_in_screen = Matrix::Matmul(Vp_matrix, transformed_vec);
	//Vec4::PrintVector(vec_in_screen);

	float _tx, _ty;
	_tx = vec_in_screen.x;
	_ty = vec_in_screen.y;
	int tx, ty;
	//TODO perhaps round elsewhere?

	/*tx = std::round(_tx);
	ty = std::round(_ty);*/

	tx = std::floor(_tx);
	ty = std::floor(_ty);
	DrawVertex(tx, ty);
};

void Context::BufferVertex3f(float x, float y, float z) {
	BufferVertex4f(x, y, z, 1.0f);
};

void Context::BufferVertex2f(float x, float y) {
	BufferVertex4f(x, y, 0.0f, 1.0f);
};
