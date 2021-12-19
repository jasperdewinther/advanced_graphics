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
	subdivide(root, poolPtr, 0, use_SAH);
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
	
	printf("%i vertices\n", primitives.size());

	printf("max depth: %i\n", count_depth(&pool[0]));
}

int BVH::count_depth(BVHNode* parent) const {
	return parent->count == 0 ? max(count_depth(&pool[parent->leftFirst]), count_depth(&pool[parent->leftFirst + 1])) + 1 : 1;
}

void BVH::write_to_dot_file(std::string filename)
{
	FILE* fptr = fopen(filename.c_str(), "w");
	for (int i = 0; i < primitives.size() * 2 - 1; i++) {
		if (pool[i].count == 0) {
			fprintf(fptr,"\tN_%i_%i -> N_%i_%i\n",
				i,
				pool[i].count,
				pool[i].leftFirst,
				pool[pool[i].leftFirst].count);
			fprintf(fptr,"\tN_%i_%i -> N_%i_%i\n",
				i,
				pool[i].count,
				pool[i].leftFirst + 1,
				pool[pool[i].leftFirst + 1].count);
		}
		else {
			for (int j = pool[i].leftFirst; j < pool[i].leftFirst + pool[i].count; j++) {
				fprintf(fptr, "\tN_%i_%i -> V_%i_%i_%i\n",
					i,
					pool[i].count,
					(int)(primitives[indices[j]].get_center().x * 1000.f + 1000.f),
					(int)(primitives[indices[j]].get_center().y * 1000.f + 1000.f),
					(int)(primitives[indices[j]].get_center().z * 1000.f + 1000.f));
			}
		}
	}
	for (int i = 0; i < primitives.size(); i++)
		fprintf(fptr, "\tV_%i_%i_%i\n",
			(int)(primitives[i].get_center().x * 1000.f + 1000.f),
			(int)(primitives[i].get_center().y * 1000.f + 1000.f),
			(int)(primitives[i].get_center().z * 1000.f + 1000.f));
	fclose(fptr);
}


void BVH::subdivide(BVHNode* parent, uint& poolPtr, uint indices_start, bool use_SAH) {
	//printf("parent: %i poolPtr: %i indices_start: %i count: %i\n", ((int)parent - (int)&pool[0])/sizeof(&pool[0]), poolPtr, indices_start, parent->count);
	//printf("bounds calc min: %f %f %f max: %f %f %f\n", parent->bounds.bmin.x, parent->bounds.bmin.y, parent->bounds.bmin.z, parent->bounds.bmax.x, parent->bounds.bmax.y, parent->bounds.bmax.z);
	if (parent->count <= 3) { parent->leftFirst = indices_start;  return; } //todo replace with something better like sah
	parent->leftFirst = poolPtr;
	BVHNode* left = &pool[poolPtr++];
	BVHNode* right = &pool[poolPtr++];
	//set left and right count and left
	int pivot = partition(parent->bounds, indices_start, parent->count, use_SAH);
	left->count = pivot - indices_start;
	left->bounds = CalculateBounds(indices_start, left->count);
	right->count = parent->count - left->count;
	right->bounds = CalculateBounds(pivot, right->count);

	subdivide(left, poolPtr, indices_start, use_SAH);
	subdivide(right, poolPtr, pivot, use_SAH);
	parent->count = 0;

}

int BVH::partition(const AABB& bb, uint start, uint count, bool use_SAH)
{
	if (use_SAH) {
		const int BINS = 8;
		int optimal_axis = 0;
		int optimal_pos = 0;
		float optimal_cost = std::numeric_limits<float>::max();
		for (int axis = 0; axis < 3; axis++) {
			for (int b = 0; b < BINS; b++) {
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

int BVH::partition_shuffle(int axis, float pos, uint start, uint count) {
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
