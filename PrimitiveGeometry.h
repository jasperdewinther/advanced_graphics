#pragma once
#include "Ray.h"
#include <optional>
#include "materials.h"

class PrimitiveGeometry
{
public:
	Material m;
	virtual void intersects(Ray& ray) const = 0; //updates ray.t if a new closest intersection is found
	virtual float3 get_normal(float3& intersection_pos) const = 0;
};

