#pragma once
#include "PrimitiveGeometry.h"

class Plane : public PrimitiveGeometry
{
	float3 n; //plane normal
	float d; //distance from (0,0,0) to plane

	void intersects(Ray& ray);
	float3 get_normal(float3& intersection_pos);
};

