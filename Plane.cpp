#include "precomp.h"
#include "Plane.h"


Plane::Plane(float3 normal, float distance, Material material):
	n(normalize(normal)),
	d(distance)
{
	m = material;
}


void Plane::intersects(Ray& ray) const
{
	float t = -(dot(ray.o, n) - d) / dot(ray.d, n);
	if (std::isnan(t)) { //check if nan
		ray.t = 0;
		ray.hitptr = this;
	} else if (t < ray.t && t >= 0 ) {
		ray.t = t;
		ray.hitptr = this;
	}
}

float3 Plane::get_normal(float3& intersection_pos) const
{
	return n;
}
