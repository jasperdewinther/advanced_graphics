#pragma once
#include "Ray.h"
#include "Triangle.h"
#include "BVH.h"
#include "Utils.h"



struct TopBVHNodeScene {
	float3 pos;
	uint obj_index;
};


class Scene
{
public:
	std::vector<std::vector<Triangle>> triangles;
	
	std::vector<BVH<Triangle>> bvhs;
	TopLevelBVH bvh;

	bool active_rays = false; //false = rays, true = rays2 is currently active buffer
	int ray_count = 0;
	std::unique_ptr<Ray[]> rays;
	std::unique_ptr<Ray[]> rays2;
	std::unique_ptr<float3[]> normals_image;
	std::unique_ptr<float3[]> hitpos_image;
	Buffer b_normals_image;
	Buffer b_hitpos_image;
	std::unique_ptr<Buffer> rays_buffer;
	std::unique_ptr<Buffer> rays2_buffer;
	std::unique_ptr<Kernel> ray_gen_kernel = std::make_unique<Kernel>((char*)"ray_gen.cl", (char*)"ray_gen");
	std::unique_ptr<Kernel> ray_extend_kernel = std::make_unique<Kernel>((char*)"ray_extend.cl", (char*)"extend");
	std::unique_ptr<Kernel> ray_shade_kernel = std::make_unique<Kernel>((char*)"ray_shade.cl", (char*)"shade");
	std::unique_ptr<Kernel> ray_connect_kernel = std::make_unique<Kernel>((char*)"ray_connect.cl", (char*)"connect");

	float3 skycolor = float3(0, 0, 0);

	Scene();
	void trace_scene(
		float3* screendata, 
		const uint screen_width,
		const uint screen_height,
		const float3& camerapos,
		const float3& camera_direction,
		const float fov, 
		const uint bounces,
		const int rand,
		const uint nthreads,
		const bool use_gpu
	);
	Buffer* normals_buffer();
	Buffer* hitpos_buffer();
private:
	std::vector<BVHNode> m_top_bvh_nodes;
	std::vector<TopBVHNodeScene> m_top_leaves;
	std::vector<uint> m_top_indices;
	std::vector<BVHNode> m_bvh_nodes;
	std::vector<uint> m_model_primitives_starts;
	std::vector<uint> m_model_bvh_starts;
	std::vector<Triangle> m_triangles;
	std::vector<uint> m_indices;
	std::unique_ptr<uint[]> m_rays_count;

	Buffer b_top_bvh_nodes;
	Buffer b_top_leaves;
	Buffer b_top_indices;
	Buffer b_bvh_nodes;
	Buffer b_model_primitives_starts;
	Buffer b_model_bvh_starts;
	Buffer b_triangles;
	Buffer b_indices;
	Buffer b_rays_count;

	void init_buffers(uint width, uint height);
	void generate(
		const uint screen_width,
		const uint screen_height,
		const float3& camerapos,
		const float3& camera_direction,
		const float fov,
		const int rand,
		const bool primary
		);
	void fill_gpu_buffers(uint screen_width, uint screen_height);
	void extend(uint i);
	void shade(uint i, const int rand, std::atomic<int>& new_ray_index);
	void connect(float3* screendata, uint i, float3 camerapos);
	void intersect_top(Ray& r) const;
	void intersect_bot(Ray& r, int obj_index) const;
	void intersect_triangle(const Triangle& tri, Ray& ray, uint triangle_index) const;
};

