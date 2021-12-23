#include "precomp.h"
#include "Sphere.h"


Sphere::Sphere(float3 position, float radius, Material material):
	pos(position),
	radius(radius),
	radius2(radius*radius),
	m(material)
{
}

void Sphere::intersects(Ray& ray) const
{
	float3 C = pos - ray.o;
	if (dot(C, C) < radius2) {
		//we are inside sphere
		float t = dot(C, ray.d);
		float3 Q = C - (t * ray.d);
		float p2 = dot(Q, Q);
		if (p2 > radius2) return;
		t += sqrt(radius2 - p2);
		if (t < ray.t && t > 0) {
			ray.t = t;
			ray.hitptr = this;
			ray.p = Primitive::sphere;
			ray.hit_normal = get_normal(ray.o + ray.d * t);
		}
	}
	else {
		float t = dot(C, ray.d);
		float3 Q = C - (t * ray.d);
		float p2 = dot(Q, Q);
		if (p2 > radius2) return;
		t -= sqrt(radius2 - p2);
		if (t < ray.t && t > 0) {
			ray.t = t;
			ray.hitptr = this;
			ray.p = Primitive::sphere;
			ray.hit_normal = get_normal(ray.o+ray.d*t);
		}
	}

}

float3 Sphere::get_normal(const float3& intersection_pos) const
{
	float3 n = intersection_pos - pos;
	if (radius == 1.0) {
		return n;
	}
	else {
		return normalize(n);
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
