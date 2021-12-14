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
	std::vector<Plane> planes;
	std::vector<Sphere> spheres;
	std::vector<Triangle> triangles;
	std::vector<BVH> bvhs;


	std::vector<Light*> lights = {};
public:
	Scene();
	float3 trace_scene(Ray& r, int max_bounces) const;
	void find_intersection(Ray& r) const;
	float3 find_direct_light_value(const float3& start_pos, const float3& normal) const;
	void delete_scene();
};

