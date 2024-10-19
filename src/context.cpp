
#include "exceptions.h"
#include "context.h"
#include <stdexcept>
#include <iostream>
#include <limits>

Context::Context(unsigned width, unsigned height) {
	is_drawing = false;
	win_width = width;
	win_height = height;
	clear_color = Color{ 0.0f, 0.0f, 0.0f };
	color_buffer = std::vector<float>(width * height * 3, 0.0f);
	depth_buffer = std::vector<float>(width * height, std::numeric_limits<float>::max());
	matrix_stack = MatrixStack();
	use_incremental_error = false;
	Vp_matrix = Matrix::Eye(4);
	PVM_matrix = Matrix::Eye(4);
	drawing_mode = SGL_POINTS;
};

float* Context::GetColorBufferPtr(void) {
	return color_buffer.data();
}

void Context::SetClearColor(float r, float g, float b) {
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
			for (unsigned long i = 0; i < color_buffer.size() / 3; ++i) {
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

void Context::SetDrawingColor(float r, float g, float b) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	drawing_color = Color {r, g, b};
};

void Context::SetPointSize(float size) {
	if (size < 0) {
		throw SGLInvalidValueException("Size must be a positive number.");
	}
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	point_size = size;
};

unsigned Context::Pixel2Index(unsigned x, unsigned y) {
	return (y * win_width + x) * 3;
}

void Context::SetPixel(unsigned x, unsigned y) {
	//check if in window
	if (x < win_width && y < win_height&& x >= 0 && y >= 0) {
		unsigned i = Pixel2Index(x, y);
		color_buffer[i] = drawing_color.r;
		color_buffer[i + 1] = drawing_color.g;
		color_buffer[i + 2] = drawing_color.b;
	}
}

void Context::DrawPoint(int x1, int y1) {
	// TODO perhaps shift a bit
	// no clear definition of even sized points e.g. 2x2
	int start_x = x1 - point_size / 2;
	int start_y = y1 - point_size / 2;
	
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			SetPixel(start_x + i, start_y + j);
		}
	}
}
