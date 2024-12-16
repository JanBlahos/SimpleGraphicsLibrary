
#include "exceptions.h"
#include "context.h"
#include <stdexcept>
#include <iostream>
#include <limits>
#include <execution>

Context::Context(unsigned width, unsigned height) : thread_pool(std::thread::hardware_concurrency()) {
	is_drawing = false;
	win_width = width;
	win_height = height;
	clear_color = Color{ 0.0f, 0.0f, 0.0f };
	drawing_color = clear_color;
	depth_buffer = std::vector<float>(width * height, std::numeric_limits<float>::max());
	color_buffer = std::vector<float>(width * height * 3, 0);
	matrix_stack = MatrixStack();
	Vp_matrix = Matrix::Eye(4);
	PVM_matrix = Matrix::Eye(4);
	depth_test = false;
	drawing_mode = SGL_POINTS;
	filling_mode = SGL_POINT;
	is_drawing = false;
	max_y = 0;
	min_y = height;
	is_setting_scene = false;
	assigning_emmisive_material = false;
	environment_map_set = false;

	unifrom_real_distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
	rng.seed(RNG_SEED);
};

Context::~Context() {
}

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

void Context::SetAreaMode(sglEAreaMode mode) {
	filling_mode = mode;
}

void Context::ClearBuffer(unsigned bitmask) {
	if (is_drawing) {
		throw SGLInvalidOperationException("Cannot call this function while drawing.");
	}
	else if (bitmask > (SGL_DEPTH_BUFFER_BIT | SGL_COLOR_BUFFER_BIT)) {
		throw SGLInvalidValueException("Invalid bitmask passed to the function.");
	} else {
		switch (bitmask) {

		case SGL_COLOR_BUFFER_BIT:
			if (color_buffer.size() == 0) {
				color_buffer.reserve(win_width * win_height * 3);
				color_buffer.resize(win_width * win_height * 3);
			}

			std::for_each(std::execution::par, color_buffer.begin(), color_buffer.end(),
				[&, i = 0](float& val) mutable {
					switch (i % 3) {
						case 0: val = clear_color.r;
							break;
						case 1: val = clear_color.g;
							break;
						case 2: val = clear_color.b;
							break;
					}
					++i;
				});

			/*for (unsigned long i = 0; i < color_buffer.capacity() / 3; ++i) {
				color_buffer[i * 3] = clear_color.r;
				color_buffer[i * 3 + 1] = clear_color.g;
				color_buffer[i * 3 + 2] = clear_color.b;
			}*/

			break;
		case SGL_DEPTH_BUFFER_BIT:
			std::fill(depth_buffer.begin(), depth_buffer.end(),
				std::numeric_limits<float>::max());
			break;
		case (SGL_DEPTH_BUFFER_BIT | SGL_COLOR_BUFFER_BIT):
			ClearBuffer(SGL_COLOR_BUFFER_BIT);
			ClearBuffer(SGL_DEPTH_BUFFER_BIT);
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
	point_size = static_cast<int>(size);
};

unsigned Context::Pixel2Index(unsigned x, unsigned y) {
	return (y * win_width + x) * 3;
}

void Context::SetPixel(unsigned x, unsigned y, float depth) {
	//check if in window
	if (x < win_width && y < win_height && x >= 0 && y >= 0) {
		unsigned i = Pixel2Index(x, y);
		unsigned j = i / 3;
		if (depth_test) {
			if (depth_buffer[j] > depth) {
				depth_buffer[j] = depth;
				color_buffer[i] = drawing_color.r;
				color_buffer[i + 1] = drawing_color.g;
				color_buffer[i + 2] = drawing_color.b;
			}
		} else {
			color_buffer[i] = drawing_color.r;
			color_buffer[i + 1] = drawing_color.g;
			color_buffer[i + 2] = drawing_color.b;
		}
	}
}

void Context::SetPixelNoChecks(unsigned x, unsigned y, Color color) {
	unsigned i = Pixel2Index(x, y);
	color_buffer[i] = color.r;
	color_buffer[i + 1] = color.g;
	color_buffer[i + 2] = color.b;
}

void Context::DrawPoint(int x1, int y1, float depth) {
	// no clear definition of even sized points e.g. 2x2
	int start_x = x1 - point_size / 2;
	int start_y = y1 - point_size / 2;
	
	for (int i = 0; i < point_size; ++i) {
		for (int j = 0; j < point_size; ++j) {
			SetPixel(start_x + i, start_y + j, depth);
		}
	}
}

const bool Context::IsDrawing() {
	return is_drawing;
}

void Context::Enable(sglEEnableFlags what) {
	if (what > SGL_DEPTH_TEST) {
		throw SGLInvalidEnumException("Unrecognized enum.");
	} else if (is_drawing) {
		throw SGLInvalidOperationException("Function called inside drawing sequence.");
	}
	depth_test = true;
}

void Context::Disable(sglEEnableFlags what) {
	if (what > SGL_DEPTH_TEST) {
		throw SGLInvalidEnumException("Unrecognized enum.");
	}
	else if (is_drawing) {
		throw SGLInvalidOperationException("Function called inside drawing sequence.");
	}
	depth_test = false;
}
