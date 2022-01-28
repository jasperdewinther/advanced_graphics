#include "common.cl"



void intersect_primitive(const Triangle& tri, Ray& ray)
{
	//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	const float EPSILON = 0.0000001;
	float3 edge1 = tri.p1 - tri.p0;
	float3 edge2 = tri.p2 - tri.p0;
	float3 h = cross(ray.d, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) return;    // This ray is parallel to this triangle.
	float f = 1.0 / a;
	float3 s = ray.o - tri.p0;
	float u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) return;
	float3 q = cross(s, edge1);
	float v = f * dot(ray.d, q);
	if (v < 0.0 || u + v > 1.0) return;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * dot(edge2, q);
	if (t > 0.f && ray.t > t) {
		ray.t = t;
		ray.hitptr = &tri;
	}
}

float2 intersects_aabb(const AABB& box, const struct Ray& r)
{
	//https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
	float tMinx = (box.minx - r.o.x) * r.invDir.x;
	float tMiny = (box.miny - r.o.y) * r.invDir.y;
	float tMinz = (box.minz - r.o.z) * r.invDir.z;
	float tMaxx = (box.maxx - r.o.x) * r.invDir.x;
	float tMaxy = (box.maxy - r.o.y) * r.invDir.y;
	float tMaxz = (box.maxz - r.o.z) * r.invDir.z;
	float t1x = min(tMinx, tMaxx);
	float t1y = min(tMiny, tMaxy);
	float t1z = min(tMinz, tMaxz);
	float t2x = max(tMinx, tMaxx);
	float t2y = max(tMiny, tMaxy);
	float t2z = max(tMinz, tMaxz);
	float tNear = max(max(t1x, t1y), t1z);
	float tFar = min(min(t2x, t2y), t2z);
	return float2(tNear, tFar);
}

void intersect_bot(Ray& r, int obj_index) const { //assumes ray intersects
	//https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
	const uint model_start = m_model_bvh_starts[obj_index];

	const BVHNode* last = nullptr;
	const BVHNode* current = &m_bvh_nodes[model_start];
	const BVHNode* near_node = nullptr;
	const BVHNode* far_node = nullptr;
	float2 intersection_test_result = r.intersects_aabb(current->bounds);
	if (intersection_test_result.x >= intersection_test_result.y || intersection_test_result.x > r.t) return; // now we know that the root is intersected and partly closer than the furthest already hit object

	for (int step = 0; step < 100000; step++) {
		if (current->count) { // if in leaf
			for (int i = current->leftFirst; i < current->leftFirst + current->count; i++) {
				const uint prim_start = m_model_primitives_starts[obj_index];
				const Triangle& t = m_triangles[prim_start + m_indices[prim_start + i]];
				intersect_triangle(t, r);
			}
			last = current;
			if (current->parent == -1) return;
			current = &m_bvh_nodes[model_start + current->parent];
			continue;
		}


		float dist_left = abs(m_bvh_nodes[model_start + current->leftFirst].bounds.minx - r.o.x) +
			abs(m_bvh_nodes[model_start + current->leftFirst].bounds.miny - r.o.y) +
			abs(m_bvh_nodes[model_start + current->leftFirst].bounds.minz - r.o.z);
		float dist_right = abs(m_bvh_nodes[model_start + current->leftFirst + 1].bounds.minx - r.o.x) +
			abs(m_bvh_nodes[model_start + current->leftFirst + 1].bounds.miny - r.o.y) +
			abs(m_bvh_nodes[model_start + current->leftFirst + 1].bounds.minz - r.o.z);
		if (dist_left < dist_right) {
			near_node = &m_bvh_nodes[model_start + current->leftFirst];
			far_node = &m_bvh_nodes[model_start + current->leftFirst + 1];
		}
		else {
			near_node = &m_bvh_nodes[model_start + current->leftFirst + 1];
			far_node = &m_bvh_nodes[model_start + current->leftFirst];
		}


		if (last == far_node) { //just went up
			last = current;
			if (current->parent == -1) return;
			current = &m_bvh_nodes[model_start + current->parent];
			continue;
		}

		// either last node is near or parent

		const BVHNode* try_child = nullptr;
		if (current->parent == -1) {
			try_child = (last != near_node) ? near_node : far_node;
		}
		else {
			try_child = (last == &m_bvh_nodes[model_start + current->parent]) ? near_node : far_node;
		}

		intersection_test_result = r.intersects_aabb(try_child->bounds);
		if (intersection_test_result.x < intersection_test_result.y) { // if intersection is found
			last = current;
			current = try_child;
		}
		else { //either move to far or up
			if (try_child == near_node) { // move to far
				last = near_node;
			}
			else { // move up
				last = current;
				if (current->parent == -1) return;
				current = &m_bvh_nodes[model_start + current->parent];
			}
		}
	}
}

