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
	num_buffered_vertices = 0;

	//get viewport and PVM matrices
	const Matrix& VM = matrix_stack.GetViewModelMatrix();
	const Matrix& P = matrix_stack.GetProjectionMatrix();
	PVM_matrix = Matrix::Matmul(P, VM);

	Vp_matrix = matrix_stack.GetViewport();

	if (filling_mode == SGL_FILL && !is_setting_scene) {
		InitScanLine();
	}

	if (is_setting_scene) {
		//insert new polygon and assign last material
		primitive_buffer.push_back(Polygon{});
		primitive_buffer.back().mat_idx = materials.size() - 1;
		next_vertex_idx = 0;
	}
};

void Context::EndDrawing() {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
	}

	// - close line loop, last triangle, etc. depending on mode
	if (!is_setting_scene) {
		if (drawing_mode == SGL_LINE_LOOP) {
			BresenhamLine(very_first_point.x, very_first_point.y, previous_point.x, previous_point.y);
		}
		else if (drawing_mode == SGL_POLYGON) {
			if (filling_mode == SGL_FILL) {
				AddEdge(very_first_point.x, very_first_point.y, very_first_point.z, previous_point.x, previous_point.y, previous_point.z);
				Fill();
				//EndScanLine();
			}
			else {
				BresenhamLine(very_first_point.x, very_first_point.y, previous_point.x, previous_point.y);
			}
		}
	}
	is_drawing = false;
};

void Context::DrawVertex(int x1, int y1, float depth) {
	num_buffered_vertices++;

	switch (drawing_mode) {

	case SGL_POINTS:
		DrawPoint(x1, y1, depth);
		break;

	case SGL_LINES:
		if (num_buffered_vertices % 2 == 0) {
			if (depth_test) {
				BresenhamLine(previous_point.x, previous_point.y, previous_point.z, x1, y1, depth);
			} else {
				BresenhamLine(previous_point.x, previous_point.y, x1, y1);
			}
			
		}
		else {
			//vertex_buffer.push_back(Point2D{ x1, y1 });
			previous_point = Point3D{ x1, y1, depth };
		}
		break;

	case SGL_LINE_STRIP:
		if (num_buffered_vertices != 1) {
			if (depth_test) {
				BresenhamLine(previous_point.x, previous_point.y, previous_point.z, x1, y1, depth);
			} else {
				BresenhamLine(previous_point.x, previous_point.y, x1, y1);
			}
		}
		previous_point = Point3D{ x1, y1, depth };
		break;

	case SGL_LINE_LOOP: //same as strip but needs to connect first and last when End() is called
		if (num_buffered_vertices == 1) {
			very_first_point.x = x1;
			very_first_point.y = y1;
			very_first_point.z = depth;
		} else {
			if (depth_test) {
				BresenhamLine(previous_point.x, previous_point.y, previous_point.z, x1, y1, depth);
			} else {
				BresenhamLine(previous_point.x, previous_point.y, x1, y1);
			}
		}
		previous_point = Point3D{ x1, y1, depth };
		break;
	case SGL_TRIANGLES: //same as polygon
		//not used yet
		break;
	case SGL_POLYGON: //same as line loop?
		if (num_buffered_vertices == 1) {
			very_first_point.x = x1;
			very_first_point.y = y1;
			very_first_point.z = depth;
		} else {
			if (filling_mode == SGL_FILL) {
				AddEdge(previous_point.x, previous_point.y, previous_point.z, x1, y1, depth);
			}
			else {
				if (depth_test) {
					BresenhamLine(previous_point.x, previous_point.y, previous_point.z, x1, y1, depth);
				}
				else {
					BresenhamLine(previous_point.x, previous_point.y, x1, y1);
				}
			}
		}
		previous_point = Point3D{ x1, y1, depth };
		break;
	default:
		break;
	}
}

void Context::AddVertexToPolygon(const Vec3& v) {
	auto& pts = primitive_buffer.back().points;
	pts[next_vertex_idx] = v;
	next_vertex_idx++;
};

void Context::BufferVertex4f(float x, float y, float z, float w) {
	//tranform to screen
	
	if (is_setting_scene) {
		Vec3 v(x, y, z);
		AddVertexToPolygon(v);
		return;
	}

	Vec4 v(x, y, z, w);
	Vec4 transformed_vec = Matrix::Matmul(PVM_matrix, v);
	transformed_vec.PerspectiveDivide();
	Vec4 vec_in_screen = Matrix::Matmul(Vp_matrix, transformed_vec);

	float _tx, _ty;
	_tx = vec_in_screen.x;
	_ty = vec_in_screen.y;
	int tx, ty;
	tx = static_cast<int>(std::floor(_tx));
	ty = static_cast<int>(std::floor(_ty));
	float depth = vec_in_screen.z;
	
	DrawVertex(tx, ty, depth);
};

void Context::BufferVertex3f(float x, float y, float z) {
	BufferVertex4f(x, y, z, 1.0f);
};

void Context::BufferVertex2f(float x, float y) {
	BufferVertex4f(x, y, 0.0f, 1.0f);
};
