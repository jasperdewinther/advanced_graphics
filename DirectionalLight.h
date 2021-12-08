#pragma once
#include "Light.h"

class DirectionalLight : public Light
{
	float w; // amount of light coming from the point
public:
	DirectionalLight(float3 direction, float3 color, float light_amount);
	float calculate_light_intensity(const Ray& r);
};

