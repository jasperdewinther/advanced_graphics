#pragma once


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
};