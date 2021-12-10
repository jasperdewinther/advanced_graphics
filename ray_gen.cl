#include "../template/cl/tools.cl"


int randomize(int* a){
	int x = *a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return *a = x;
}


__kernel void main(__global void* data, float3 camerapos, float3 screen_center, int antialiasing, int width, int height, float3 up, float3 side, float aspect_ratio, int ray_data_size){
	const int i = get_global_id(0);

	int randomizer = i;

	for (int n = 0; n < antialiasing; n++) {
		float px = ((float)x + (float)xorshift32(&randomizer) / (float)0xffffffff) / (float)width;
		float py = ((float)y + (float)xorshift32(&randomizer) / (float)0xffffffff) / (float)height;
		float3 dir = screen_center + (side * ((px - 0.5f) * aspect_ratio)) + (up * ((py * -1) + 0.5f));
		dir = dir - camerapos;
		dir = normalize(dir);
		rays[(x + width * y) * antialiasing + n] = Ray(camerapos, dir);
	}
}

// EOF