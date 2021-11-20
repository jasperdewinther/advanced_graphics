#pragma once
#include "Vector.h"
#include <limits>


class Ray
{
public:
	Vector3 o; //origin
	Vector3 d; //direction
	float t; //distance to closest intersection

	Ray(Vector3 origin, Vector3 direction);
};

