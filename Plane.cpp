#include "precomp.h"
#include "Plane.h"


Plane::Plane(float3 normal, float distance_from_origin, Material material):
	n(normalize(normal)),
	d(distance_from_origin),
	m(material)
{}


void Plane::intersects(Ray& ray) const
{
	float div = dot(n, ray.d);
	if (div == 0 && dot(ray.o, n)+d == 0) {
		ray.t = 0;
		ray.hitptr = this;
		ray.p = Primitive::plane;
	}
	else {
		float t = -(dot(ray.o, n) + d) / div;
		if (t < ray.t && t >= 0) {
			ray.t = t;
			ray.hitptr = this;
			ray.p = Primitive::plane;
		}
	}
}

float3 Plane::get_normal() const
{
	return n;
}
