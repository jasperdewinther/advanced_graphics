#include "precomp.h"
#include "BVH.h"

BVH::BVH(std::vector<Triangle> vertices, bool use_SAH):
	primitives(vertices)
{
	//from slides
	// create index array
	int N = (int)primitives.size();
	indices = new uint[N];
	for (int i = 0; i < N; i++) indices[i] = i;
	// allocate BVH root node
	pool = new BVHNode[N*2-1];
	BVHNode* root = &pool[0];
	uint poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	subdivide(root, poolPtr, 0);
}

BVH::~BVH()
{
	delete[] indices;
	delete[] pool;
}


void BVH::subdivide(BVHNode* parent, uint& poolPtr, uint indices_start) {
	parent->bounds = CalculateBounds(primitives, indices_start, parent->count);
	if (parent->count <= 3) return; //todo replace with something better like sah
	parent->leftFirst = poolPtr;
	BVHNode* left = &pool[poolPtr++];
	BVHNode* right = &pool[poolPtr++];
	//set left and right count and left
	int pivot = partition(parent->bounds, indices_start, parent->count);
	left->count = pivot;
	right->count = parent->count - pivot;
	subdivide(left, poolPtr, indices_start);
	subdivide(right, poolPtr, pivot);
	parent->count = 0;

}

int BVH::partition(const aabb& bb, uint start, uint count)
{
	//use bounds to calculate split axis
	//then partition indices
	float3 diff = bb.bmax3 - bb.bmin3;
	int split_axis; //index of axis which is split
	float pos; //location of pivot
	if (diff.x > diff.y) {
		if (diff.x > diff.z) { split_axis = 0;pos = bb.bmin3.x + diff.x;}
		else { split_axis = 2;pos = bb.bmin3.z + diff.z;}
	}
	else {
		if (diff.y > diff.z) { split_axis = 1;pos = bb.bmin3.y + diff.y;}
		else { split_axis = 2;pos = bb.bmin3.z + diff.z;}
	}
	int end = start + count - 1;
	int i = start;
	if (split_axis == 0) {
		for (; i < end-1; i++) {
			if (primitives[indices[i]].get_centre().x > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	else if (split_axis == 1) {
		for (; i < end-1; i++) {
			if (primitives[indices[i]].get_centre().y > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	else {
		for (; i < end-1; i++) {
			if (primitives[indices[i]].get_centre().z > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	return i;
}


aabb CalculateBounds(const std::vector<Triangle>& triangles, uint first, uint amount)
{
	float3 minp = float3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	float3 maxp = float3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	for (uint i = first; i < first + amount; i++) {
		minp.x = min(triangles[i].p0.x, minp.x);
		minp.y = min(triangles[i].p0.y, minp.y);
		minp.z = min(triangles[i].p0.z, minp.z);
		minp.x = min(triangles[i].p1.x, minp.x);
		minp.y = min(triangles[i].p1.y, minp.y);
		minp.z = min(triangles[i].p1.z, minp.z);
		minp.x = min(triangles[i].p2.x, minp.x);
		minp.y = min(triangles[i].p2.y, minp.y);
		minp.z = min(triangles[i].p2.z, minp.z);
		maxp.x = max(triangles[i].p0.x, maxp.x);
		maxp.y = max(triangles[i].p0.y, maxp.y);
		maxp.z = max(triangles[i].p0.z, maxp.z);
		maxp.x = max(triangles[i].p1.x, maxp.x);
		maxp.y = max(triangles[i].p1.y, maxp.y);
		maxp.z = max(triangles[i].p1.z, maxp.z);
		maxp.x = max(triangles[i].p2.x, maxp.x);
		maxp.y = max(triangles[i].p2.y, maxp.y);
		maxp.z = max(triangles[i].p2.z, maxp.z);
	}
	return aabb(minp, maxp);
}


void BVH::intersects(Ray& r) const{
	BVHNode* n = &pool[0];
	// more from slides
	float2 result = r.intersects_aabb(n->bounds);
	if (result.x > result.y) return;
	if (n->count) {
		for (int i = n->leftFirst; i < n->leftFirst + n->count; i++) {
			primitives[indices[i]].intersects(r);
		}
	}
	else
	{
		float2 result_left = r.intersects_aabb(pool[n->leftFirst].bounds);
		float2 result_right = r.intersects_aabb(pool[n->leftFirst + 1].bounds);
		if (result_left.x > result_left.y) intersect_internal(r, n->leftFirst);
		if (result_right.x > result_right.y) intersect_internal(r, n->leftFirst + 1);
	}
}
void BVH::intersect_internal(Ray& r, int node_index) const { //assumes ray intersects
	BVHNode* n = &pool[node_index];
	if (n->count) {
		for (int i = n->leftFirst; i < n->leftFirst + n->count; i++) {
			primitives[indices[i]].intersects(r);
		}
	}
	else
	{
		float2 result_left = r.intersects_aabb(pool[n->leftFirst].bounds);
		float2 result_right = r.intersects_aabb(pool[n->leftFirst+1].bounds);
		if (result_left.x <= result_left.y) intersect_internal(r, n->leftFirst);
		if (result_right.x <= result_right.y) intersect_internal(r, n->leftFirst+1);
	}
}
