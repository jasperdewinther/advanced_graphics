#include "common.cl"

__constant float BSIGMA = 0.1;
__constant uint MSIZE = 3;
__constant float image_kernel[15] = {
    0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565,
    0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044,
    0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216};

float normpdf(float x, float sigma) {
  return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float normpdf3(float4 v, float sigma) {
  return 0.39894 * exp(-0.5 * dot(v.xyz, v.xyz) / (sigma * sigma)) / sigma;
}

__kernel void filter(__global float4 *accumulation_buffer,
                     __global float4 *output, __global float4 *normals,
                     __global float4 *hitpos, int accumulation_count, int max_i,
                     int screen_width) {
  // uses a modified version of coded from https://www.shadertoy.com/view/4dfGDH
  int i = get_global_id(0);
  if (i > max_i)
    return;

  const int kSize = (MSIZE - 1) / 2;
  float4 c = accumulation_buffer[i];
  float4 n = normals[i];
  float4 h = hitpos[i];
  if (n.x == 0.f && n.y == 0.f && n.z == 0.f) {
    output[i] = max(0.0001f, min(0.999f, c / accumulation_count));
    return;
  }

  float Z = 0.f;
  int x = i % screen_width;
  int y = i / screen_width;
  float4 final_color = {0, 0, 0, 0};
  float bZ = 1.0 / normpdf(0.0, BSIGMA);

  for (int k = -kSize; k <= kSize; ++k) {
    if (y + k >= screen_width || y + k < 0)
      continue;
    for (int l = -kSize; l <= kSize; ++l) {
      if (x + l >= max_i / screen_width || x + l < 0)
        continue;

      float4 nn = normals[x + l + ((y + k) * screen_width)];
      float4 hh = hitpos[x + l + ((y + k) * screen_width)];

      float factor = normpdf3(nn - n, BSIGMA) * normpdf3(hh - h, BSIGMA);
      Z += factor;
      final_color +=
          factor * accumulation_buffer[x + l + ((y + k) * screen_width)];
    }
  }
  final_color = (final_color / Z) / (float)accumulation_count;
  final_color = max(0.0001f, min(0.999f, final_color));
  output[i] = final_color;
}