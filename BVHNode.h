#pragma once

struct BVHNodeCompressed {
	float minx;
	float miny;
	float minz;
	float maxx;
	float maxy;
	float maxz;
	int leftFirst;
	int count; // first 8 bits are for count, the other 24 are for parent (max 16777216)
};

struct BVHNode {
	float minx;
	float miny;
	float minz;
	float maxx;
	float maxy;
	float maxz;
	int leftFirst;
	int count;
	int parent;
	float3 get_center();
	BVHNodeCompressed compress();
};

