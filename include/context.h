#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <vector>

#include "sgl.h"
#include "matrixStack.h"

typedef struct {
	float r;
	float g;
	float b;
} Color;

class Context
{
public:
	Context(int width, int height);

	float* GetColorBufferPtr(void);

	void SetClearColor(float& r, float& g, float& b);

	void ClearBuffer(unsigned buffer_type);

	void BeginDrawing(sglEElementType mode);

	void EndDrawing();

	void BufferVertex4f(float x, float y, float z, float w);

	void BufferVertex3f(float x, float y, float z);

	void BufferVertex2f(float x, float y);

	MatrixStack matrix_stack;
private:
	bool is_drawing;
	Color clear_color;
	sglEElementType drawing_mode;

	std::vector<float> color_buffer;
	std::vector<float> depth_buffer;
	std::vector<float> vertex_buffer;
};

#endif // !_CONTEXT_H_
