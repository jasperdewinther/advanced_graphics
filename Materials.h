#pragma once

#include <variant>
#include <functional>

enum Material {
	red = 0,
	mirror = 1,
	checkerboard = 2,
	normal = 3
};

struct MaterialData
{
	std::variant<float3, std::function<float3(float3, float3)>> color; //it either is a static color or gets calculated with the location and normal
	float specularity; //1.0 = 100% reflective
};

MaterialData materials[4] = {
	MaterialData{float3(1.f,0.f,0.f), 0.f},
	MaterialData{float3(1.f,1.f,1.f), 1.f},
	MaterialData{[](float3 loc, float3 norm) {return (((int)loc.x + (int)loc.y) & 1) ? float3(1.f,0.f,0.f) : float3(0.f,1.f,0.f); }, 0.f},
	MaterialData{[](float3 loc, float3 norm) {return norm; } , 0.f},
};