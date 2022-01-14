#pragma once
#include "Ray.h"
#include "Triangle.h"
#include "BVH.h"
#include "Utils.h"

class Scene
{
public:
	std::vector<std::vector<Triangle>> triangles;
	std::vector<BVH<Triangle>> bvhs;
	TopLevelBVH bvh;

	float3 skycolor = float3(0, 0, 0);

	Scene();
	float3 trace_scene(Ray& r, uint bounces, bool complexity_view, int rand) const;
	void find_intersection(Ray& r) const;
};

