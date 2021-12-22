#pragma once
#include "Ray.h"


class Light
{
public:
	float3 pos;
	float3 color;
protected:
	Light(float3 position, float3 color);
public:
	virtual float calculate_light_intensity(const Ray& r) const = 0;
};

