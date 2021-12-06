#pragma once
#include "Materials.h"
#include "Ray.h"

class Sphere
{
	float radius;
	float radius2;
public:
	Material m;
	float3 pos;

	Sphere(float3 position, float radius, Material material);
	void intersects(Ray& ray) const;
	float3 get_normal(const float3& intersection_pos) const;
	void set_r(float radius);
	float r();
	float r2();
};

