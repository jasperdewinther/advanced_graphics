#include "precomp.h"
#include "Plane.h"

void Plane::intersects(Ray& ray)
{
	float t = -(ray.o.dot(n) + d) / (ray.d.dot(n));
	if (t == NAN) {
		ray.t = 0;
	}
	if (t >= 0) {
		ray.t = t;
		ray.hitptr = this;
	}
}
