#pragma once
#include "Ray.h"
#include <optional>


class PrimitiveGeometry
{
	virtual void intersects(Ray& ray) = 0; //updates ray.t if a new closest intersection is found
};

