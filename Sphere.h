#pragma once
#include "PrimitiveGeometry.h"

class Sphere : PrimitiveGeometry
{
	float radius;
	float radius2;
public:
	float3 pos;

	Sphere(float3 position, float radius);
	void intersects(Ray& ray);
	void set_r(float radius);
	float r();
	float r2();
};

