#include "precomp.h"
#include "BVH.h"



template<>
BVH<Triangle>::BVH(std::vector<Triangle> primitives, bool use_SAH):
	primitives(primitives)
{
	Timer t = Timer();
	//from slides
	// create index array
	int N = (int)primitives.size();
	indices = std::make_unique<uint[]>(N);
	for (int i = 0; i < N; i++) indices[i] = i;
	centers = std::make_unique<float3[]>(N);
	for (int i = 0; i < N; i++) centers[i] = primitives[i].get_center();
	// allocate BVH root node
	pool = std::make_unique<BVHNode[]>(N*2-1);
	BVHNode* root = &pool[0];
	std::atomic<uint> poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	root->bounds = CalculateBounds(0, N);
	subdivide(root, poolPtr, 0, use_SAH);
	printf("built bvh in %f seconds\n", t.elapsed());
	printf("%i primitives\n", primitives.size());
	printf("max depth: %i\n", count_depth(&pool[0]));
	printf("node count: %i\n", count_nodes(&pool[0]));
	//write_to_dot_file("out.dot");
}

template<>
BVH<TopBVHNode>::BVH(std::vector<TopBVHNode> primitives, bool use_SAH) :
	primitives(primitives)
{
	Timer t = Timer();
	//from slides
	// create index array
	int N = (int)primitives.size();
	indices = std::make_unique<uint[]>(N);
	for (int i = 0; i < N; i++) indices[i] = i;
	centers = std::make_unique<float3[]>(N);
	for (int i = 0; i < N; i++) centers[i] = primitives[i].pos + primitives[i].obj->pool[0].bounds.get_center();
	// allocate BVH root node
	pool = std::make_unique<BVHNode[]>(N * 2 - 1);
	BVHNode* root = &pool[0];
	std::atomic<uint> poolPtr = 2;
	// subdivide root node
	root->leftFirst = 0;
	root->count = N;
	root->bounds = CalculateBounds(0, N);
	subdivide(root, poolPtr, 0, use_SAH);
	printf("built bvh in %f seconds\n", t.elapsed());
	printf("%i primitives\n", primitives.size());
	printf("max depth: %i\n", count_depth(&pool[0]));
	printf("node count: %i\n", count_nodes(&pool[0]));
}

template<typename T>
void BVH<T>::flatten(BVHNode* node)
{
	BVHNode temp_nodes[section_width];
	if (node->count == 0) {
		temp_nodes[0] = pool[node->leftFirst];
		temp_nodes[1] = pool[node->leftFirst+1];
	}
	int j = 2;
	for (int i = 0; i < section_width; i++) {
		if (temp_nodes[i].count == 0) {
			temp_nodes[j++] = pool[temp_nodes[i].leftFirst];
			temp_nodes[j++] = pool[temp_nodes[i].leftFirst+1];
		}
	}
	for (int i = 0; i < section_width; i++) {
		if (temp_nodes[i] != -1) {
			flatten(temp_nodes[i]);
		}
	}
}

template<typename T>
int BVH<T>::count_depth(BVHNode* node) const {
	return node->count == 0 ? max(count_depth(&pool[node->leftFirst]), count_depth(&pool[node->leftFirst + 1])) + 1 : 1;
}
template<typename T>
int BVH<T>::count_nodes(BVHNode* node) const {
	return node->count == 0 ? count_nodes(&pool[node->leftFirst]) + count_nodes(&pool[node->leftFirst + 1]) : 1;
}
template<>
void BVH<Triangle>::write_to_dot_file(std::string filename) const
{
	FILE* fptr = fopen(filename.c_str(), "w");
	fprintf(fptr, "digraph{\n");

	float scalex = 1.f/(pool[0].bounds.maxx - pool[0].bounds.minx);
	float scaley = 1.f/(pool[0].bounds.maxy - pool[0].bounds.miny);
	float scalez = 1.f/(pool[0].bounds.maxz - pool[0].bounds.minz);

	for (int i = 0; i < primitives.size() * 2 - 1; i++) {
		if (pool[i].count == 0 && i!=1) {
			if (pool[i].leftFirst == 0 || pool[i].leftFirst == 1) {
				continue;
			}
			fprintf(fptr, "\tN_%i_%i -> N_%i_%i\n",
				i,
				pool[i].count,
				pool[i].leftFirst,
				pool[pool[i].leftFirst].count);
			fprintf(fptr, "\tN_%i_%i -> N_%i_%i\n",
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
					(int)(primitives[indices[j]].get_center().x * 1000.f * scalex + 1000.f),
					(int)(primitives[indices[j]].get_center().y * 1000.f * scaley + 1000.f),
					(int)(primitives[indices[j]].get_center().z * 1000.f * scalez + 1000.f));
			}
		}
	}
	fprintf(fptr, "}\n");
	fclose(fptr);
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

	if (left->count > 10000000000 || right->count > 10000000000) {
		printf("left: %i right %i\n", left->count, right->count);
		std::thread t1;
		std::thread t2;
		bool b1 = false;
		bool b2 = false;
		if (left->count > 10000000000) {
			b1 = true;
			t1 = std::thread(&BVH::subdivide, this, left, std::ref(poolPtr), indices_start, use_SAH);
		}
		if (right->count > 10000000000) {
			b2 = true;
			t2 = std::thread(&BVH::subdivide, this, right, std::ref(poolPtr), pivot, use_SAH);
		}
		if (b1) {
			t1.join();
		}
		if (b2) {
			t2.join();
		}
		printf("done with i %i with parent count %i\n", indices_start, parent->count);
	}
	if (left->count <= 10000000000) subdivide(left, poolPtr, indices_start, use_SAH);
	if (right->count <= 1000000000) subdivide(right, poolPtr, pivot, use_SAH);
	parent->count = 0;

}

