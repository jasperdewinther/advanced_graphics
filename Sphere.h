#pragma once
#include "Vector.h"
#include "PrimitiveGeometry.h"

class Sphere : PrimitiveGeometry
{
	float radius;
	float radius2;
public:
	Vector3 pos;

	Sphere(Vector3 position, float radius);
	void intersects(Ray& ray);
	void set_r(float radius);
	float r();
	float r2();
};

