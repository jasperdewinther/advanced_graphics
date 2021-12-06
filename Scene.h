#pragma once
#include "RayHelper.h"
#include "Light.h"
#include "PointLight.h"
#include "Ray.h"
#include "Sphere.h"
#include "Plane.h"
#include "Triangle.h"

class Scene
{
	int n_planes = 0;
	Plane* planes;
	int n_spheres = 0;
	Sphere* spheres;
	int n_triangles = 0;
	Triangle* triangles;


	std::vector<Light*> lights = {};
public:
	Scene();
	float3 trace_scene(Ray& r, int max_bounces) const;
	void find_intersection(Ray& r) const;
	float3 find_direct_light_value(const float3& start_pos, const float3& normal) const;
	void delete_scene();
};

