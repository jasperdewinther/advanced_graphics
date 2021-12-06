#pragma once
#include "Materials.h"
#include "Ray.h"

class Plane
{
	float3 n; //plane normal
	float d; //point on plane
public:
	Material m;
	Plane(float3 normal, float distance_from_origin, Material material);
	void intersects(Ray& ray) const;
	float3 get_normal() const;
};

