#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <vector>

#include "sgl.h"
#include "matrixStack.h"

#define NUM_SEGMENTS 40
#define QUADRANT_SEGMENTS 10

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
	Context(unsigned width, unsigned height);

	float* GetColorBufferPtr(void);

	void SetClearColor(float r, float g, float b);

	void ClearBuffer(unsigned buffer_type);

	void BeginDrawing(sglEElementType mode);

	void EndDrawing();

	void BufferVertex4f(float x, float y, float z, float w);

	void BufferVertex3f(float x, float y, float z);

	void BufferVertex2f(float x, float y);

	/// <summary>
	/// Draw circle using the Bressenham algorithm
	/// </summary>
	/// <param name="x"> X coordinate of the center</param>
	/// <param name="y"> Y coordinate of the center</param>
	/// <param name="z"> Z coordinate of the center</param>
	/// <param name="radius"> radius of the circle</param>
	void BresenhamCircle(float x, float y, float z, float radius);

	/// <summary>
	/// Draw an arc by approximating it with NUM_SEGMENTS * |from - to| \ 2 * PI vertices.
	/// </summary>
	/// <param name="x"> X coordinate of the arc center</param>
	/// <param name="y"> Y coordinate of the arc center </param>
	/// <param name="z"> Z coordinate of the arc center </param>
	/// <param name="radius"> The arc radius </param>
	/// <param name="from"> Starting angle in radians of the arc measured CCW from 
	/// positive X axis. </param>
	/// <param name="to"> Ending angle in radians of the arc measured CCW from 
	/// positive X axis.</param>
	void DrawArc(float x, float y, float z, float radius, float from, float to);

	/// <summary>
	/// Draw an elipse by approximating it with NUM_SEGMENTS vertices
	/// </summary>
	/// <param name="x"> X coordinate of the center</param>
	/// <param name="y"> Y coordinate of the center</param>
	/// <param name="z"> Z coordinate of the center </param>
	/// <param name="a"> Size of the a axis which is aligned with x coordinate axis.</param>
	/// <param name="b"> Size of the b axis which is aligned with y coordinate axis. </param>
	void DrawEllipse(float x, float y, float z, float a, float b);

	void SetDrawingColor(float r, float g, float b);

	void SetPointSize(float size);

	//sets pixel color in color buffer to current drawing color
	void SetPixel(unsigned x, unsigned y);

	MatrixStack matrix_stack;
private:
	unsigned win_width;
	unsigned win_height;

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

	Matrix PVM_matrix;
	Matrix Vp_matrix;

	bool use_incremental_error;

	//returns index to the color buffer based off of screen coords
	unsigned Pixel2Index(unsigned x, unsigned y);

	void DrawPoint(int x1, int y1);

	/// <summary>
	/// Used to handle the drawing switch based on the current mode
	/// </summary>
	/// <param name="x1"> Vertex x coordinate</param>
	/// <param name="y1"> Vertex y coordinate</param>
	void DrawVertex(int x1, int y1);

	void BresenhamLine(int x1, int y1, int x2, int y2);

	void PlotLineX(int x1, int y1, int x2, int y2);

	void PlotLineY(int x1, int y1, int x2, int y2);

	static Vec4 VertexToScreen(const Vec4& vertex, const Matrix& PVM, const Matrix& Vp);

};

#endif // !_CONTEXT_H_
