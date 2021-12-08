#include "precomp.h"
#include "SpotLight.h"


SpotLight::SpotLight(float3 position, float3 direction, float max_angle, float3 color, float light_amount):
	dir(normalize(direction)),
	w(light_amount),
	angle(max_angle),
	Light(position, color)
{
	position = position;
}
float SpotLight::calculate_light_intensity(const Ray& r) {
	if (dot(float3(-1,-1,-1)*r.d, dir) > angle) {
		float3 direction = r.o - pos;
		return (w / (powf(dot(direction, direction), 2)));
	}
	return 0.f;
}