//---------------------------------------------------------------------------
// context.h
// header for context class and rasterization related methods
//---------------------------------------------------------------------------

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <vector>

#include "sgl.h"
#include "matrixStack.h"

#define NUM_SEGMENTS 40
#define QUADRANT_SEGMENTS 10
#define USE_INCREMENTAL_ERROR false

typedef struct {
	float r;
	float g;
	float b;
} Color;

typedef struct {
	int x;
	int y;
} Point2D;

/// <summary>
/// Class handling matrix stacks and rasterization calls
/// </summary>
class Context
{
public:
	/// <summary>
	/// Initializes context with given width and height
	/// </summary>
	/// <param name="width">raster width</param>
	/// <param name="height">raster height</param>
	Context(unsigned width, unsigned height);

	/// <summary>
	/// Return a pointer to the color buffer
	/// </summary>
	/// <returns></returns>
	float* GetColorBufferPtr(void);

	void SetClearColor(float r, float g, float b);

	/// <summary>
	/// Clears target buffer
	/// </summary>
	/// <param name="buffer_type"> What buffer to clear, represented by
	///  a bitmask. SGL_COLOR_BUFFER_BIT / SGL_DEPTH_BUFFER_BIT</param>
	void ClearBuffer(unsigned buffer_type);

	/// <summary>
	/// Begins the drawing sequence
	/// </summary>
	/// <param name="mode"> What to draw, e.g. SGL_LINE, SGL_TRIANGLES, ...</param>
	void BeginDrawing(sglEElementType mode);

	/// <summary>
	/// End of the drawing sequence
	/// </summary>
	void EndDrawing();

	/// <summary>
	/// Buffers a 4 component vertex into vertex buffer
	/// </summary>
	/// <param name="x"> vertex x coordinate</param>
	/// <param name="y"> vertex y coordinate</param>
	/// <param name="z"> vertex z coordinate</param>
	/// <param name="w"> vertex w coordinate</param>
	void BufferVertex4f(float x, float y, float z, float w);

	/// <summary>
	/// Buffers a 4 component vertex into vertex buffer,
	/// sets w = 1.0f
	/// </summary>
	/// <param name="x"> vertex x coordinate</param>
	/// <param name="y"> vertex y coordinate</param>
	/// <param name="z"> vertex z coordinate</param>
	void BufferVertex3f(float x, float y, float z);

	/// <summary>
	/// Buffers a 4 component vertex into vertex buffer,
	/// sets z = 0.0f and w = 1.0f
	/// </summary>
	/// <param name="x"> vertex x coordinate</param>
	/// <param name="y"> vertex y coordinate</param>
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

	/// <summary>
	/// Sets the drawing color
	/// </summary>
	/// <param name="r"> red component</param>
	/// <param name="g"> green component</param>
	/// <param name="b"> blue component</param>
	void SetDrawingColor(float r, float g, float b);

	/// <summary>
	/// Sets size of points
	/// </summary>
	/// <param name="size"></param>
	void SetPointSize(float size);

	/// <summary>
	/// Sets pixel color in color buffer to current drawing color
	/// </summary>
	/// <param name="x"> pixel x coordinate</param>
	/// <param name="y"> pixel y coordinate</param>
	void SetPixel(unsigned x, unsigned y);

	/// <summary>
	/// Functions as a getter
	/// </summary>
	/// <returns>Whether the library is executing a drawing sequence</returns>
	const bool IsDrawing();

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
	//hw02
	//std::vector<float> depth_buffer;

	//the first specified point during Begin() End() sequence
	Point2D very_first_point;
	Point2D previous_point;
	unsigned num_buffered_vertices;

	Matrix PVM_matrix;
	Matrix Vp_matrix;

	//returns index to the color buffer based off of screen coords
	unsigned Pixel2Index(unsigned x, unsigned y);

	/// <summary>
	/// Attempts to draw a point into color buffer
	/// </summary>
	/// <param name="x1"> Point x coordinate</param>
	/// <param name="y1"> Point y coordinate</param>
	void DrawPoint(int x1, int y1);

	/// <summary>
	/// Used to handle the drawing switch based on the current mode
	/// </summary>
	/// <param name="x1"> Vertex x coordinate</param>
	/// <param name="y1"> Vertex y coordinate</param>
	void DrawVertex(int x1, int y1);

	/// <summary>
	/// Draws a line using the Bresenham algorithm
	/// </summary>
	/// <param name="x1">first point x coordinate</param>
	/// <param name="y1">first point y coordinate</param>
	/// <param name="x2">second point x coordinate</param>
	/// <param name="y2">second point y coordinate</param>
	/// <returns></returns>
	void BresenhamLine(int x1, int y1, int x2, int y2);

	/// <summary>
	/// Draw line which has greater change
	/// in x axis using the Bresenham algorithm
	/// </summary>
	/// <param name="x1">Start point x coordinate. Maker sure that x1 < x2 </param>
	/// <param name="y1"> Start point y coordinate.</param>
	/// <param name="x2"> End point x coordinate. Make sure that x2 > x1 </param>
	/// <param name="y2"> End point y coordinate.</param>
	void PlotLineX(int x1, int y1, int x2, int y2);

	/// <summary>
	/// Draw line which has greater change
	/// in x axis using the Bresenham algorithm
	/// </summary>
	/// <param name="x1">Start point x coordinate. Maker sure that x1 < x2 </param>
	/// <param name="y1"> Start point y coordinate.</param>
	/// <param name="x2"> End point x coordinate. </param>
	/// <param name="y2"> End point y coordinate.  Make sure that y2 > y1</param>
	void PlotLineY(int x1, int y1, int x2, int y2);

	/// <summary>
	/// Transforms given vertex to screen coords using PVM, Vp and perspective division
	/// </summary>
	/// <param name="vertex">Vertex to be transformed</param>
	/// <param name="PVM">PVM matrix</param>
	/// <param name="Vp">Viewport matrix</param>
	static Vec4 VertexToScreen(const Vec4& vertex, const Matrix& PVM, const Matrix& Vp);
};

#endif // !_CONTEXT_H_
