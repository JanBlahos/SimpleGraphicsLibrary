
#include "context.h"

Context::Context(int width, int height) {
	is_drawing = false;
	clear_color = Color{ 0.0f, 0.0f, 0.0f };
	color_buffer = std::vector<float>(width * height * 3, 0.0f);
	depth_buffer = std::vector<float>(width * height, std::numeric_limits<float>::max());
};

float* Context::GetColorBufferPtr(void) {
	return color_buffer.data();
}

void Context::SetClearColor(float& r, float& g, float& b, sglEErrorCode& error) {
	if (is_drawing) {
		error = SGL_INVALID_OPERATION;
	} else {
		clear_color = Color{ r, g, b };
	}
}

void Context::ClearBuffer(unsigned buffer_type, sglEErrorCode& error) {
	if (is_drawing) {
		error = SGL_INVALID_OPERATION;
	} else {
		switch (buffer_type) {

		case SGL_COLOR_BUFFER_BIT:
			for (int i = 0; i < color_buffer.size() / 3; ++i) {
				color_buffer[i * 3] = clear_color.r;
				color_buffer[i * 3 + 1] = clear_color.g;
				color_buffer[i * 3 + 2] = clear_color.b;
			}
			break;
		case SGL_DEPTH_BUFFER_BIT:
			std::fill(depth_buffer.begin(), depth_buffer.end(),
				std::numeric_limits<float>::max());
			break;
		default:
			break;
		}
	}
};

void Context::BeginDrawing(sglEElementType mode, sglEErrorCode& error) {
	if (mode >= SGL_LAST_ELEMENT_TYPE) {
		error = SGL_INVALID_ENUM;
		return;
	}
	if (is_drawing) {
		error = SGL_INVALID_OPERATION;
		return;
	}
	is_drawing = true;
	drawing_mode = mode;
};

void Context::EndDrawing(sglEErrorCode& error) {
	if (!is_drawing) {
		error = SGL_INVALID_OPERATION;
		return;
	}
	is_drawing = false;

	// TODO
	// - tranform gathered vertices using current MVP matrix
	// - rasterize current element type using tranformed vertices and color specified by sglColor3f
	// - clear the vertex buffer
};

void Context::BufferVertex4f(float x, float y, float z, float w) {
	//TODO store in vertexbuffer, depending on the implementation change
	// vertexbuffer type
};

void Context::BufferVertex3f(float x, float y, float z) {
	BufferVertex4f(x, y, z, 1.0f);
};

void Context::BufferVertex2f(float x, float y) {
	BufferVertex4f(x, y, 0.0f, 1.0f);
};
