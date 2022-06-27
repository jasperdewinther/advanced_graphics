#include "common.cl"

__kernel void connect(__global float4 *screendata, __global struct Ray *rays,
                      __constant int *max_i) {
  uint i = get_global_id(0);
  if (i > max_i[0])
    return;
  struct Ray r = rays[i];
  screendata[r.pixel_id] = r.E;
}