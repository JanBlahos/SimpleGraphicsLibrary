
#include "exceptions.h"
#include "context.h"
#include <limits>

Context::Context(int width, int height) {
	is_drawing = false;
	clear_color = Color{ 0.0f, 0.0f, 0.0f };
	color_buffer = std::vector<float>(width * height * 3, 0.0f);
	depth_buffer = std::vector<float>(width * height, std::numeric_limits<float>::max());
};

float* Context::GetColorBufferPtr(void) {
	return color_buffer.data();
}

void Context::SetClearColor(float& r, float& g, float& b) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	} else {
		clear_color = Color{ r, g, b };
	}
}

void Context::ClearBuffer(unsigned buffer_type) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	else if (buffer_type > SGL_DEPTH_BUFFER_BIT) {
		throw SGLInvalidValueException("Invalid bitmask passed to the function.");
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

void Context::BeginDrawing(sglEElementType mode) {
	if (mode >= SGL_LAST_ELEMENT_TYPE) {
		throw SGLInvalidEnumException("This drawing mode doesn't exist.");
	}
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	is_drawing = true;
	drawing_mode = mode;
};

void Context::EndDrawing() {
	if (!is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while not drawing.");
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
