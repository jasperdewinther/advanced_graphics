#pragma once

enum Material {
	red = 0,
	mirror = 1,
	magenta = 2,
	cyan = 3,
	white = 4,
	glass = 5,
	red_glass = 6,
	reflective_blue = 7,
	white_light = 8,
	emerald = 9,
	bright_green = 10,
	yellow = 11,
	green_light = 12,
};

struct MaterialData
{
	float3 color; //it either is a static color or gets calculated with the location and normal
	float specularity; //1.0 = 100% reflective
	float transparent; // 1 if not transparent at all, 0 if completely transparent, this has to do with beers law
	float refractive_index; 
	bool isLight;
	MaterialData(
		float3 color, 
		float specularity,
		float transparent=1, 
		float refractive_index=0,
		bool isLight=false
	);
};

const uint MATERIALCOUNT = 13;
const MaterialData materials[MATERIALCOUNT] = {
	MaterialData(float3(1.f,0.f,0.f), 0.0f),
	MaterialData(float3(1.f,1.f,1.f), 1.0f),
	MaterialData(float3(0.212f,0.263f,0.f), 0.0f),
	MaterialData(float3(0.1f,1.f,1.f), 0.1f),
	MaterialData(float3(1.f,1.f,1.f), 0.0f),
	MaterialData(float3(1.f,1.f,1.f), 0.0f, 0.f, 1.5f), //glass
	MaterialData(float3(0.878f,0.066f,0.373f), 0.0f, 0.7f, 1.2f),
	MaterialData(float3(0.1f,0.6f,0.9f), 0.9f),
	MaterialData(float3(10.f,10.f,10.f), 0.0f, 1.f,0.f, true),
	MaterialData(float3(0.314f,0.784f,0.471f), 0.0f, 0.3f, 1.57f),
	MaterialData(float3(0.66, 0.859, 0.118), 0.f),
	MaterialData(float3(0.878, 0.906, 0.133), 0.f),
	MaterialData(float3(6.6, 8.59, 1.18), 0.f, 1, 0, true),
};

Material get_random_material();