#pragma once
#include "Ray.h"
#include "Plane.h"
#include "Sphere.h"
#include "Triangle.h"

Material get_material_hitptr(const Ray& r);

float3 get_normal_hitptr(const Ray& r, const float3& pos);