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
	Ray* rays = nullptr;
	Ray* rays2 = nullptr;
	float3* temp_image = nullptr;
	std::unique_ptr<Buffer> rays_buffer;
	std::unique_ptr<Buffer> rays2_buffer;
	std::unique_ptr<Kernel> ray_gen_kernel = std::make_unique<Kernel>((char*)"ray_gen.cl", (char*)"ray_gen");
	std::unique_ptr<Kernel> ray_extend_kernel = std::make_unique<Kernel>((char*)"ray_extend.cl", (char*)"extend");

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
		const uint nthreads
	);
	float3 trace_scene(Ray& r, uint bounces, bool complexity_view, int rand) const;
private:
	std::vector<BVHNode> m_top_bvh_nodes;
	std::vector<TopBVHNodeScene> m_top_leaves;
	std::vector<uint> m_top_indices;
	std::vector<BVHNode> m_bvh_nodes;
	std::vector<uint> m_model_primitives_starts;
	std::vector<uint> m_model_bvh_starts;
	std::vector<Triangle> m_triangles;
	std::vector<uint> m_indices;
	uint* m_rays;

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
	void extend(uint i);
	void shade(uint i, const int rand, std::atomic<int>& new_ray_index);
	void connect(float3* screendata, uint i);
	void intersect_top(Ray& r) const;
	void intersect_bot(Ray& r, int obj_index) const;
	void intersect_triangle(const Triangle& tri, Ray& ray, uint triangle_index) const;
};

