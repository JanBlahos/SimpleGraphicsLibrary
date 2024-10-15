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

typedef struct {
	int x;
	int y;
} Point2D;

class Context
{
public:
	Context(int width, int height);

	float* GetColorBufferPtr(void);

	void SetClearColor(float r, float g, float b);

	void ClearBuffer(unsigned buffer_type);

	void BeginDrawing(sglEElementType mode);

	void EndDrawing();

	void BufferVertex4f(float x, float y, float z, float w);

	void BufferVertex3f(float x, float y, float z);

	void BufferVertex2f(float x, float y);

	void SetDrawingColor(float r, float g, float b);

	void SetPointSize(float size);

	//sets pixel color in color buffer to current drawing color
	void SetPixel(unsigned x, unsigned y);

	MatrixStack matrix_stack;
private:
	int win_width;
	int win_height;

	bool is_drawing;
	Color clear_color;
	sglEElementType drawing_mode;

	Color drawing_color;
	int point_size;

	std::vector<float> color_buffer;
	std::vector<float> depth_buffer;

	std::deque<Point2D> vertex_buffer;
	//the first specified point during Begin() End() sequence
	Point2D first_point;

	std::shared_ptr<Matrix> PVM_matrix;
	std::shared_ptr<Matrix> Vp_matrix;

	bool use_incremental_error;

	//returns index to the color buffer based off of screen coords
	unsigned Pixel2Index(unsigned x, unsigned y);

	void DrawPoint(int x1, int y1);

	void BresenhamLine(int x1, int y1, int x2, int y2);

	void PlotLineX(int x1, int y1, int x2, int y2);

	void PlotLineY(int x1, int y1, int x2, int y2);

};

#endif // !_CONTEXT_H_
