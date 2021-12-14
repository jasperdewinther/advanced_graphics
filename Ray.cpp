#include "precomp.h"
#include "Ray.h"

Ray::Ray(float3 origin, float3 direction):
	o(origin),
	d(direction),
	invDir(1/direction),
	t(std::numeric_limits<float>::max()),
	hitptr(nullptr)
{}

float2 Ray::intersects_aabb(const aabb & box)
{
	//https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
	float3 tMin = (box.bmin3 - o) * invDir;
	float3 tMax = (box.bmax3 - o) * invDir;
	float3 t1 = float3(min(tMin.x, tMax.x), min(tMin.y, tMax.y), min(tMin.z, tMax.z));
	float3 t2 = float3(max(tMin.x, tMax.x), max(tMin.y, tMax.y), max(tMin.z, tMax.z));
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return float2(tNear, tFar);
}


void generate_primary_rays(const float3& camerapos, const float3& camera_direction, float fov, int width, int height, Ray* rays, int nthreads, int antialiasing) {
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


	run_multithreaded(nthreads, width, height, false, [&antialiasing, &width, &height, &up, &side, &screen_center, &aspect_ratio, &camerapos, &rays](int x, int y) {
		xorshift_state rand = xorshift_state{ (uint)x * y + 1 };

		for (int n = 0; n < antialiasing; n++) {
			float px = ((float)x + (float)xorshift32(&rand) / (float)UINT32_MAX) / (float)width;
			float py = ((float)y + (float)xorshift32(&rand) / (float)UINT32_MAX) / (float)height;
			float3 dir = screen_center + (side * ((px - 0.5f) * aspect_ratio)) + (up * ((py * -1) + 0.5f));
			dir = dir - camerapos;
			dir = normalize(dir);
			rays[(x + width * y) * antialiasing + n] = Ray(camerapos, dir);
		}
		
	});
}

