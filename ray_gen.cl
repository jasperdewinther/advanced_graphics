#include "common.cl"

__kernel void ray_gen(__global struct Ray *ray_data, int width, int height,
                      float upx, float upy, float upz, float sidex, float sidey,
                      float sidez, float screen_centerx, float screen_centery,
                      float screen_centerz, float aspect_ratio,
                      float cameraposx, float cameraposy, float cameraposz,
                      int ray_size, int noise) {
  uint x = get_global_id(0);
  uint y = get_global_id(1);

  if (x > width - 1 || y > height - 1)
    return;

  struct xorshift_state rand = {
      .a = noise == 0 ? 12 : noise}; // 12 is an arbitrary number

  float px =
      ((float)x + (float)xorshift32(&rand) / (float)4294967295) / (float)width;
  float py =
      ((float)y + (float)xorshift32(&rand) / (float)4294967295) / (float)height;

  float dirx = screen_centerx + (sidex * ((px - 0.5f) * aspect_ratio)) +
               (upx * ((py * -1) + 0.5f));
  float diry = screen_centery + (sidey * ((px - 0.5f) * aspect_ratio)) +
               (upy * ((py * -1) + 0.5f));
  float dirz = screen_centerz + (sidez * ((px - 0.5f) * aspect_ratio)) +
               (upz * ((py * -1) + 0.5f));
  dirx = dirx - cameraposx;
  diry = diry - cameraposy;
  dirz = dirz - cameraposz;
  float3 dir = (float3)(dirx, diry, dirz);
  dir = normalize(dir);
  uint pixel_index = x + (width * y);
  struct Ray r = {
      .o = {cameraposx, cameraposy, cameraposz, 0},
      .d = {dir.x, dir.y, dir.z, 0},
      .invDir = {1.f / dir.x, 1.f / dir.y, 1.f / dir.z, 0},
      .t = 999999.f,
      .pixel_id = pixel_index,
      .E = {0.f, 0.f, 0.f, 0.f},
      .T = {1.f, 1.f, 1.f, 0.f},
      .hitptr = 4294967295,
  };

  ray_data[pixel_index] = r;
}
