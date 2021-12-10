#include "precomp.h"
#include "Triangle.h"
#include <vector>
#include <fstream>


Triangle::Triangle(float3 p0, float3 p1, float3 p2, float3 normal, Material material) :
	p0(p0),
	p1(p1),
	p2(p2),
    normal(normal),
    m(material)
{}

void Triangle::intersects(Ray& ray) const
{
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    const float EPSILON = 0.0000001;
    float3 edge1 = p1 - p0;
    float3 edge2 = p2 - p0;
    float3 h = cross(ray.d, edge2);
    float a = dot(edge1,h);
    if (a > -EPSILON && a < EPSILON) return;    // This ray is parallel to this triangle.
    float f = 1.0 / a;
    float3 s = ray.o - p0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return;
    float3 q = cross(s,edge1);
    float v = f * dot(ray.d, q);
    if (v < 0.0 || u + v > 1.0) return;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);
    if (t > 0.f && ray.t > t) {
        ray.t = t;
        ray.hitptr = this;
        ray.p = Primitive::triangle;
    }
}

float3 Triangle::get_normal() const
{
	return normal;
}


std::vector<Triangle> get_mesh_from_file(const std::string& filename, float scalefactor, float3 position, Material material){
    //assumes normals of all vertices belonging to one triangle are the same 

    std::ifstream infile = std::ifstream(filename);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(infile, line))
    {
        lines.push_back(line);
    }

    std::vector<float3> vertices;
    vertices.reserve(std::count_if(lines.begin(), lines.end(), [](const std::string& val) {return val[0] == 'v' && val[1] == ' '; }));
    std::for_each(lines.begin(), lines.end(), [&vertices](const std::string& val) {
        if (val[0] == 'v' && val[1] == ' ') {
            std::vector<std::string> parts = split(val, ' ');
            float3 v = float3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]));
            vertices.push_back(v);
        }
    });


    std::vector<float3> normals;
    normals.reserve(std::count_if(lines.begin(), lines.end(), [](const std::string& val) {return val[0] == 'v' && val[1] == 'n'; }));
    std::for_each(lines.begin(), lines.end(), [&normals](const std::string& val) {
        if (val[0] == 'v' && val[1] == 'n') {
            std::vector<std::string> parts = split(val, ' ');
            float3 vn = float3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]));
            normals.push_back(vn);
        }
    });

    std::vector<Triangle> triangles;
    for (long i = 0; i < lines.size(); i++) { //append all vertices
        if (lines[i][0] == 'f' && lines[i][1] == ' ') {
            std::vector<std::string> parts = split(lines[i], ' ');
            std::vector<std::string> part0 = split(parts[1], '/');
            std::vector<std::string> part1 = split(parts[2], '/');
            std::vector<std::string> part2 = split(parts[3], '/');

            float3 p0 = vertices[std::stol(part0[0])-1]*scalefactor+ position;
            float3 p1 = vertices[std::stol(part1[0])-1]*scalefactor+ position;
            float3 p2 = vertices[std::stol(part2[0])-1]*scalefactor+ position;

            float3 n0;
            float3 n1;
            float3 n2;
            bool failed = false;
            if (std::stol(part0[2]) - 1 < normals.size()) n0 = normals[std::stol(part0[2]) - 1];
            else failed = true;
            if (std::stol(part1[2]) - 1 < normals.size()) n0 = normals[std::stol(part1[2]) - 1];
            else failed = true;
            if (std::stol(part2[2]) - 1 < normals.size()) n0 = normals[std::stol(part2[2]) - 1];
            else failed = true;
            float3 n = failed ? normalize(cross(p2 - p1, p0 - p1)) : normalize(float3((n0.x + n1.x + n2.x) / 3.f, (n0.y + n1.y + n2.y) / 3.f, (n0.z + n1.z + n2.z) / 3.f));
            triangles.push_back(Triangle(p0, p1, p2, n, material));
        }
    }


    return triangles;
}
