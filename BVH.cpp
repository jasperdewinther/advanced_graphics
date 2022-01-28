#include "precomp.h"
#include "BVH.h"



template<>
BVH<Triangle>::BVH(std::vector<Triangle> prims, bool use_SAH):
	primitives(prims)
{
	BVH_construct(use_SAH);
}
template<>
BVH<TopBVHNode>::BVH(std::vector<TopBVHNode> prims, bool use_SAH):
	primitives(prims)
{
	BVH_construct(use_SAH);
}

template<typename T>
void BVH<T>::BVH_construct(bool use_SAH)
{
	Timer t = Timer();
	//from slides
	// create index array
	int N = (int)primitives.size();
	indices = std::make_unique<uint[]>(N);
	for (int i = 0; i < N; i++) indices[i] = i;
	centers = std::make_unique<float3[]>(N);
	set_centers(N);
	
	// allocate BVH root node
	pool = std::make_unique<BVHNode[]>(N * 2 - 1);
	BVHNode* root = &pool[0];
	std::atomic<uint> poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	root->bounds = CalculateBounds(0, N);
	root->parent = -1;
	subdivide(root, poolPtr, 0, use_SAH);
	printf("built bvh in %f seconds\n", t.elapsed());
	printf("%i primitives\n", primitives.size());
	printf("max depth: %i\n", count_depth(&pool[0]));
	printf("node count: %i\n", count_nodes(&pool[0]));
	elements_of_pool_used = count_nodes(root);
	primitive_count = N;
}

template<>
void BVH<TopBVHNode>::set_centers(uint N)
{
	for (int i = 0; i < N; i++) centers[i] = primitives[i].pos + primitives[i].obj->pool[0].bounds.get_center();
}

template<>
void BVH<Triangle>::set_centers(uint N)
{
	for (int i = 0; i < N; i++) centers[i] = primitives[i].get_center();
}

template<typename T>
int BVH<T>::count_depth(BVHNode* node) const {
	if (node->count != 0) return 1;
	int max_depth = 0;
	for (int i = 0; i < 2; i++)
		max_depth = max(max_depth, count_depth(&pool[node->leftFirst+i])+1);
	return max_depth;
}
template<typename T>
int BVH<T>::count_nodes(BVHNode* node) const {
	if (node->count != 0) return 1;
	int count = 0;
	for (int i = 0; i < 2; i++)
		count += count_nodes(&pool[node->leftFirst + i]) + 1;
	return count;
}

template<typename T>
void BVH<T>::subdivide(BVHNode* parent, std::atomic<uint>& poolPtr, uint indices_start, bool use_SAH) {
	//printf("parent: %i poolPtr: %i indices_start: %i count: %i\n", ((int)parent - (int)&pool[0])/sizeof(&pool[0]), poolPtr, indices_start, parent->count);
	//printf("bounds calc min: %f %f %f max: %f %f %f\n", parent->bounds.bmin.x, parent->bounds.bmin.y, parent->bounds.bmin.z, parent->bounds.bmax.x, parent->bounds.bmax.y, parent->bounds.bmax.z);
	if (parent->count <= 3) { parent->leftFirst = indices_start;  return; } //todo replace with something better like sah
	int index = (poolPtr += 2) - 2;
	parent->leftFirst = index;
	BVHNode* left = &pool[index];
	BVHNode* right = &pool[index+1];
	//set left and right count and left
	int pivot = partition(parent->bounds, indices_start, parent->count, use_SAH);
	left->count = pivot - indices_start;
	left->bounds = CalculateBounds(indices_start, left->count);
	right->count = parent->count - left->count;
	right->bounds = CalculateBounds(pivot, right->count);

	subdivide(left, poolPtr, indices_start, use_SAH);
	subdivide(right, poolPtr, pivot, use_SAH);
	left->parent = parent - pool.get();
	right->parent = parent - pool.get();
	parent->count = 0;
	
}

