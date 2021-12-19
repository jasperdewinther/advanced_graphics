#include "precomp.h"
#include "BVH.h"

BVH::BVH(std::vector<Triangle> vertices, bool use_SAH):
	primitives(vertices)
{
	//from slides
	// create index array
	int N = (int)primitives.size();
	indices = std::make_unique<uint[]>(N);
	for (int i = 0; i < N; i++) indices[i] = i;
	centers = std::make_unique<float3[]>(N);
	for (int i = 0; i < N; i++) centers[i] = vertices[i].get_center();
	// allocate BVH root node
	pool = std::make_unique<BVHNode[]>(N*2-1);
	BVHNode* root = &pool[0];
	uint poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	root->bounds = CalculateBounds(0, N);
	subdivide(root, poolPtr, 0);
}

void BVH::print_details() const
{
	for (int i = 0; i < primitives.size(); i++) printf("vertice: %f %f %f\n", primitives[i].get_center().x, primitives[i].get_center().y, primitives[i].get_center().z);
	for (int i = 0; i < primitives.size(); i++) printf("%i ", indices[i]);
	printf("\n");
	for (int i = 0; i < primitives.size() * 2 - 1; i++) printf("index: %i leftFirst: %i count: %i min: %f %f %f max: %f %f %f\n",
		i,
		pool[i].leftFirst,
		pool[i].count,
		pool[i].bounds.minx,
		pool[i].bounds.miny,
		pool[i].bounds.minz,
		pool[i].bounds.maxx,
		pool[i].bounds.maxy,
		pool[i].bounds.maxz);
	printf("\n");
	for (int i = 0; i < primitives.size() * 2 - 1; i++) {
		if (pool[i].count == 0) {
			printf("\tN_%i_%i -> N_%i_%i\n",
				i, 
				pool[i].count,
				pool[i].leftFirst,
				pool[pool[i].leftFirst].count);
			printf("\tN_%i_%i -> N_%i_%i\n",
				i,
				pool[i].count,
				pool[i].leftFirst+1,
				pool[pool[i].leftFirst+1].count);
		}
		else {
			for (int j = pool[i].leftFirst; j < pool[i].leftFirst + pool[i].count; j++) {
				printf("\tN_%i_%i -> V_%i_%i_%i\n",
					i, 
					pool[i].count,
					(int)(primitives[indices[j]].get_center().x*1000.f+1000.f),
					(int)(primitives[indices[j]].get_center().y*1000.f+1000.f),
					(int)(primitives[indices[j]].get_center().z*1000.f+1000.f));
			}
		}
	}
	for (int i = 0; i < primitives.size(); i++) 
		printf("\tV_%i_%i_%i\n", 
			(int)(primitives[i].get_center().x*1000.f+1000.f), 
			(int)(primitives[i].get_center().y*1000.f+1000.f), 
			(int)(primitives[i].get_center().z*1000.f+1000.f));
	printf("%i vertices\n", primitives.size());
}


void BVH::subdivide(BVHNode* parent, uint& poolPtr, uint indices_start) {
	//printf("parent: %i poolPtr: %i indices_start: %i count: %i\n", ((int)parent - (int)&pool[0])/sizeof(&pool[0]), poolPtr, indices_start, parent->count);
	//printf("bounds calc min: %f %f %f max: %f %f %f\n", parent->bounds.bmin.x, parent->bounds.bmin.y, parent->bounds.bmin.z, parent->bounds.bmax.x, parent->bounds.bmax.y, parent->bounds.bmax.z);
	if (parent->count <= 3) { parent->leftFirst = indices_start;  return; } //todo replace with something better like sah
	parent->leftFirst = poolPtr;
	BVHNode* left = &pool[poolPtr++];
	BVHNode* right = &pool[poolPtr++];
	//set left and right count and left
	int pivot = partition(parent->bounds, indices_start, parent->count);
	left->count = pivot - indices_start;
	left->bounds = CalculateBounds(indices_start, left->count);
	right->count = parent->count - left->count;
	right->bounds = CalculateBounds(pivot, right->count);

	subdivide(left, poolPtr, indices_start);
	subdivide(right, poolPtr, pivot);
	parent->count = 0;

}

int BVH::partition(const AABB& bb, uint start, uint count)
{
	//use bounds to calculate split axis
	//then partition indices
	float diffx = bb.maxx - bb.minx;
	float diffy = bb.maxy - bb.miny;
	float diffz = bb.maxz - bb.minz;
	int split_axis; //index of axis which is split
	float pos; //location of pivot
	if (diffx > diffy) {
		if (diffx > diffz) { split_axis = 0;pos = bb.minx + (diffx/2.f);}
		else { split_axis = 2;pos = bb.minz + (diffz/2.f);}
	}
	else {
		if (diffy > diffz) { split_axis = 1;pos = bb.miny + (diffy/2.f);}
		else { split_axis = 2;pos = bb.minz + (diffz/2.f);}
	}
	int end = start + count-1;
	int i = start;
	if (split_axis == 0) {
		for (; i < end; i++) {
			if (centers[indices[i]].x > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	else if (split_axis == 1) {
		for (; i < end; i++) {
			if (centers[indices[i]].y > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	else {
		for (; i < end; i++) {
			if (centers[indices[i]].z > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	return i;
}


AABB BVH::CalculateBounds(uint first, uint amount) const
{
	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max();
	float minz = std::numeric_limits<float>::max();
	float maxx = std::numeric_limits<float>::min();
	float maxy = std::numeric_limits<float>::min();
	float maxz = std::numeric_limits<float>::min();

	for (uint i = first; i < first + amount; i++) {
		minx = min(primitives[indices[i]].p0.x, minx);
		minx = min(primitives[indices[i]].p1.x, minx);
		minx = min(primitives[indices[i]].p2.x, minx);
		miny = min(primitives[indices[i]].p0.y, miny);
		miny = min(primitives[indices[i]].p1.y, miny);
		miny = min(primitives[indices[i]].p2.y, miny);
		minz = min(primitives[indices[i]].p0.z, minz);
		minz = min(primitives[indices[i]].p1.z, minz);
		minz = min(primitives[indices[i]].p2.z, minz);
		maxx = max(primitives[indices[i]].p0.x, maxx);
		maxx = max(primitives[indices[i]].p1.x, maxx);
		maxx = max(primitives[indices[i]].p2.x, maxx);
		maxy = max(primitives[indices[i]].p0.y, maxy);
		maxy = max(primitives[indices[i]].p1.y, maxy);
		maxy = max(primitives[indices[i]].p2.y, maxy);
		maxz = max(primitives[indices[i]].p0.z, maxz);
		maxz = max(primitives[indices[i]].p1.z, maxz);
		maxz = max(primitives[indices[i]].p2.z, maxz);
	}

	return AABB(minx, miny, minz, maxx, maxy, maxz);
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
		float2 result_right = r.intersects_aabb(pool[n->leftFirst+1].bounds);
		if (result_left.x <= result_left.y) intersect_internal(r, n->leftFirst);
		if (result_right.x <= result_right.y) intersect_internal(r, n->leftFirst+1);
	}
}

void BVH::intersect_internal(Ray& r, int node_index) const { //assumes ray intersects
	BVHNode* n = &pool[node_index];
	if (n->count) {
		//printf("left: %i count: %i\n", n->leftFirst, n->count);
		for (int i = n->leftFirst; i < n->leftFirst + n->count; i++) {
			//printf("i: %i index: %i vertex: %f %f %f\n", i, indices[i], primitives[indices[i]].get_center().x, primitives[indices[i]].get_center().y, primitives[indices[i]].get_center().z);
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
