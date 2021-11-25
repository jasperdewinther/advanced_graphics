#pragma once
#include "PrimitiveGeometry.h"

class Plane : public PrimitiveGeometry
{
	float3 n; //plane normal
	float d; //distance from (0,0,0) to plane
public:
	Plane(float3 normal, float distance, Material material);
	void intersects(Ray& ray) override;
	float3 get_normal(float3& intersection_pos) override;
};