template<typename T>
int BVH<T>::partition(const AABB& bb, uint start, uint count, bool use_SAH)
{
	if (use_SAH) {
		const int BINS = 8;
		int optimal_axis = 0;
		float optimal_pos = 0;
		float optimal_cost = std::numeric_limits<float>::max();
		for (int axis = 0; axis < 3; axis++) {
			for (int b = 1; b < BINS; b++) {
				int end = start + count - 1;
				int i = start;
				float pos;
				if (axis == 0) pos = lerp(bb.minx, bb.maxx, (float)b / (float)BINS);
				if (axis == 1) pos = lerp(bb.miny, bb.maxy, (float)b / (float)BINS);
				if (axis == 2) pos = lerp(bb.minz, bb.maxz, (float)b / (float)BINS);
				int pivot = partition_shuffle(axis, pos, start, count);

				int bb1_count = pivot - start;
				int bb2_count = count - bb1_count;

				AABB bb1 = CalculateBounds(start, bb1_count);
				AABB bb2 = CalculateBounds(pivot, bb2_count);

				float half_area1 = (bb1.maxx - bb1.minx) * (bb1.maxy - bb1.miny) + (bb1.maxx - bb1.minx) * (bb1.maxz - bb1.minz) + (bb1.maxy - bb1.miny) * (bb1.maxz - bb1.minz);
				float half_area2 = (bb2.maxx - bb2.minx) * (bb2.maxy - bb2.miny) + (bb2.maxx - bb2.minx) * (bb2.maxz - bb2.minz) + (bb2.maxy - bb2.miny) * (bb2.maxz - bb2.minz);
				float cost = half_area1 * bb1_count + half_area2 * bb2_count;
				if (cost < optimal_cost) {
					optimal_axis = axis;
					optimal_pos = pos;
					optimal_cost = cost;
				}
			}
		}
		//printf("optimal %i %f %f\n", optimal_axis, optimal_pos, optimal_cost);
		return partition_shuffle(optimal_axis, optimal_pos, start, count);
	}
	else {
		//use bounds to calculate split axis
		//then partition indices
		float diffx = bb.maxx - bb.minx;
		float diffy = bb.maxy - bb.miny;
		float diffz = bb.maxz - bb.minz;
		int split_axis; //index of axis which is split
		float pos; //location of pivot
		if (diffx > diffy) {
			if (diffx > diffz) { split_axis = 0; pos = bb.minx + (diffx / 2.f); }
			else { split_axis = 2; pos = bb.minz + (diffz / 2.f); }
		}
		else {
			if (diffy > diffz) { split_axis = 1; pos = bb.miny + (diffy / 2.f); }
			else { split_axis = 2; pos = bb.minz + (diffz / 2.f); }
		}
		return partition_shuffle(split_axis, pos, start, count);
	}
}
template<typename T>
int BVH<T>::partition_shuffle(int axis, float pos, uint start, uint count) {
	int end = start + count - 1;
	int i = start;
	if (axis == 0) {
		for (; i < end; i++) {
			if (centers[indices[i]].x > pos) { //we have to swap
				std::swap(indices[i], indices[end]);
				end--;
			}
		}
	}
	else if (axis == 1) {
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

template<>
AABB BVH<Triangle>::CalculateBounds(uint first, uint amount) const
{
	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max();
	float minz = std::numeric_limits<float>::max();
	float maxx = std::numeric_limits<float>::lowest();
	float maxy = std::numeric_limits<float>::lowest();
	float maxz = std::numeric_limits<float>::lowest();

	for (uint i = first; i < first + amount; i++) {
		minx = min(primitives[indices[i]].p0.x, minx);
		miny = min(primitives[indices[i]].p0.y, miny);
		minz = min(primitives[indices[i]].p0.z, minz);
		maxx = max(primitives[indices[i]].p0.x, maxx);
		maxy = max(primitives[indices[i]].p0.y, maxy);
		maxz = max(primitives[indices[i]].p0.z, maxz);
		minx = min(primitives[indices[i]].p1.x, minx);
		miny = min(primitives[indices[i]].p1.y, miny);
		minz = min(primitives[indices[i]].p1.z, minz);
		maxx = max(primitives[indices[i]].p1.x, maxx);
		maxy = max(primitives[indices[i]].p1.y, maxy);
		maxz = max(primitives[indices[i]].p1.z, maxz);
		minx = min(primitives[indices[i]].p2.x, minx);
		miny = min(primitives[indices[i]].p2.y, miny);
		minz = min(primitives[indices[i]].p2.z, minz);
		maxx = max(primitives[indices[i]].p2.x, maxx);
		maxy = max(primitives[indices[i]].p2.y, maxy);
		maxz = max(primitives[indices[i]].p2.z, maxz);
	}
	//printf("%f %f %f %f %f %f\n", minx, maxx, miny, maxy, minz, maxz);
	return AABB(minx - bb_epsilon, miny - bb_epsilon, minz - bb_epsilon, maxx + bb_epsilon, maxy + bb_epsilon, maxz + bb_epsilon);
}
template<>
AABB BVH<TopBVHNode>::CalculateBounds(uint first, uint amount) const
{
	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max();
	float minz = std::numeric_limits<float>::max();
	float maxx = std::numeric_limits<float>::lowest();
	float maxy = std::numeric_limits<float>::lowest();
	float maxz = std::numeric_limits<float>::lowest();

	for (uint i = first; i < first + amount; i++) {
		minx = min(primitives[indices[i]].obj->pool[0].bounds.minx + primitives[indices[i]].pos.x, minx);
		maxx = max(primitives[indices[i]].obj->pool[0].bounds.maxx + primitives[indices[i]].pos.x, maxx);
		miny = min(primitives[indices[i]].obj->pool[0].bounds.miny + primitives[indices[i]].pos.y, miny);
		maxy = max(primitives[indices[i]].obj->pool[0].bounds.maxy + primitives[indices[i]].pos.y, maxy);
		minz = min(primitives[indices[i]].obj->pool[0].bounds.minz + primitives[indices[i]].pos.z, minz);
		maxz = max(primitives[indices[i]].obj->pool[0].bounds.maxz + primitives[indices[i]].pos.z, maxz);
	}

	return AABB(minx - bb_epsilon, miny - bb_epsilon, minz - bb_epsilon, maxx + bb_epsilon, maxy + bb_epsilon, maxz + bb_epsilon);
}