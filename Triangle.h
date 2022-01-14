#pragma once
#include <vector>
#include <fstream>
#include "Materials.h"
#include "Utils.h"

class Triangle
{
public:
	float3 p0;
	float3 p1;
	float3 p2;
	float3 normal;
	Material m;
	Triangle(float3 p0, float3 p1, float3 p2, float3 normal, Material material);
	float3 get_normal() const;
	float3 get_center() const;
};


std::vector<Triangle> get_mesh_from_file(const std::string& filename, float scalefactor, Material material);

Triangle parse_triangle(const std::string& s, const std::vector<float3>& vertices, const std::vector<float3>& normals, const float& scalefactor, const Material& material);
std::pair<Triangle, Triangle> parse_quad(const std::string& s, const std::vector<float3>& vertices, const std::vector<float3>& normals, const float& scalefactor, const Material& material);