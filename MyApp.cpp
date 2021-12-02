#include "precomp.h"
#include "myapp.h"
#include "Ray.h"
#include <format>
#include "Sphere.h"
#include "Plane.h"


TheApp* CreateApp() { return new MyApp(); }

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void MyApp::Init()
{
	// anything that happens only once at application start goes here
	total_time = Timer();
	s = Scene();
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	fix_ray_buffer();
	set_progression();
	

	float3 camera_pos = float3(sin(scene_progress * PI*2)*10, 3, cos(scene_progress * PI*2)*10);
	float3 camera_dir = normalize(-camera_pos);


	Timer t = Timer();
	generate_primary_rays(camera_pos, camera_dir, 90 + sin(scene_progress*PI*2)*40, screen->width, screen->height, rays, nthreads);
	float time_ray_gen = t.elapsed();


	screen->Clear( 0 );
	t.reset();
	render_scene();
	float time_trace = t.elapsed();


	printf("ray: %f trace: %f\n", time_ray_gen, time_trace);
}

void Tmpl8::MyApp::fix_ray_buffer()
{
	if (rays == nullptr || screen->width != old_width || screen->height != old_height) {
		delete[] rays;
		rays = (Ray*)malloc(sizeof(Ray) * screen->width * screen->height);
		old_width = screen->width;
		old_height = screen->height;
	}
}

void Tmpl8::MyApp::set_progression()
{
	if (!block_progress) {
		scene_progress += ((100.0 / 1000) / 10);
		scene_progress = scene_progress - (int)scene_progress;
	}
}

void Tmpl8::MyApp::render_scene()
{
	float columns_per_thread = (float)screen->height / (float)nthreads;
	float rows_per_thread = (float)screen->width / (float)nthreads;
	std::vector<std::thread> t = {};
	t.reserve(nthreads*nthreads);
	for (int i = 0; i < nthreads*nthreads; i++) {
		t.push_back(std::thread([i, &columns_per_thread, &rows_per_thread, this]() {
			for (int y = (int)((i/nthreads) * columns_per_thread); y < (float)((i / nthreads) + 1) * columns_per_thread; y++) {
				for (int x = (int)((i%nthreads) * rows_per_thread); x < (float)((i % nthreads) + 1) * rows_per_thread; x++) {
					Ray r = rays[x + screen->width * y];
					float3 color = s.trace_scene(r, 100);

					uint red = min((uint)(color.x * 255.0f), 255u);
					uint green = min((uint)(color.y * 255.0f), 255u);
					uint blue = min((uint)(color.z * 255.0f), 255u);

					screen->Plot(x, y, (red << 16) + (green << 8) + blue);
				}
			}
			}));
	}

	for (auto& thread : t) {
		thread.join();
	}
}

void Tmpl8::MyApp::KeyDown(int key)
{
	
	if (key == 32) { //space stops scene
		block_progress = block_progress?false:true;
		printf("scene progress: %s\n", block_progress?"true":"false");
	}
	if(key == 90) { //z toggles multithreading
		nthreads = nthreads == 1 ? (int)std::thread::hardware_concurrency() : 1;
		printf("number of threads used: %i\n", nthreads);
	}
}

void MyApp::Shutdown() {
	printf("total runtime: %f", total_time.elapsed());
	s.delete_scene();
	delete[] rays;
}



