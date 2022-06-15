#include "common.cl"

void intersect_triangle(struct Triangle tri, struct Ray *ray,
                        uint triangle_index) {
  // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
  const float EPSILON = 0.0000001;
  float4 edge1 = tri.p1 - tri.p0;
  float4 edge2 = tri.p2 - tri.p0;
  float3 h = cross(ray->d.xyz, edge2.xyz);
  float a = dot(edge1.xyz, h);
  if (a > -EPSILON && a < EPSILON)
    return; // This ray is parallel to this triangle.
  float f = 1.0 / a;
  float3 s = ray->o.xyz - tri.p0.xyz;
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

float2 intersects_aabb_glob(struct BVHNode box, struct Ray *r) {
  // https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
  float tMinx = (box.minx - r->o.x) * r->invDir.x;
  float tMiny = (box.miny - r->o.y) * r->invDir.y;
  float tMinz = (box.minz - r->o.z) * r->invDir.z;
  float tMaxx = (box.maxx - r->o.x) * r->invDir.x;
  float tMaxy = (box.maxy - r->o.y) * r->invDir.y;
  float tMaxz = (box.maxz - r->o.z) * r->invDir.z;
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

struct NearFarNodes {
  struct BVHNode near;
  struct BVHNode far;
};

struct NearFarNodes get_near_far_nodes(__constant struct BVHNode *bvh_nodes,
                                       uint index, struct Ray *r) {
  float dist_left = fabs(bvh_nodes[index].minx - r->o.x) +
                    fabs(bvh_nodes[index].miny - r->o.y) +
                    fabs(bvh_nodes[index].minz - r->o.z);
  float dist_right = fabs(bvh_nodes[index + 1].minx - r->o.x) +
                     fabs(bvh_nodes[index + 1].miny - r->o.y) +
                     fabs(bvh_nodes[index + 1].minz - r->o.z);
  struct NearFarNodes nodes;
  if (dist_left < dist_right) {
    nodes.near = bvh_nodes[index];
    nodes.far = bvh_nodes[index + 1];
  } else {
    nodes.near = bvh_nodes[index + 1];
    nodes.far = bvh_nodes[index];
  }
  return nodes;
}

// bool is true if iteration should be terminated
bool take_step(struct NearFarNodes nearfar, __constant struct BVHNode *nodes,
               struct BVHNode *last, struct BVHNode *current, uint offset,
               struct Ray *r) {
  if (same_node(*last, nearfar.far)) { // just went up
    *last = *current;
    if (current->parent == -1)
      return true;
    *current = nodes[offset + current->parent];
    return false;
  }

  struct BVHNode try_child;
  if (current->parent == -1) {
    try_child = !same_node(*last, nearfar.near) ? nearfar.near : nearfar.far;
  } else {
    try_child = (same_node(*last, nodes[offset + current->parent]))
                    ? nearfar.near
                    : nearfar.far;
  }

  float2 intersection_test_result = intersects_aabb_glob(try_child, r);
  if (intersection_test_result.x <
      intersection_test_result.y) { // if intersection is found
    *last = *current;
    *current = try_child;
  } else {                                    // either move to far or up
    if (same_node(try_child, nearfar.near)) { // move to far
      *last = nearfar.near;
    } else { // move up
      *last = *current;
      if (current->parent == -1)
        return true;
      *current = nodes[offset + current->parent];
    }
  }
  return false;
}

__kernel void extend(__global struct Ray *ray_data,
                     __constant struct BVHNode *m_top_bvh_nodes,
                     __constant struct TopBVHNode *m_top_leaves,
                     __constant uint *m_top_indices,
                     __constant struct BVHNode *m_bvh_nodes,
                     __constant uint *m_model_primitives_starts,
                     __constant uint *m_model_bvh_starts,
                     __constant struct Triangle *m_triangles,
                     __constant uint *m_indices, int max_i) {

  uint i = get_global_id(0);
  if (i > max_i)
    return;
  struct Ray r = ray_data[i];
  struct BVHNode last;
  struct BVHNode current = m_top_bvh_nodes[0];

  struct TopBVHNode blas_node;
  struct BVHNode blast;
  struct BVHNode bcurrent;

  float2 intersection_test_result = intersects_aabb_glob(current, &r);
  if (intersection_test_result.x >= intersection_test_result.y) return;

  bool tracing_bottom_level = false;
  uint blas_index = 0;
  uint bobj_index;
  uint bmodel_start;

  for (int step = 0; step < 10000; step++) {
    if (current.count && !tracing_bottom_level) { // when tracing tlas
      tracing_bottom_level = true;
      blas_node = m_top_leaves[m_top_indices[current.leftFirst + blas_index]];
      r.o -= blas_node.pos;
      bobj_index = blas_node.obj;
      bmodel_start = m_model_bvh_starts[bobj_index];
      bcurrent = m_bvh_nodes[bmodel_start];
    }

    if (tracing_bottom_level) { // when tracing blas
      if (bcurrent.count) { // if in leaf
        for (int bi = bcurrent.leftFirst; bi < bcurrent.leftFirst + bcurrent.count; bi++) {
          uint bprim_start = m_model_primitives_starts[bobj_index];
          uint btriangle_index = bprim_start + m_indices[bprim_start + bi];
          struct Triangle bt = m_triangles[btriangle_index];
          intersect_triangle(bt, &r, btriangle_index);
        }
        blast = bcurrent;
        bcurrent = m_bvh_nodes[bmodel_start + bcurrent.parent];
      }
      
      if (bcurrent.parent == -1) { // done with this blas
        blas_index++;
        r.o += blas_node.pos;
        if (blas_index < current.count) { // if there is another model in this tlas node
          blas_node = m_top_leaves[m_top_indices[current.leftFirst + blas_index]];
          r.o -= blas_node.pos;
          bobj_index = blas_node.obj;
          bmodel_start = m_model_bvh_starts[bobj_index];
          bcurrent = m_bvh_nodes[bmodel_start];
        } else { // when done with this tlas node
          blas_index = 0;
          tracing_bottom_level = false;
          last = current;
          current = m_top_bvh_nodes[current.parent];
          //printf("written ray data 179");
          if (current.parent == -1) {
            ray_data[i] = r;
            return;
          }
        }
      }
    }
    __constant struct BVHNode* step_bvh;
    struct BVHNode step_last;
    struct BVHNode step_current;
    uint step_offset;
    bool return_on_end;
      
    if (!tracing_bottom_level) {
      step_bvh = &m_top_bvh_nodes;
      step_last = last;
      step_current = current;
      step_offset = 0;
      return_on_end = true;
    } else {
      step_bvh = &m_bvh_nodes;
      step_last = blast;
      step_current = bcurrent;
      step_offset = bmodel_start;
      return_on_end = false;
    }
    struct NearFarNodes nodes = get_near_far_nodes(step_bvh, step_current.leftFirst, &r);
    bool kill = take_step(nodes, step_bvh, &step_last, &step_current, 0, &r) && return_on_end;
    if (kill) {
    
      //ray_data[i] = r;
      //printf("written ray data 197");
      break;
    }
  }
  ray_data[i] = r;
}