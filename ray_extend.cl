#include "common.cl"

void intersect_triangle(struct Triangle *tri, __global struct Ray *ray,
                        uint triangle_index) {
  // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
  const float EPSILON = 0.0000001;
  float4 edge1 = tri->p1 - tri->p0;
  float4 edge2 = tri->p2 - tri->p0;
  float3 h = cross(ray->d.xyz, edge2.xyz);
  float a = dot(edge1.xyz, h);
  if (a > -EPSILON && a < EPSILON)
    return; // This ray is parallel to this triangle.
  float f = 1.0 / a;
  float3 s = ray->o.xyz - tri->p0.xyz;
  float u = f * dot(s, h);
  if (u < 0.0 || u > 1.0)
    return;
  float3 q = cross(s, edge1.xyz);
  float v = f * dot(ray->d.xyz, q);
  if (v < 0.0 || u + v > 1.0)
    return;
  // At this stage we can compute t to find out where the intersection point is
  // on the line.
  float t = f * dot(edge2.xyz, q);
  if (t > 0.f && ray->t > t) {
    ray->t = t;
    ray->hitptr = triangle_index;
  }
}

float2 intersects_aabb_glob(__global struct BVHNode *box,
                            __global struct Ray *r) {
  // https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
  float tMinx = (box->minx - r->o.x) * r->invDir.x;
  float tMiny = (box->miny - r->o.y) * r->invDir.y;
  float tMinz = (box->minz - r->o.z) * r->invDir.z;
  float tMaxx = (box->maxx - r->o.x) * r->invDir.x;
  float tMaxy = (box->maxy - r->o.y) * r->invDir.y;
  float tMaxz = (box->maxz - r->o.z) * r->invDir.z;
  float t1x = min(tMinx, tMaxx);
  float t1y = min(tMiny, tMaxy);
  float t1z = min(tMinz, tMaxz);
  float t2x = max(tMinx, tMaxx);
  float t2y = max(tMiny, tMaxy);
  float t2z = max(tMinz, tMaxz);
  float tNear = max(max(t1x, t1y), t1z);
  float tFar = min(min(t2x, t2y), t2z);
  float2 nearfar = {tNear, tFar};
  return nearfar;
}

void intersect_bot(__global struct Ray *r, int obj_index,
                   __global struct BVHNode *m_bvh_nodes,
                   __global uint *m_model_primitives_starts,
                   __global uint *m_model_bvh_starts,
                   __global struct Triangle *m_triangles,
                   __global uint *m_indices

) { // assumes ray intersects
  // https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
  uint model_start = m_model_bvh_starts[obj_index];

  __global struct BVHNode *last;
  __global struct BVHNode *current = &m_bvh_nodes[model_start];
  __global struct BVHNode *near_node;
  __global struct BVHNode *far_node;

  for (int step = 0; step < 1000; step++) {
    if (current->count) { // if in leaf
      for (int i = current->leftFirst; i < current->leftFirst + current->count;
           i++) {
        uint prim_start = m_model_primitives_starts[obj_index];
        uint triangle_index = prim_start + m_indices[prim_start + i];
        struct Triangle t = m_triangles[triangle_index];
        //float sqrd_dist = r->t * r->t;
        /*if (dot(t.p0, t.p0) < sqrd_dist || dot(t.p1, t.p1) < sqrd_dist ||
            dot(t.p2, t.p2) < sqrd_dist)*/ // dont check triangle if its further
                                         // than already hit
        intersect_triangle(&t, r, triangle_index);
      }
      last = current;
      if (current->parent == -1)
        return;
      current = &m_bvh_nodes[model_start + current->parent];
      continue;
    }

    float dist_left =
        fabs(m_bvh_nodes[model_start + current->leftFirst].minx - r->o.x) +
        fabs(m_bvh_nodes[model_start + current->leftFirst].miny - r->o.y) +
        fabs(m_bvh_nodes[model_start + current->leftFirst].minz - r->o.z);
    float dist_right =
        fabs(m_bvh_nodes[model_start + current->leftFirst + 1].minx - r->o.x) +
        fabs(m_bvh_nodes[model_start + current->leftFirst + 1].miny - r->o.y) +
        fabs(m_bvh_nodes[model_start + current->leftFirst + 1].minz - r->o.z);
    if (dist_left < dist_right) {
      near_node = &m_bvh_nodes[model_start + current->leftFirst];
      far_node = &m_bvh_nodes[model_start + current->leftFirst + 1];
    } else {
      near_node = &m_bvh_nodes[model_start + current->leftFirst + 1];
      far_node = &m_bvh_nodes[model_start + current->leftFirst];
    }

    if (last == far_node) { // just went up
      last = current;
      if (current->parent == -1)
        return;
      current = &m_bvh_nodes[model_start + current->parent];
      continue;
    }

    // either last node is near or parent

    __global struct BVHNode *try_child;
    if (current->parent == -1) {
      try_child = (last != near_node) ? near_node : far_node;
    } else {
      try_child = (last == &m_bvh_nodes[model_start + current->parent])
                      ? near_node
                      : far_node;
    }

    float2 intersection_test_result = intersects_aabb_glob(try_child, r);
    if (intersection_test_result.x < intersection_test_result.y /*&&
        intersection_test_result.x < r->t*/) { // if intersection is found
      last = current;
      current = try_child;
    } else {                        // either move to far or up
      if (try_child == near_node) { // move to far
        last = near_node;
      } else { // move up
        last = current;
        if (current->parent == -1)
          return;
        current = &m_bvh_nodes[model_start + current->parent];
      }
    }
  }
}

