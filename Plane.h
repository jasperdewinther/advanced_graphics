#pragma once
#include "PrimitiveGeometry.h"
#include "Vector.h"

class Plane : PrimitiveGeometry
{
	Vector3 n; //plane normal
	float d; //distance from (0,0,0) to plane

	void intersects(Ray& ray);
};

