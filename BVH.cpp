#include "precomp.h"
#include "BVH.h"

BVH::BVH(const std::vector<Triangle>& vertices, bool use_SAH):
	primitives(vertices)
{
	//from slides
	// create index array
	int N = primitives.size();
	indices = new uint[N];
	for (int i = 0; i < N; i++) indices[i] = i;
	// allocate BVH root node
	pool = new BVHNode[N*2-1];
	BVHNode* root = &pool[0];
	uint poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	root->bounds = CalculateBounds(primitives, root->leftFirst, root->count);
	subdivide(root, poolPtr);
}

void BVH::subdivide(BVHNode* parent, uint poolPtr) {
	if (parent->count <= 3) return;

}

void BVH::intersect(Ray& r)
{
	BVHNode* n = &pool[0];
	// more from slides
	if (!r.intersects_aabb(n.bounds)) return;
	if (n->count)
	{
		IntersectPrimitives();
	}
	else
	{
		pool[n->leftFirst].Traverse(r);
		pool[n->leftFirst + 1].Traverse(r);
	}
}

aabb CalculateBounds(const std::vector<Triangle>& triangles, uint first, uint amount)
{
	float3 minp = float3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	float3 maxp = float3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	for (uint i = first; i < first + amount; i++) {
		minp.x = min(triangles[i].p0.x, minp.x);
		minp.y = min(triangles[i].p0.y, minp.y);
		minp.z = min(triangles[i].p0.z, minp.z);
		maxp.x = max(triangles[i].p0.x, maxp.x);
		maxp.y = max(triangles[i].p0.y, maxp.y);
		maxp.z = max(triangles[i].p0.z, maxp.z);
	}
	return aabb(minp, maxp);
}
