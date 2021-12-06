#pragma once
#include "PrimitiveGeometry.h"
#include "Light.h"
#include "PointLight.h"
#include "Ray.h"
#include "Sphere.h"
#include "Plane.h"

class Scene
{
	std::vector<PrimitiveGeometry*> primitives = {};
	std::vector<Light*> lights = {};
public:
	Scene();
	float3 trace_scene(Ray& r, int max_bounces) const;
	void find_intersection(const std::vector<PrimitiveGeometry*>& scene, Ray& r) const;
	float3 find_direct_light_value(const std::vector<PrimitiveGeometry*>& scene, const float3& start_pos, const float3& normal) const;
	float refraction_index_at(const float3& p);
	void delete_scene();
};

