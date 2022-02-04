#include "common.cl"

__kernel void connect(__global float4 *screendata, __global struct Ray *rays,
                      __global struct Ray *rays2, int active_rays, int max_i) {
  uint i = get_global_id(0);
  if (i > max_i)
    return;
  struct Ray r = active_rays ? rays2[i] : rays[i];
  screendata[r.pixel_id] = r.E;
}