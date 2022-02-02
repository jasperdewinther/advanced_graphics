#include "precomp.h"
#include "Ray.h"

Ray::Ray(float3 origin, float3 direction, uint pixel_id, float3 E, float3 T):
	o(origin),
	d(direction),
	invDir(1/direction),
	t(std::numeric_limits<float>::max()),
	pixel_id(pixel_id),
	E(E),
	T(T),
	hitptr(4294967295)
{}
Ray::Ray() {

}

float2 Ray::intersects_aabb(const BVHNode & box)
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
float2 Ray::intersects_aabb(const BVHNode* box)
{
	//https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
	float tMinx = (box->minx - o.x) * invDir.x;
	float tMiny = (box->miny - o.y) * invDir.y;
	float tMinz = (box->minz - o.z) * invDir.z;
	float tMaxx = (box->maxx - o.x) * invDir.x;
	float tMaxy = (box->maxy - o.y) * invDir.y;
	float tMaxz = (box->maxz - o.z) * invDir.z;
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
	kernel->SetArgument(17, (int)0);
	kernel->Run2D(int2(width + (16-(width%16)), height + (16-(height%16))), int2(16,16));
	
}

