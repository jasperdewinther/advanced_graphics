#pragma once
#include "PrimitiveGeometry.h"

class Plane : public PrimitiveGeometry
{
	float3 n; //plane normal
	float d; //point on plane
public:
	Plane(float3 normal, float distance_from_origin, Material material);
	void intersects(Ray& ray) const override;
	float3 get_normal(float3& intersection_pos) const override;
};

