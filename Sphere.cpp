#include "precomp.h"
#include "Sphere.h"


Sphere::Sphere(float3 position, float radius):
	pos(position),
	radius(radius),
	radius2(radius*radius)
{}

void Sphere::intersects(Ray& ray)
{
	float3 C = pos - ray.o;
	float t = dot(C, ray.d);
	float3 Q = C - (t * ray.d);
	float p2 = dot(Q, Q);
	if (p2 > radius2) return;
	t -= sqrt(radius2 - p2);
	if ((t < ray.t) && (t > 0)) {
		ray.t = t;
		ray.hitptr = this;
	}
}

void Sphere::set_r(float new_radius)
{
	radius = new_radius;
	radius2 = new_radius * new_radius;
}

float Sphere::r()
{
	return radius;
}

float Sphere::r2()
{
	return radius2;
}
