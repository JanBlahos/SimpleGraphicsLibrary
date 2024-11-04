#include "context.h"
#include <iostream>


void Context::InitScanLine() {
	buckets_per_height = std::unique_ptr<EdgeBucketList[]>(new EdgeBucketList[win_height]);
	active_buckets = EdgeBucketList{};
}

void Context::EndScanLine() {
	active_buckets = EdgeBucketList{};
}

void Context::AddToBuckets(float start_x, int y_lower, float slope, EdgeBucketList& bucket_list, float start_z, float depth_slope) {
	bucket_list.buckets[bucket_list.count].curr_x = start_x;
	bucket_list.buckets[bucket_list.count].y_lower = y_lower;
	bucket_list.buckets[bucket_list.count].slope = slope;
	//added for depth interpolation
	bucket_list.buckets[bucket_list.count].curr_z = start_z;
	bucket_list.buckets[bucket_list.count].depth_slope = depth_slope;
	bucket_list.count++;
	SortBuckets(active_buckets);
}

void Context::RemoveBucketsByBounds(int height) {
	for (int i = 0; i < active_buckets.count; i++) {
		if (height < active_buckets.buckets[i].y_lower) {
			for (int j = i; j < active_buckets.count - 1; j++) {
				active_buckets.buckets[j].y_lower = active_buckets.buckets[j + 1].y_lower;
				active_buckets.buckets[j].curr_x = active_buckets.buckets[j + 1].curr_x;
				active_buckets.buckets[j].slope = active_buckets.buckets[j + 1].slope;
				//added for depth interpolation
				active_buckets.buckets[j].curr_z = active_buckets.buckets[j + 1].curr_z;
				active_buckets.buckets[j].depth_slope = active_buckets.buckets[j + 1].depth_slope;
			}
			active_buckets.count--;
			i--;
		}
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
		//added for depth interpolation
		temp.curr_z = bucket_list.buckets[i].curr_z;
		temp.depth_slope = bucket_list.buckets[i].depth_slope;
		int j = i - 1;
		while (j >= 0 && (temp.curr_x < bucket_list.buckets[j].curr_x)) {
			bucket_list.buckets[j + 1].y_lower = bucket_list.buckets[j].y_lower;
			bucket_list.buckets[j + 1].curr_x = bucket_list.buckets[j].curr_x;
			bucket_list.buckets[j + 1].slope = bucket_list.buckets[j].slope;
			//added for depth interpolation
			bucket_list.buckets[j + 1].curr_z = bucket_list.buckets[j].curr_z;
			bucket_list.buckets[j + 1].depth_slope = bucket_list.buckets[j].depth_slope;
			j--;
		}
		bucket_list.buckets[j + 1].y_lower = temp.y_lower;
		bucket_list.buckets[j + 1].curr_x = temp.curr_x;
		bucket_list.buckets[j + 1].slope = temp.slope;
		//added for depth interpolation
		bucket_list.buckets[j + 1].curr_z = temp.curr_z;
		bucket_list.buckets[j + 1].depth_slope = temp.depth_slope;
	}
}


void Context::AddEdge(float x1, int y1, float z1, float x2, int y2, float z2) {
	// dont fill horizontal edges
	if (y1 == y2) {
		return;
	}
	float slope =  x1 == x2 ? 0.0f : (x2 - x1) / (static_cast<float> (y2 - y1));
	//slope = slope == 0.0f ? 0.0f :  1 / slope;

	float z_slope = z1 == z2 ? 0.0f : (z2 - z1) / (static_cast<float> (y2 - y1));
	//z_slope = z_slope == 0.0f ? 0.0f : 1 / slope;

	int y_start, y_end;
	float x_start;
	float z_start;
	//std::cout << "Adding edge ( " << x1 << "," << y1 << "," << z1 << ") (" << x2 << "," << y2 << "," << z2 << ")" << std::endl;
	//std::cout << "Z slope: " << z_slope << std::endl;
	
	if (y2 > y1) {
		y_start = y2;
		// + 1 since the edges need to be shortened by 1 before filling
		y_end = y1 + 1;
		x_start = x2;
		z_start = z2;
	}
	else {
		y_start = y1;
		// + 1 since the edges need to be shortened by 1 before filling
		y_end = y2 + 1;
		x_start = x1;
		z_start = z1;
	}
	max_y = std::max(y_start, max_y);
	min_y = std::min(y_end, min_y);

	AddToBuckets(x_start, y_end, slope, buckets_per_height[y_start], z_start, z_slope);
}

void Context::FillLine(int height) {
	for (int i = 0; i < active_buckets.count - 1; i += 2) {
		auto first_point = active_buckets.buckets[i];
		auto second_point = active_buckets.buckets[i + 1];
		int start_x = static_cast<int>(std::floor(first_point.curr_x));
		int end_x = static_cast<int>(std::floor(second_point.curr_x));
		float z1 = first_point.curr_z;
		float z2 = second_point.curr_z;
		float depth_step;
		if (end_x == start_x) {
			depth_step = 0.0f;
		} else {
			depth_step = (z2 - z1) / (end_x - start_x);
		}
		float current_depth = z1;
		for (int x = start_x; x <= end_x; x++) {
			//interpolate depth again
			SetPixel(x, height, current_depth);
			current_depth += depth_step;
		}
	}
}

void Context::UpdateBucketsBySlope() {
	for (int i = 0; i < active_buckets.count; i++) {
		auto& bucket = active_buckets.buckets[i];
		bucket.curr_x = bucket.curr_x - bucket.slope;
		//update z
		bucket.curr_z = bucket.curr_z - bucket.depth_slope;
	}
}


void Context::Fill() {
	for (int h = max_y; h >= min_y; h--) {
		RemoveBucketsByBounds(h);
		UpdateBucketsBySlope();
		bool added = false;
		auto& bucket_list = buckets_per_height[h];
		for (int i = 0; i < bucket_list.count; i++) {
			added = true;
			auto bucket = bucket_list.buckets[i];
			AddToBuckets(bucket.curr_x, bucket.y_lower, bucket.slope, active_buckets, bucket.curr_z, bucket.depth_slope);
		}
		FillLine(h);
		if (!added) {
			SortBuckets(active_buckets);
		}
	}
	EndScanLine();
}

void PrintBucketList(EdgeBucketList& bucket_list) {
	for (int i = 0; i < bucket_list.count; i++) {
		auto& bucket = bucket_list.buckets[i];
		std::cout << "Element:  curr_x:" << bucket.curr_x << " slope " << bucket.slope << " lower_y " << bucket.y_lower << std::endl;
	}
}
