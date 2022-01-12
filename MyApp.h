#pragma once
#include "Scene.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SceneBuilders.h"



/*
implemented:
- BVH with top level BVH
- Very significant performance boost in comparison to brute force (a single teapot with 3600 triangles could take 20 minutes per frame)
- Usage of SAH and binning when constructing the bvh
- Can render 1B triangles in seconds (provided that the camera is not in some bad position)

- Model rotation. However, with slight artifact when top level bb has an border in the same location as a low level bb, then parts of the model might not intersect correctly when rotated
- Multiple scenes which can be loaded 
*/


namespace Tmpl8
{

class MyApp : public TheApp
{
private:
	Scene s = SceneBuilders::path_tracing_test();
	Timer total_time;
	int nthreads = std::thread::hardware_concurrency();
	int old_width;
	int old_height;
	Ray* rays;
	float3* accumulation_buffer;
	float3* pixel_colors;
	float3* temp_image; // used for post processing when a temporary image is required
	ImGuiContext* ctx;
	int virtual_width;
	int virtual_height;
	uint accumulation_count = 0;

	std::unique_ptr<Kernel> ray_gen_kernel = std::make_unique<Kernel>((char*)"ray_gen.cl", (char*)"ray_gen");
	std::unique_ptr<Buffer> rays_buffer;

	float time_setup, time_ray_gen, time_trace, post_processing, time_draw; //performance timers

	//all imgui settings
	bool multithreading = true;
	int bounces = 10;
	float scene_progress = 0.f; //between 0 and 1 where a scene progress of 0 has the same camera position as 1;
	bool block_progress = true;
	int fov = 90;
	float view_height = 8.f;
	int upscaling = 8;
	float gamma_correction = 1.0;
	float vignetting = 0.0;
	int chromatic_aberration = 0;
	float distance_to_center = 10.f;
	bool complexity_view = false;
	bool rotate_objects = false;
	float rotation_progress = 0.f;
	 



	void fix_ray_buffer();
	void set_progression();
	void trace_rays();
	void apply_post_processing();
	void render_pixels();

	void update_rotations();
	void reset_image();
	uint color_to_uint(const float3& color);

public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void PostDraw();
	void Shutdown();
	// input handling
	void MouseUp(int button) {};
	void MouseDown(int button) {};
	void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; };
	void KeyUp(int key) {};
	void KeyDown(int key) {};
	// data members
	int2 mousePos;
};

} // namespace Tmpl8