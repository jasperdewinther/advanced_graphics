#pragma once
#include "PrimitiveGeometry.h"
#include "Scene.h"

namespace Tmpl8
{

class MyApp : public TheApp
{
private:
	Scene s;
	float scene_progress; //between 0 and 1 where a scene progress of 0 has the same camera position as 1;
	bool block_progress;
	Timer total_time;
	int nthreads = 1;
	int old_width;
	int old_height;
	Ray* rays;
	float3* pixel_colors;

	void fix_ray_buffer();
	void set_progression();
	void trace_rays();
	void render_pixels();

public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void Shutdown();
	// input handling
	void MouseUp(int button) { /* implement if you want to detect mouse button presses */ };
	void MouseDown(int button) { /* implement if you want to detect mouse button presses */ };
	void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; };
	void KeyUp(int key) {};
	void KeyDown(int key);
	// data members
	int2 mousePos;
};

} // namespace Tmpl8