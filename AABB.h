#pragma once
struct AABB
{
	float minx;
	float miny;
	float minz;
	float maxx;
	float maxy;
	float maxz;
	AABB();
	AABB(const float3& min, const float3& max);
	AABB(float minx, float miny, float minz, float maxx, float maxy, float maxz);
	float3 get_center();
};