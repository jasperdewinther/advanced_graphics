#include "precomp.h"
#include "Sphere.h"


Sphere::Sphere(Vector3 position, float radius):
	pos(position),
	radius(radius),
	radius2(radius*radius)
{}

void Sphere::intersects(Ray& ray)
{
	Vector3 C = pos - ray.o;
	float t = C.dot(ray.d);
	Vector3 Q = C - (t * ray.d);
	float p2 = Q.dot(Q);
	if (p2 > radius2) return;
	t -= sqrt(radius2 - p2);
	if ((t < ray.t) && (t > 0)) ray.t = t;
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
