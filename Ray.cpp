#include "precomp.h"
#include "Ray.h"

Ray::Ray(float3 origin, float3 direction):
	o(origin),
	d(direction),
	t(std::numeric_limits<float>::max()),
	hitptr(nullptr)
{}


void generate_primary_rays(const float3& camerapos, const float3& camera_direction, float fov, int width, int height, Ray* rays, int nthreads) {
	float aspect_ratio = (float)width / (float)height;
	float half_aspect_ratio = aspect_ratio / 2;
	float3 screen_center = camerapos + (camera_direction * (float)(half_aspect_ratio/tan(fov*PI / 360.0)));
	float3 side =
		cross(float3(
			camera_direction.x, 
			camera_direction.y == 0 ? 0.0000001f : 0, 
			camera_direction.z)
		,camera_direction); //the very small value is used when camera_direction[1] == 0, to still find a perpendicular vector
	float3 up = cross(camera_direction,side);
	side = normalize(side);
	if (up.y < 0) {
		up.y *= -1;
		side *= -1;
	}
	up = normalize(up);

	float columns_per_thread = (float)height/(float)nthreads;
	std::vector<std::thread> t = {};
	t.reserve(nthreads);
	for (int i = 0; i < nthreads; i++) {
		t.push_back(std::thread([i, &columns_per_thread, &width, &height, &rays, &screen_center, &side, &aspect_ratio, &up, &camerapos] () {
			for (int y = (int)(i*columns_per_thread); y < (float)(i+1) * columns_per_thread; y++) {
				for (int x = 0; x < width; x++) {
					float px = (float)x / (float)width;
					float py = (float)y / (float)height;
					float3 dir = screen_center + (side * ((px - 0.5f) * aspect_ratio)) + (up * ((py * -1) + 0.5f));
					dir = dir - camerapos;
					dir = normalize(dir);
					rays[x + width * y] = Ray(camerapos, dir);
				}
			}
		}));
	}

	for (auto& thread : t) {
		thread.join();
	}
}

