#include "precomp.h"
#include "PointLight.h"

PointLight::PointLight(float3 position, float3 color, float light_amount):
	Light(position, color),
	w(light_amount)
{
}

float PointLight::calculate_light_intensity(const Ray& r) const
{
	float3 direction = r.o-pos;
	return (w / (powf(dot(direction, direction),2)));
}
