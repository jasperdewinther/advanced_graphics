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
public:
	std::unique_ptr<uint[]> indices;
	std::unique_ptr<BVHNode[]> pool;
	std::vector<Triangle> primitives;
	std::unique_ptr<float3[]> centers;

	BVH(std::vector<Triangle> triangles, bool use_SAH);


	void intersects(Ray& r) const;
	void print_details() const;
private:
	void subdivide(BVHNode* parent, uint* poolPtr, uint indices_start);
	int partition(const aabb& bb, uint start, uint count);
	void intersect_internal(Ray& r, int node_index = 0) const;
};

aabb CalculateBounds(const std::vector<Triangle>& triangles, uint first, uint amount);