#pragma once
#include "PrimitiveGeometry.h"

class Sphere : public PrimitiveGeometry
{
	float radius;
	float radius2;
public:
	float3 pos;

	Sphere(float3 position, float radius, Material material);
	void intersects(Ray& ray) const override;
	float3 get_normal(float3& intersection_pos) const override;
	void set_r(float radius);
	float r();
	float r2();
};