__kernel void
extend(__global struct Ray *ray_data, __global struct BVHNode *m_top_bvh_nodes,
       __global struct TopBVHNode *m_top_leaves, __global uint *m_top_indices,
       __global struct BVHNode *m_bvh_nodes,
       __global uint *m_model_primitives_starts,
       __global uint *m_model_bvh_starts, __global struct Triangle *m_triangles,
       __global uint *m_indices, int max_i) {

  uint i = get_global_id(0);
  if (i > max_i)
    return;
  __global struct Ray *r = &ray_data[i];

  // https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
  __global struct BVHNode *last;
  __global struct BVHNode *current = &m_top_bvh_nodes[0];
  __global struct BVHNode *near_node;
  __global struct BVHNode *far_node;
  float2 intersection_test_result = intersects_aabb_glob(current, r);
  if (intersection_test_result.x >= intersection_test_result.y/* ||
      intersection_test_result.x > r->t*/)
    return; // now we know that the root is intersected and partly closer than
            // the furthest already hit object

  for (int step = 0; step < 1000; step++) {
    if (current->count) { // if in leaf
      for (int i = current->leftFirst; i < current->leftFirst + current->count;
           i++) {
        __global struct TopBVHNode *node = &m_top_leaves[m_top_indices[i]];
        r->o -= node->pos;
        intersect_bot(r, node->obj, m_bvh_nodes, m_model_primitives_starts,
                      m_model_bvh_starts, m_triangles, m_indices);
        r->o += node->pos;
      }
      last = current;
      if (current->parent == -1)
        return;
      current = &m_top_bvh_nodes[current->parent];
      continue;
    }

    float dist_left = fabs(m_top_bvh_nodes[current->leftFirst].minx - r->o.x) +
                      fabs(m_top_bvh_nodes[current->leftFirst].miny - r->o.y) +
                      fabs(m_top_bvh_nodes[current->leftFirst].minz - r->o.z);
    float dist_right =
        fabs(m_top_bvh_nodes[current->leftFirst + 1].minx - r->o.x) +
        fabs(m_top_bvh_nodes[current->leftFirst + 1].miny - r->o.y) +
        fabs(m_top_bvh_nodes[current->leftFirst + 1].minz - r->o.z);
    if (dist_left < dist_right) {
      near_node = &m_top_bvh_nodes[current->leftFirst];
      far_node = &m_top_bvh_nodes[current->leftFirst + 1];
    } else {
      near_node = &m_top_bvh_nodes[current->leftFirst + 1];
      far_node = &m_top_bvh_nodes[current->leftFirst];
    }

    if (last == far_node) { // just went up
      last = current;
      if (current->parent == -1)
        return;
      current = &m_top_bvh_nodes[current->parent];
      continue;
    }

    // either last node is near or parent

    __global struct BVHNode *try_child;
    if (current->parent == -1) {
      try_child = (last != near_node) ? near_node : far_node;
    } else {
      try_child =
          (last == &m_top_bvh_nodes[current->parent]) ? near_node : far_node;
    }

    intersection_test_result = intersects_aabb_glob(try_child, r);
    if (intersection_test_result.x < intersection_test_result.y /*&&
        intersection_test_result.x < r->t*/) { // if intersection is found
      last = current;
      current = try_child;
    } else {                        // either move to far or up
      if (try_child == near_node) { // move to far
        last = near_node;
      } else { // move up
        last = current;
        if (current->parent == -1)
          return;
        current = &m_top_bvh_nodes[current->parent];
      }
    }
  }
}