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
	PVM_matrix = Matrix::Eye(4);

	matrix_stack.SetMode(SGL_PROJECTION);
	auto p = matrix_stack.Top();
	PVM_matrix->Matmul(p);

	matrix_stack.SetMode(SGL_MODELVIEW);
	auto vm = matrix_stack.Top();
	PVM_matrix->Matmul(vm);

	Vp_matrix = matrix_stack.GetViewport();

};

void Context::EndDrawing() {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}
	is_drawing = false;

	// TODO
	// - close line loop, last triangle, etc. depending on mode
	// - clear the vertex buffer


	if (drawing_mode == SGL_LINE_LOOP) {
		BresenhamLine(first_point.x, first_point.y, vertex_buffer.back().x, vertex_buffer.back().y);
	}

	vertex_buffer.clear();
};

void Context::BufferVertex4f(float x, float y, float z, float w) {
	//tranform to screen
	auto transform_matrix = Matrix::Eye(4);
	auto v = std::make_shared<Vec4>(x, y, z, w);

	transform_matrix->Matmul(Vp_matrix);
	/*for (int i = 0; i < 16; ++i) {
		std::cout << vec_in_screen->GetData()[i] << " ";
	}
	std::cout << "\n";*/

	transform_matrix->Matmul(PVM_matrix);
	//Matrix::PrintMatrix(transform_matrix);
	v->PerspectiveDivide();
	auto vec_in_screen = transform_matrix->Matmul(v);
	//Vec4::PrintVector(vec_in_screen);

	float _tx, _ty;
	_tx = vec_in_screen->x;
	_ty = vec_in_screen->y;
	int tx, ty;
	//TODO perhaps round elsewhere?
	tx = std::round(_tx);
	ty = std::round(_ty);

	switch (drawing_mode) {

	case SGL_POINTS:
		DrawPoint(tx, ty);
		break;

	case SGL_LINES:
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.front().x, vertex_buffer.front().y, tx, ty);
			vertex_buffer.clear();
		} else {
			vertex_buffer.push_back(Point2D{ tx, ty });
		}
		break;

	case SGL_LINE_STRIP: //same as lines but doesn't clear buffer, just pops front
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.front().x, vertex_buffer.front().y, tx, ty);
			vertex_buffer.pop_front();
		}
		//always push new vertex
		vertex_buffer.push_back(Point2D{ tx, ty });
		break;

	case SGL_LINE_LOOP: //same as strip but needs to connect first and last when End() is called
		if (first_point.x == -1) {
			first_point.x = tx;
			first_point.y = ty;
		}
		if (!vertex_buffer.empty()) {
			BresenhamLine(vertex_buffer.back().x, vertex_buffer.back().y, tx, ty);
			vertex_buffer.pop_front();
		}
		vertex_buffer.push_back(Point2D{ tx, ty });
		break;
		//TODO triangles, polygon, etc. (from sglEElementType)
	default:
		break;
	}
};

void Context::BufferVertex3f(float x, float y, float z) {
	BufferVertex4f(x, y, z, 1.0f);
};

void Context::BufferVertex2f(float x, float y) {
	BufferVertex4f(x, y, 0.0f, 1.0f);
};
