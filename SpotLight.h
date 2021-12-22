#pragma once
#include "Light.h"



class SpotLight : public Light
{
	float3 dir;
	float w; // amount of light coming from the point
	float angle;
public:
	SpotLight(float3 position, float3 direction, float max_angle, float3 color, float light_amount);
	float calculate_light_intensity(const Ray& r) const;
};