template<typename T>
int BVH<T>::partition(const AABB& bb, uint start, uint count, bool use_SAH)
{
	if (use_SAH) {
		const int BINS = 8;
		int optimal_axis = 0;
		int optimal_pos = 0;
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
	float maxx = std::numeric_limits<float>::min();
	float maxy = std::numeric_limits<float>::min();
	float maxz = std::numeric_limits<float>::min();

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

	return AABB(minx, miny, minz, maxx, maxy, maxz);
}
template<>
AABB BVH<TopBVHNode>::CalculateBounds(uint first, uint amount) const
{
	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max();
	float minz = std::numeric_limits<float>::max();
	float maxx = std::numeric_limits<float>::min();
	float maxy = std::numeric_limits<float>::min();
	float maxz = std::numeric_limits<float>::min();

	for (uint i = first; i < first + amount; i++) {
		minx = min(primitives[indices[i]].obj->pool[0].bounds.minx + primitives[indices[i]].pos.x, minx);
		maxx = max(primitives[indices[i]].obj->pool[0].bounds.maxx + primitives[indices[i]].pos.x, maxx);
		miny = min(primitives[indices[i]].obj->pool[0].bounds.miny + primitives[indices[i]].pos.y, miny);
		maxy = max(primitives[indices[i]].obj->pool[0].bounds.maxy + primitives[indices[i]].pos.y, maxy);
		minz = min(primitives[indices[i]].obj->pool[0].bounds.minz + primitives[indices[i]].pos.z, minz);
		maxz = max(primitives[indices[i]].obj->pool[0].bounds.maxz + primitives[indices[i]].pos.z, maxz);
	}

	return AABB(minx, miny, minz, maxx, maxy, maxz);
}


template<>
void BVH<Triangle>::intersects(Ray& r) const {
	instantiated_intersect(r);
}
template<>
void BVH<TopBVHNode>::intersects(Ray& r) const {
	instantiated_intersect(r);
}

template<typename T>
void BVH<T>::instantiated_intersect(Ray& r) const {
	BVHNode* n = &pool[0];
	// more from slides
	float2 result = r.intersects_aabb(n->bounds);
	if (result.x > result.y) return;
	r.complexity += 1;
	intersect_internal(r);
}

template<typename T>
void BVH<T>::intersect_internal(Ray& r, int node_index) const { //assumes ray intersects
	BVHNode* n = &pool[node_index];
	if (n->count) {
		//printf("left: %i count: %i\n", n->leftFirst, n->count);
		for (int i = n->leftFirst; i < n->leftFirst + n->count; i++) {
			//printf("i: %i index: %i vertex: %f %f %f\n", i, indices[i], primitives[indices[i]].get_center().x, primitives[indices[i]].get_center().y, primitives[indices[i]].get_center().z);
			intersect_primitive(primitives[indices[i]], r);
		}
	}
	else
	{
		r.complexity += 1;
		float2 result_left = r.intersects_aabb(pool[n->leftFirst].bounds);
		float2 result_right = r.intersects_aabb(pool[n->leftFirst+1].bounds);
		//if (result_left.x <= result_left.y) intersect_internal(r, n->leftFirst);
		//if (result_right.x <= result_right.y) intersect_internal(r, n->leftFirst+1);

		if (result_left.x < result_left.y) {
			if (result_right.x < result_right.y) {
				if (result_left.x < result_right.x) {
					intersect_internal(r, n->leftFirst);
					if (result_right.x < r.t) {
						intersect_internal(r, n->leftFirst + 1);
					}
				}
				else {
					intersect_internal(r, n->leftFirst + 1);
					if (result_left.x < r.t) {
						intersect_internal(r, n->leftFirst);
					}
				}
			}
			else {
				intersect_internal(r, n->leftFirst);
			}
		}
		else if (result_right.x <= result_right.y) intersect_internal(r, n->leftFirst + 1);
	}
}

void intersect_primitive(const TopBVHNode& node, Ray& ray) {
	ray.complexity += 1;
	ray.o -= node.pos;
	node.obj->intersects(ray);
	ray.o += node.pos;
}