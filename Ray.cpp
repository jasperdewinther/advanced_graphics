#include "precomp.h"
#include "Ray.h"
#include "immintrin.h"

Ray::Ray(float3 origin, float3 direction):
	o(origin),
	d(direction),
	invDir(1/direction),
	t(std::numeric_limits<float>::max()),
	hitptr(nullptr)
{}

float2 Ray::intersects_aabb(const AABB & box)
{
	//https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
	float tMinx = (box.minx - o.x) * invDir.x;
	float tMiny = (box.miny - o.y) * invDir.y;
	float tMinz = (box.minz - o.z) * invDir.z;
	float tMaxx = (box.maxx - o.x) * invDir.x;
	float tMaxy = (box.maxy - o.y) * invDir.y;
	float tMaxz = (box.maxz - o.z) * invDir.z;
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


void generate_primary_rays(
	const float3& camerapos, 
	const float3& camera_direction, 
	float fov, 
	int width, 
	int height, 
	Ray* rays, 
	int nthreads, 
	Kernel* kernel, 
	Buffer* buffer,
	int noise
) {
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
	
	kernel->SetArgument(0, buffer);
	kernel->SetArgument(1, width);
	kernel->SetArgument(2, height);
	kernel->SetArgument(3, up.x);
	kernel->SetArgument(4, up.y);
	kernel->SetArgument(5, up.z);
	kernel->SetArgument(6, side.x);
	kernel->SetArgument(7, side.y);
	kernel->SetArgument(8, side.z);
	kernel->SetArgument(9, screen_center.x);
	kernel->SetArgument(10, screen_center.y);
	kernel->SetArgument(11, screen_center.z);
	kernel->SetArgument(12, aspect_ratio);
	kernel->SetArgument(13, camerapos.x);
	kernel->SetArgument(14, camerapos.y);
	kernel->SetArgument(15, camerapos.z);
	kernel->SetArgument(16, (int)(sizeof(Ray)/sizeof(float)));
	kernel->SetArgument(17, (int)noise);
	kernel->Run2D(int2(width - (width%4), height - (height%4)), int2(4,4));
	buffer->CopyFromDevice();



	/*run_multithreaded(nthreads, width, height, false, [&antialiasing, &width, &height, &up, &side, &screen_center, &aspect_ratio, &camerapos, &rays](int x, int y) {
		xorshift_state rand = xorshift_state{ (uint)x * y + 1 };

		for (int n = 0; n < antialiasing; n++) {
			float px = ((float)x + (float)xorshift32(&rand) / (float)UINT32_MAX) / (float)width;
			float py = ((float)y + (float)xorshift32(&rand) / (float)UINT32_MAX) / (float)height;
			float3 dir = screen_center + (side * ((px - 0.5f) * aspect_ratio)) + (up * ((py * -1) + 0.5f));
			dir = dir - camerapos;
			dir = normalize(dir);
			rays[(x + width * y) * antialiasing + n] = Ray(camerapos, dir);
		}
	});*/
}

