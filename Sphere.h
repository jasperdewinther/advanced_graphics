#pragma once
#include "PrimitiveGeometry.h"

class Sphere : public PrimitiveGeometry
{
	float radius;
	float radius2;
public:
	float3 pos;

	Sphere(float3 position, float radius);
	void intersects(Ray& ray);
	float3 get_normal(float3& intersection_pos);
	void set_r(float radius);
	float r();
	float r2();
};

