#pragma once
#include <limits>
#include <vector>

class PrimitiveGeometry;

class Ray
{
public:
	float3 o; //origin
	float3 d; //direction
	float t; //distance to closest intersection
	const PrimitiveGeometry* hitptr; //a pointer to the hit object

	Ray(float3 origin, float3 direction);
};

std::vector<Ray> generate_primary_rays(const float3& camerapos, const float3& camera_direction, float fov, int width, int height); //fov in horizontal degrees