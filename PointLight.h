#pragma once
#include "Light.h"

class PointLight : public Light
{
public:
	float w; // amount of light coming from the point
	PointLight(float3 position, float3 color, float light_amount);
	float calculate_light_intensity(const Ray& r);
};

