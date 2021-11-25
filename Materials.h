#pragma once

#include <variant>
#include <functional>

enum Material {
	red = 0,
	mirror = 1,
	checkerboard = 2,
	normal = 3,
	magenta= 4,
	cyan = 5,
};

struct MaterialData
{
	std::variant<float3, std::function<float3(float3, float3)>> color; //it either is a static color or gets calculated with the location and normal
	float specularity; //1.0 = 100% reflective
	float3 get_color(const float3& pos, const float3& norm);
};

const MaterialData materials[6] = {
	MaterialData{float3(1.f,0.f,0.f), 0.1f},
	MaterialData{float3(1.f,1.f,1.f), 1.f},
	MaterialData{[](const float3& pos, const float3& norm) {return (((int)pos.x + (int)pos.z) & 1) ? float3(0.f,0.f,1.f) : float3(0.f,1.f,0.f); }, 0.1f},
	MaterialData{[](const float3& pos, const float3& norm) {return (norm+1)/2; } , 0.1f},
	MaterialData{float3(1.f,0.f,1.f), 0.1f},
	MaterialData{float3(0.f,1.f,1.f), 0.1f},
};