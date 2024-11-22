//---------------------------------------------------------------------------
// scanline.h
// header file containing the scanline algorithm
// that is first given edges that are stored in a list
// sorted by x for each height. And then when scaline fill
// is called it draws a filled polygon.
//---------------------------------------------------------------------------
#ifndef _SCANLINE_H_
#define _SCANLINE_H_
#define MAX_VERTICES 100

typedef struct edgebucket {
	//int y_upper;
	int y_lower;
	float curr_x;
	float slope;
}EdgeBucket;

typedef struct edgebucketlist {
	int count;
	EdgeBucket buckets[MAX_VERTICES];

}EdgeBucketList;

class ScanLineFill {
public:
	/// <summary>
	/// Construct scanline fill algorithm class 
	/// </summary>
	/// <param name="height"> Height of the window used for drawing</param>
	ScanLineFill(int height);

	~ScanLineFill();

	/// <summary>
	/// Add edge to the edge list
	/// </summary>
	/// <param name="x1"> X coordinate of first point </param>
	/// <param name="y1"> Y coordinate of the first point </param>
	/// <param name="x2"> X coordinate of the second point </param>
	/// <param name="y2"> Y coordinate of the second point </param>
	void AddEdge(float x1, unsigned y1, float x2, unsigned y2);

	/// <summary>
	/// Fill the current polygon
	/// </summary>
	void Fill();
private:
	EdgeBucketList* buckets_per_height;
	EdgeBucketList active_buckets;
	//int height_;
	
	/// <summary>
	/// Sort buckets in the given list by their x coordinate
	/// </summary>
	/// <param name="height"></param>
	void SortBuckets(EdgeBucketList& bucket_list);

	/// <summary>
	/// Fill line by connecting the pairs 
	/// of buckets in active buckets
	/// </summary>
	void FillLine(unsigned height);

	/// <summary>
	/// Add a new bucket to a guven bucket list
	/// </summary>
	/// <param name="buckets"></param>
	void AddToBuckets(float start_x, int y_lower, float slope, EdgeBucketList& bucket_list);

	/// <summary>
	/// Remove buckets where height < y_lower or 
	/// </summary>
	/// <param name="height"></param>
	void RemoveBucketsByBounds(unsigned height);
};

#endif