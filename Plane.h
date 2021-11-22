#pragma once
#include "PrimitiveGeometry.h"

class Plane : PrimitiveGeometry
{
	float3 n; //plane normal
	float d; //distance from (0,0,0) to plane

	void intersects(Ray& ray);
};

