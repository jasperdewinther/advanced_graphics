#pragma once
#include "RayHelper.h"
#include "Light.h"
#include "PointLight.h"
#include "Ray.h"
#include "Sphere.h"
#include "Plane.h"
#include "Triangle.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include "BVH.h"

class Scene
{
public:
	const std::vector<Plane> planes;
	const std::vector<Sphere> spheres;
	const std::vector<Triangle> triangles;
	const std::vector<BVH> bvhs;


	const std::vector<Light*> lights = {};
	Scene();
	float3 trace_scene(Ray& r, int max_bounces) const;
	void find_intersection(Ray& r) const;
	float3 find_direct_light_value(const float3& start_pos, const float3& normal) const;
	void delete_scene();
};

