#include "precomp.h"
#include "AABB.h"

AABB::AABB():
	minx(0.f),
	miny(0.f),
	minz(0.f),
	maxx(0.f),
	maxy(0.f),
	maxz(0.f)
{
}

AABB::AABB(const float3& min, const float3& max):
	minx(min.x),
	miny(min.y),
	minz(min.z),
	maxx(min.x),
	maxy(min.y),
	maxz(min.z)
{}

AABB::AABB(float minx, float miny, float minz, float maxx, float maxy, float maxz):
	minx(minx),
	miny(miny),
	minz(minz),
	maxx(maxx),
	maxy(maxy), 
	maxz(maxz)
{
}
