#include "precomp.h"
#include "Plane.h"

void Plane::intersects(Ray& ray)
{
	float t = -(dot(ray.o, n) + d) / dot(ray.d, n);
	if (t == NAN) {
		ray.t = 0;
	}
	if (t >= 0) {
		ray.t = t;
		ray.hitptr = this;
	}
}

float3 Plane::get_normal(float3& intersection_pos)
{
	return n;
}
