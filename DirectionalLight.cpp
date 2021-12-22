#include "precomp.h"
#include "DirectionalLight.h"



DirectionalLight::DirectionalLight(float3 direction, float3 color, float light_amount):
	Light(-direction*1000000, color), //make sure that almost all lights check for intersections in roughly the same direction
	w(light_amount)
{

}
float DirectionalLight::calculate_light_intensity(const Ray& r) const {
	return w;
}