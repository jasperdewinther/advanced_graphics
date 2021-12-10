#pragma once
#include <vector>
#include "Triangle.h"


struct BVHNode {
	aabb bounds;
	int leftFirst;
	int count;
};

class BVH
{
	uint* indices;
	BVHNode* pool;
	const std::vector<Triangle>& primitives;
public:
	BVH(const std::vector<Triangle>& triangles, bool use_SAH);
	void subdivide(BVHNode* parent, uint poolPtr);
	void intersect(Ray& r);
};

aabb CalculateBounds(const std::vector<Triangle>& triangles, uint first, uint amount);