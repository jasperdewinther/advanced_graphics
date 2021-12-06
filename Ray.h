#pragma once
#include <limits>
#include <vector>
#include <Utils.h>


class Ray
{
public:
	float3 o; //origin
	float3 d; //direction
	float t; //distance to closest intersection
	const void* hitptr; //a pointer to the hit object
	Primitive p = Primitive::nothing;

	Ray(float3 origin, float3 direction);
};

void generate_primary_rays(const float3& camerapos, const float3& camera_direction, float fov, int width, int height, Ray* rays, int nthreads, int antialiasing); //fov in horizontal degrees

