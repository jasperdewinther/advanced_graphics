#pragma once
#include "Scene.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



/*implemented:
- generic ray tracer 
- free camera in code or along some path at runtime
- UI using imgui
- planes and spheres
- extendable material class 
- basic scene
- shadows
- reflections
- refraction
- beers law

- multithreaded rendering
- anti aliasing
- obj file loading
*/


namespace Tmpl8
{

class MyApp : public TheApp
{
private:
	Scene s;
	Timer total_time;
	int nthreads = std::thread::hardware_concurrency();
	int old_width;
	int old_height;
	Ray* rays;
	float3* pixel_colors;
	ImGuiContext* ctx;
	int virtual_width;
	int virtual_height;


	//all imgui settings
	bool multithreading = true;
	int bounces = 10;
	float scene_progress = 0; //between 0 and 1 where a scene progress of 0 has the same camera position as 1;
	bool block_progress = true;
	int fov = 90;
	float view_height = 3.f;
	int upscaling = 8;
	int antialiasing = 1;

	void fix_ray_buffer();
	void set_progression();
	void trace_rays();
	void render_pixels();

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