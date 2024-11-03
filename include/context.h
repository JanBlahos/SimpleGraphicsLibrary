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
#define MAX_VERTICES 100

typedef struct edgebucket {
	//int y_upper;
	int y_lower;
	float curr_x;
	float slope;
}EdgeBucket;

typedef struct edgebucketlist {
	int count = 0;
	EdgeBucket buckets[MAX_VERTICES];

}EdgeBucketList;

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

	~Context();

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
	/// Sets the current primitive fill mode
	/// </summary>
	/// <param name="mode"></param>
	void SetAreaMode(sglEAreaMode mode);

	/// <summary>
	/// Enables functions given by bitmask
	/// </summary>
	/// <param name="mode"></param>
	void Enable(sglEEnableFlags what);

	/// <summary>
	/// Disables functions given by bitmask
	/// </summary>
	/// <param name="mode"></param>
	void Disable(sglEEnableFlags what);

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


	///***
	/// SCAN LINE ALGORITHM FUNCTIONS
	/// ***///

	/// <summary>
	/// Add edge to the scan line algorithm edge list
	/// </summary>
	/// <param name="x1"> X coordinate of first point </param>
	/// <param name="y1"> Y coordinate of the first point </param>
	/// <param name="x2"> X coordinate of the second point </param>
	/// <param name="y2"> Y coordinate of the second point </param>
	void AddEdge(float x1, int y1, float x2, int y2);

	/// <summary>
	/// Fill the current polygon using the scan line algorithm
	/// </summary>
	void Fill();

	MatrixStack matrix_stack;
private:
	unsigned win_width;
	unsigned win_height;

	///****
	/// SCANLINE ALGORITHM VARIABLES
	///****
	std::unique_ptr<EdgeBucketList[]> buckets_per_height;
	EdgeBucketList active_buckets;
	int min_y;
	int max_y;

	bool is_drawing;
	Color clear_color;
	sglEElementType drawing_mode;
	sglEAreaMode filling_mode;
	bool depth_test;

	Color drawing_color;
	int point_size;

	std::vector<float> color_buffer;
	std::vector<float> depth_buffer;

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

	/// <summary>
	/// Helper function for drawing 8 symmetrical vertices/lines for circle
	/// </summary>
	/// <param name="centerx"> Center x coordinate</param>
	/// <param name="centery"> Center y coordinate</param>
	/// <param name="x"> Current x coordinate in second octant</param>
	/// <param name="y"> Current y coordinate in second octant</param>
	/// <param name="lines"> If true draw lines from center otherwise draw only points </param>
	void DrawSymmetrical(int centerx, int centery, int x, int y, bool lines);

	///***
	/// SCAN LINE ALGORITHM FUNCTIONS
	/// ***///

	/// <summary>
	/// Initialize scanline algorithm to prepare it 
	/// for polygon edge addition
	/// </summary>
	void InitScanLine();

	/// <summary>
	/// When a polygon has been 
	/// succesfully filled destroy currently stored data
	/// to prepare for the next InitScanLine call.
	/// </summary>
	void EndScanLine();

	/// <summary>
	/// Sorts edge buckets used for filling in 
	/// the given list by their x coordinate
	/// </summary>
	/// <param name="height"></param>
	void SortBuckets(EdgeBucketList& bucket_list);

	/// <summary>
	/// Fill line by connecting the pairs 
	/// of buckets in the active buckets lists
	/// </summary>
	/// <param name="height"> Current line height</param>
	void FillLine(int height);

	/// <summary>
	/// Add a new bucket to a given bucket list
	/// </summary>
	/// <param name="start_x"> Bucket current x</param>
	/// <param name="y_lower"> Y coordinate of the edge end </param>
	/// <param name="slope"> Edge slope in used to update the x coordinate</param>
	/// <param name="bucket_list"> List in which to construct the new bucket</param>
	void AddToBuckets(float start_x, int y_lower, float slope, EdgeBucketList& bucket_list);

	/// <summary>
	/// Remove buckets from the active buckets list
	/// where height < y_lower
	/// </summary>
	/// <param name="height"> Current line height </param>
	void RemoveBucketsByBounds(int height);

	/// <summary>
	/// For each bucket in the currently active list move the current
	/// x by slope
	/// </summary>
	void UpdateBucketsBySlope();
};

#endif // !_CONTEXT_H_
