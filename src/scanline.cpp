#include "context.h"
#include <iostream>


void Context::InitScanLine() {
	buckets_per_height = new EdgeBucketList[win_height];
	active_buckets = EdgeBucketList{};
}

void Context::EndScanLine() {
	//TODO delete throws an error, use a unique_ptr instead?
	// not important now
	
	//delete[] buckets_per_height;
	//active_buckets = EdgeBucketList{};
}

void Context::AddToBuckets(float curr_x, int y_lower, float slope, EdgeBucketList& bucket_list) {
	bucket_list.buckets[bucket_list.count].curr_x = curr_x;
	bucket_list.buckets[bucket_list.count].y_lower = y_lower;
	bucket_list.buckets[bucket_list.count].slope = slope;
	bucket_list.count++;
	SortBuckets(active_buckets);
}

void Context::RemoveBucketsByBounds(int height) {
	for (int i = 0; i < active_buckets.count; i++) {
		if (height < active_buckets.buckets[i].y_lower) {
			//std::cout << "Removing bucket : " << active_buckets.buckets[i].curr_x << " at height " << height << std::endl;
			for (int j = i; j < active_buckets.count - 1; j++) {
				active_buckets.buckets[j].y_lower = active_buckets.buckets[j + 1].y_lower;
				active_buckets.buckets[j].curr_x = active_buckets.buckets[j + 1].curr_x;
				active_buckets.buckets[j].slope = active_buckets.buckets[j + 1].slope;
			}
			active_buckets.count--;
			i--;
		}
	}
}
void PrintBucketList(EdgeBucketList& bucket_list) {
	for (int i = 0; i < bucket_list.count; i++) {
		auto& bucket = bucket_list.buckets[i];
		std::cout << "Element:  curr_x:" << bucket.curr_x << " slope " << bucket.slope << " lower_y " << bucket.y_lower << std::endl;
	}
}


void Context::SortBuckets(EdgeBucketList& bucket_list) {
	// TODO: Use some smarter sorting algorithm 
	// than insertion sort to speedup.
	EdgeBucket temp;
	for (int i = 1; i < bucket_list.count; i++) {
		temp.y_lower = bucket_list.buckets[i].y_lower;
		temp.curr_x = bucket_list.buckets[i].curr_x;
		temp.slope = bucket_list.buckets[i].slope;
		int j = i - 1;
		while (j >= 0 && (temp.curr_x < bucket_list.buckets[j].curr_x)) {
			bucket_list.buckets[j + 1].y_lower = bucket_list.buckets[j].y_lower;
			bucket_list.buckets[j + 1].curr_x = bucket_list.buckets[j].curr_x;
			bucket_list.buckets[j + 1].slope = bucket_list.buckets[j].slope;
			j--;
		}
		bucket_list.buckets[j + 1].y_lower = temp.y_lower;
		bucket_list.buckets[j + 1].curr_x = temp.curr_x;
		bucket_list.buckets[j + 1].slope = temp.slope;
	}
}


void Context::AddEdge(float x1, int y1, float x2, int y2) {
	// dont fill horizontal edges
	if (y1 == y2) {
		return;
	}
	/// TODO: not sure if this slope is correct 
	/// or if it should be 1 / this value.
	float slope =  x1 == x2 ? 0.0f : (static_cast<float> (y2 - y1)) / (x2 - x1);
	slope = slope == 0.0f ? 0.0f :  1 / slope;
	int y_start, y_end;
	float x_start, x_end;
	if (y2 > y1) {
		y_start = y2;
		// + 1 since the edges need to be shortened by 1 before filling
		y_end = y1 + 1;
		x_start = x2;
		x_end = x1;
	}
	else {
		y_start = y1;
		// + 1 since the edges need to be shortened by 1 before filling
		y_end = y2 + 1;
		x_end = x2;
		x_start = x1;
	}
	max_y = std::max(y_start, max_y);
	min_y = std::min(y_end, min_y);
	//std::cout << "Adding edge (" << x_start << "," << y_start << ") (" << x_end << "," << y_end << ")" << std::endl;
	//std::cout << "Slope " << slope << std::endl;

	AddToBuckets(x_start, y_end, slope, buckets_per_height[y_start]);
}

void Context::FillLine(int height) {
	for (int i = 0; i < active_buckets.count - 1; i += 2) {
		auto first_point = active_buckets.buckets[i];
		auto second_point = active_buckets.buckets[i + 1];
		int start_x = static_cast<int>(std::floor(first_point.curr_x));
		int end_x = static_cast<int>(std::floor(second_point.curr_x));
		//std::cout << "Bucket pair (" << start_x << "," << height << ") (" << end_x << "," << height << ")" << std::endl;
		for (int x = start_x; x <= end_x; x++) {
			//std::cout << "Filling pixel " << x << " " << height << std::endl;
			SetPixel(x, height);
		}
	}
}

void Context::UpdateBucketsBySlope() {
	for (int i = 0; i < active_buckets.count; i++) {
		auto& bucket = active_buckets.buckets[i];
		bucket.curr_x = bucket.curr_x - bucket.slope;
	}
}


void Context::Fill() {
	for (int h = max_y; h >= min_y; h--) {
		//std::cout << "Height " << h << std::endl;
		//std::cout << "Active_buckets:" << std::endl;
		RemoveBucketsByBounds(h);
		//PrintBucketList(active_buckets);
		UpdateBucketsBySlope();
		bool added = false;
		auto& bucket_list = buckets_per_height[h];
		for (int i = 0; i < bucket_list.count; i++) {
			added = true;
			auto bucket = bucket_list.buckets[i];
			AddToBuckets(bucket.curr_x, bucket.y_lower, bucket.slope, active_buckets);
		}
		FillLine(h);
		if (!added) {
			SortBuckets(active_buckets);
		}
	}
	EndScanLine();
}
