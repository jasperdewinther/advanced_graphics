#pragma once
#include <vector>
#include "Triangle.h"
#include "Ray.h"
#include "BVHNode.h"
#include "AABB.h"



struct bb_intersection_results {
	float2 first;
	float2 second;
};

const float bb_epsilon = 0.000001;

template<typename T>
class BVH
{
public:
	std::unique_ptr<uint[]> indices;
	std::unique_ptr<BVHNode[]> pool;
	std::vector<T> primitives;
	std::unique_ptr<float3[]> centers;
	uint elements_of_pool_used;
	uint primitive_count;
	

	std::unique_ptr<BVHNodeCompressed[]> get_compressed() const;
	BVH() = default;
	BVH(std::vector<T> prims, bool use_SAH);
private:
	void BVH_construct(bool use_SAH);
	void set_centers(uint N);
	int count_depth(BVHNode* node) const;
	int count_nodes(BVHNode* node) const;
	int max_parent(BVHNode* node) const;
	int max_leftFirst(BVHNode* node) const;
	int max_count(BVHNode* node) const;
	void subdivide(BVHNode* parent, std::atomic<uint>& poolPtr, uint indices_start, bool use_SAH);
	int partition(const BVHNode& bb, uint start, uint count, bool use_SAH);
	int partition_shuffle(int axis, float pos, uint start, uint count);
	AABB CalculateBounds(uint first, uint amount) const;
};

struct TopBVHNode {
	BVH<Triangle>* obj;
	float3 pos;
};


using TopLevelBVH = BVH<TopBVHNode>;