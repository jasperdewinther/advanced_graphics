#pragma once
#include <limits>
#include <vector>
#include "AABB.h"
#include "Triangle.h"


class Ray
{
public:
	float3 o; //origin
	float3 d; //direction
	float3 invDir;
	float3 hit_normal; //direction when primitive was hit, has to do with rotated objects
	float t; //distance to closest intersection
	const Triangle* hitptr; //a pointer to the hit object
	int complexity = 0;

	Ray(float3 origin, float3 direction);
	float2 intersects_aabb(const AABB& box); //returns tnear and tfar
};

void generate_primary_rays(
	const float3& camerapos, 
	const float3& camera_direction, 
	float fov, 
	int width, 
	int height,
	Ray* rays, 
	int nthreads, 
	Kernel* kernel,
	Buffer* buffer,
	int noise); //fov in horizontal degrees
