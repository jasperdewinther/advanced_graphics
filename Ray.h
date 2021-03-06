#pragma once
#include <limits>
#include <vector>
#include "BVHNode.h"
#include "Triangle.h"


class Ray
{
public:
	float3 o; //origin
	float3 d; //direction
	float3 invDir;
	float t; //distance to closest intersection
	int pixel_id;
	float3 E;
	float3 T;
	uint hitptr; //a pointer to the hit object

	Ray();
	Ray(float3 origin, float3 direction, uint pixel_id, float3 E, float3 T);
	float2 intersects_aabb(const BVHNode& box); //returns tnear and tfar
	float2 intersects_aabb(const BVHNode* box);
};

void generate_primary_rays(
	const float3& camerapos, 
	const float3& camera_direction, 
	float fov, 
	int width, 
	int height,
	Kernel* kernel,
	Buffer* buffer,
	int noise); //fov in horizontal degrees