void Scene::intersect_top(Ray& r) const { //assumes ray intersects
	//https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
	const BVHNode* last = nullptr;
	const BVHNode* current = &m_top_bvh_nodes[0];
	const BVHNode* near_node = nullptr;
	const BVHNode* far_node = nullptr;
	float2 intersection_test_result = r.intersects_aabb(current->bounds);
	if (intersection_test_result.x >= intersection_test_result.y || intersection_test_result.x > r.t) return; // now we know that the root is intersected and partly closer than the furthest already hit object
	
	for (int step = 0; step < 100000; step++) {
		if (current->count) { // if in leaf
			for (int i = current->leftFirst; i < current->leftFirst + current->count; i++) {
				TopBVHNodeScene node = m_top_leaves[m_top_indices[i]];
				r.o -= node.pos;
				intersect_bot(r, node.obj_index);
				r.o += node.pos;
			}
			last = current;
			if (current->parent == -1) return;
			current = &m_top_bvh_nodes[current->parent];
			continue;
		}


		float dist_left = abs(m_top_bvh_nodes[current->leftFirst].bounds.minx - r.o.x) + 
			abs(m_top_bvh_nodes[current->leftFirst].bounds.miny - r.o.y) + 
			abs(m_top_bvh_nodes[current->leftFirst].bounds.minz - r.o.z);
		float dist_right = abs(m_top_bvh_nodes[current->leftFirst + 1].bounds.minx - r.o.x) +
			abs(m_top_bvh_nodes[current->leftFirst + 1].bounds.miny - r.o.y) +
			abs(m_top_bvh_nodes[current->leftFirst + 1].bounds.minz - r.o.z);
		if (dist_left < dist_right) {
			near_node = &m_top_bvh_nodes[current->leftFirst];
			far_node = &m_top_bvh_nodes[current->leftFirst + 1];
		}
		else {
			near_node = &m_top_bvh_nodes[current->leftFirst + 1];
			far_node = &m_top_bvh_nodes[current->leftFirst];
		}


		if (last == far_node) { //just went up
			last = current;
			if (current->parent == -1) return;
			current = &m_top_bvh_nodes[current->parent];
			continue;
		}

		// either last node is near or parent

		const BVHNode* try_child = nullptr;
		if (current->parent == -1) {
			try_child = (last != near_node) ? near_node : far_node;
		} else {
			try_child = (last == &m_top_bvh_nodes[current->parent]) ? near_node : far_node;
		}

		intersection_test_result = r.intersects_aabb(try_child->bounds);
		if (intersection_test_result.x < intersection_test_result.y) { // if intersection is found
			last = current;
			current = try_child;
		} else { //either move to far or up
			if (try_child == near_node) { // move to far
				last = near_node;
			} else { // move up
				last = current;
				if (current->parent == -1) return;
				current = &m_top_bvh_nodes[current->parent];
			}
		}
	}
}








__kernel void extend(
	__global struct Ray* ray_data,
	__global struct BVHNode* m_top_bvh_nodes,
	__global struct TopBVHNode* m_top_leaves,
	__global uint* m_top_indices,
	__global struct BVHNode* m_bvh_nodes,
	__global uint* m_model_primitives_starts,
	__global uint* m_model_bvh_starts,
	__global struct Triangle* m_triangles,
	__global uint* m_indices
	
	) {
	uint i = get_global_id( 0 );
	Ray& r = ray_data[i];
	find_intersection(r);
}