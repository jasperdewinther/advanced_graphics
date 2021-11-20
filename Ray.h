#pragma once
#include "Vector.h"
#include <limits>
#include <vector>

class PrimitiveGeometry;

class Ray
{
public:
	Vector3 o; //origin
	Vector3 d; //direction
	float t; //distance to closest intersection
	PrimitiveGeometry* hitptr; //a pointer to the hit object

	Ray(Vector3 origin, Vector3 direction);
};

std::vector<Ray> generate_primary_rays(const Vector3& camerapos, const Vector3& camera_direction, float fov, int width, int height); //fov in horizontal degrees