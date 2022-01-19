#pragma once
#include "Ray.h"
#include "Triangle.h"
#include "BVH.h"
#include "Utils.h"

struct DoubleBool {
	bool first;
	bool second;
};

class Scene
{
public:
	std::vector<std::vector<Triangle>> triangles;
	std::vector<BVH<Triangle>> bvhs;
	TopLevelBVH bvh;

	bool active_rays = false; //false = rays, true = rays2 is currently active buffer
	int ray_count = 0;
	int shadowray_count = 0;
	Ray* rays = nullptr;
	Ray* rays2 = nullptr;
	std::unique_ptr<Buffer> rays_buffer;
	std::unique_ptr<Kernel> ray_gen_kernel = std::make_unique<Kernel>((char*)"ray_gen.cl", (char*)"ray_gen");

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
private:
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
	void find_intersection(Ray& r) const;
};

