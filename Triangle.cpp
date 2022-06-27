#include "precomp.h"
#include "Triangle.h"



Triangle::Triangle(float3 p0, float3 p1, float3 p2, float3 normal, Material material) :
	p0(p0),
	p1(p1),
	p2(p2),
    normal(normal),
    m(material)
{}



float3 Triangle::get_normal() const
{
	return normal;
}

float3 Triangle::get_center() const
{
    return float3((p0.x+p1.x+p2.x)/3.0, (p0.y + p1.y + p2.y) / 3.0, (p0.z + p1.z + p2.z) / 3.0);
}

TriangleCompressed Triangle::compress()
{
    return TriangleCompressed{
        p0,
        p1,
        p2,
        float4{normal.x, normal.y, normal.z, (float)m}
    };
}


std::vector<Triangle> get_mesh_from_file(const std::string& filename, float scalefactor, Material material){
    //assumes normals of all vertices belonging to one triangle are the same 
    printf("loading %s\n", filename.c_str());
    Timer t = Timer();

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
            float3 vn = float3(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]));
            vertices.push_back(vn);
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
    triangles.reserve(std::count_if(lines.begin(), lines.end(), [](const std::string& val) {return val[0] == 'f' && val[1] == ' '; }));
    for (long i = 0; i < lines.size(); i++) { //append all vertices
        if (lines[i][0] == 'f' && lines[i][1] == ' ') {
            if (split(lines[i], ' ').size() == 5) {
                std::pair<Triangle, Triangle> result = parse_quad(lines[i], vertices, normals, scalefactor, material);
                triangles.push_back(result.first);
                triangles.push_back(result.second);
            }
            else {
                triangles.push_back(parse_triangle(lines[i], vertices, normals, scalefactor, material));
            }
        }
    }

    printf("loaded %s in %f seconds containing a total of %i triangles\n", filename.c_str(), t.elapsed(), triangles.size());
    return triangles;
}

Triangle parse_triangle(const std::string& s, const std::vector<float3>& vertices, const std::vector<float3>& normals, const float& scalefactor, const Material& material) {
    std::vector<std::string> parts = split(s, ' ');
    std::vector<std::string> part0 = split(parts[1], '/');
    std::vector<std::string> part1 = split(parts[2], '/');
    std::vector<std::string> part2 = split(parts[3], '/');

    float3 p0 = vertices[std::stol(part0[0]) - 1] * scalefactor;
    float3 p1 = vertices[std::stol(part1[0]) - 1] * scalefactor;
    float3 p2 = vertices[std::stol(part2[0]) - 1] * scalefactor;

    float3 n0;
    float3 n1;
    float3 n2;
    bool failed = false;
    if (std::stol(part0[2]) - 1 < normals.size()) n0 = normals[std::stol(part0[2]) - 1];
    else failed = true;
    if (std::stol(part1[2]) - 1 < normals.size()) n1 = normals[std::stol(part1[2]) - 1];
    else failed = true;
    if (std::stol(part2[2]) - 1 < normals.size()) n2 = normals[std::stol(part2[2]) - 1];
    else failed = true;
    float3 n = failed ? normalize(cross(p2 - p1, p0 - p1)) : normalize(float3((n0.x + n1.x + n2.x) / 3.f, (n0.y + n1.y + n2.y) / 3.f, (n0.z + n1.z + n2.z) / 3.f));
    return Triangle(p0, p1, p2, n, material);
}
std::pair<Triangle, Triangle> parse_quad(const std::string& s, const std::vector<float3>& vertices, const std::vector<float3>& normals, const float& scalefactor, const Material& material) {
    std::vector<std::string> parts = split(s, ' ');
    std::vector<std::string> part0 = split(parts[1], '/');
    std::vector<std::string> part1 = split(parts[2], '/');
    std::vector<std::string> part2 = split(parts[3], '/');
    std::vector<std::string> part3 = split(parts[4], '/');

    float3 p0 = vertices[std::stol(part0[0]) - 1] * scalefactor;
    float3 p1 = vertices[std::stol(part1[0]) - 1] * scalefactor;
    float3 p2 = vertices[std::stol(part2[0]) - 1] * scalefactor;
    float3 p3 = vertices[std::stol(part3[0]) - 1] * scalefactor;

    float3 n0;
    float3 n1;
    float3 n2;
    float3 n3;
    bool failed = false;
    if (std::stol(part0[2]) - 1 < normals.size()) n0 = normals[std::stol(part0[2]) - 1];
    else failed = true;
    if (std::stol(part1[2]) - 1 < normals.size()) n1 = normals[std::stol(part1[2]) - 1];
    else failed = true;
    if (std::stol(part2[2]) - 1 < normals.size()) n2 = normals[std::stol(part2[2]) - 1];
    else failed = true;
    if (std::stol(part3[2]) - 1 < normals.size()) n3 = normals[std::stol(part3[2]) - 1];
    else failed = true;
    float3 n = failed ? normalize(cross(p2 - p1, p0 - p1)) : normalize(float3((n0.x + n1.x + n2.x) / 3.f, (n0.y + n1.y + n2.y) / 3.f, (n0.z + n1.z + n2.z) / 3.f));
    return std::pair(Triangle(p0, p1, p2, n, material), Triangle(p2, p0, p3, n, material));
}