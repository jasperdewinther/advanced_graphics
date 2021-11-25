#pragma once
#include "PrimitiveGeometry.h"
#include "Ray.h"
#include "Sphere.h"
#include "Plane.h"

class Scene
{
	std::vector<PrimitiveGeometry*> scene = {};
public:
	Scene();
	float3 trace_scene(Ray& r, float3& energy, const float3& sun_dir);
	void find_intersection(const std::vector<PrimitiveGeometry*>& scene, Ray& r);
	void delete_scene();
};

