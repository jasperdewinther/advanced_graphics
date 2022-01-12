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
	std::vector<Plane> planes;
	std::vector<Sphere> spheres;
	std::vector<std::vector<Triangle>> triangles;
	std::vector<BVH<Triangle>> bvhs;
	TopLevelBVH bvh;


	std::vector<SpotLight> spot_lights;
	std::vector<PointLight> point_lights;
	std::vector<DirectionalLight> directional_lights;

	float3 skycolor = float3(0, 0, 0);

	Scene();
	float3 trace_scene(Ray& r, uint bounces, bool complexity_view, int rand) const;
	void find_intersection(Ray& r) const;
	float3 find_direct_light_value(const float3& start_pos, const float3& normal) const;
	void delete_scene();
};

