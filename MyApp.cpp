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
	ctx = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	virtual_width = screen->width / upscaling;
	virtual_height = screen->height / upscaling;
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	Timer t = Timer();
	fix_ray_buffer();
	set_progression();
	
	float3 camera_pos = float3(sin(scene_progress * PI*2)*10, view_height, cos(scene_progress * PI*2)*10);
	float3 camera_dir = normalize(float3(0,2,0) - camera_pos);
	float time_setup = t.elapsed();
	t.reset();
	
	generate_primary_rays(camera_pos, camera_dir, (float)fov, virtual_width, virtual_height, rays, nthreads);
	float time_ray_gen = t.elapsed();


	screen->Clear( 0 );

	t.reset();
	trace_rays();
	render_pixels();
	float time_trace = t.elapsed();
	
	printf("setup: %f ray: %f trace: %f\n", time_setup, time_ray_gen, time_trace);
	printf("--------------------------\n");
}

void Tmpl8::MyApp::fix_ray_buffer()
{
	if (rays == nullptr || virtual_width != old_width || virtual_height != old_height) {
		delete[] rays;
		rays = (Ray*)malloc(sizeof(Ray) * virtual_width * virtual_height);

		delete[] pixel_colors;
		pixel_colors = (float3*)malloc(sizeof(float3) * virtual_width * virtual_height);

		old_width = virtual_width;
		old_height = virtual_height;
	}
}

void Tmpl8::MyApp::set_progression()
{
	if (!block_progress) {
		scene_progress += ((100.0 / 1000) / 10);
		/*if ((int)scene_progress == 1) {
			printf("total runtime: %f", total_time.elapsed());
			throw exception("full circle");
		}*/
		scene_progress = scene_progress - (int)scene_progress;
	}
}

void Tmpl8::MyApp::trace_rays()
{
	std::vector<std::thread> t = {};
	t.reserve(nthreads * nthreads);
	for (int i = 0; i < nthreads*nthreads; i++) {
		t.push_back(std::thread([i, this]() {
			//Timer t = Timer();

			for (float y = (float)i / nthreads; y < virtual_height; y += (float)nthreads) {
				for (float x = (float)(i % nthreads); x < virtual_width; x += (float)nthreads) {
					Ray r = rays[(int)x + virtual_width * (int)y];
					pixel_colors[(int)x + virtual_width * (int)y] = s.trace_scene(r, bounces);
				}
			}
			//printf("ray: %f i: %i\n", t.elapsed(), i);
			}));
	}

	for (auto& thread : t) {
		thread.join();
	}
}

void Tmpl8::MyApp::render_pixels() {
	if (upscaling == 1) {
		for (int y = 0; y < virtual_height; y++) {
			for (int x = 0; x < virtual_width; x++) {
				float3 color = pixel_colors[x + screen->width * y];

				uint red = min((uint)(color.x * 255.0f), 255u);
				uint green = min((uint)(color.y * 255.0f), 255u);
				uint blue = min((uint)(color.z * 255.0f), 255u);

				screen->Plot(x, y, (red << 16) + (green << 8) + blue);
			}
		}
	}
	else {
		for (int y = 0; y < virtual_height; y++) {
			for (int x = 0; x < virtual_width; x++) {
				float3 color = pixel_colors[x + virtual_width * y];
				for (int y2 = 0; y2 < upscaling; y2++) {
					for (int x2 = 0; x2 < upscaling; x2++) {

						uint red = min((uint)(color.x * 255.0f), 255u);
						uint green = min((uint)(color.y * 255.0f), 255u);
						uint blue = min((uint)(color.z * 255.0f), 255u);

						screen->Plot(x*upscaling+x2, y*upscaling+y2, (red << 16) + (green << 8) + blue);
					}
				}
			}
		}
	}
}
void Tmpl8::MyApp::PostDraw()
{
	
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	bool new_multithreading = multithreading;
	ImGui::Checkbox("Multithreading", &new_multithreading);
	if (multithreading != new_multithreading) {
		multithreading = new_multithreading;
		nthreads = multithreading ? (int)std::thread::hardware_concurrency() : 1;
	}
	ImGui::Text("number of threads used: %i", nthreads);

	ImGui::SliderInt("bounces", &bounces, 0, 20);
	ImGui::SliderFloat("scene progress", &scene_progress, 0.f, 1.f);
	ImGui::SliderInt("fov", &fov, 0, 180);
	ImGui::SliderFloat("view height", &view_height, 0.1f, 19.9f);
	if (ImGui::SliderInt("upscaling", &upscaling, 1, 8)) {
		virtual_width = screen->width / upscaling;
		virtual_height = screen->height / upscaling;
		fix_ray_buffer();
	}
	ImGui::Checkbox("block scene progress", &block_progress);
	ImGui::Text("last frame time: %f", ImGui::GetIO().DeltaTime);
	float3 colors = pixel_colors[(int)(mousePos.x / upscaling + (mousePos.y / upscaling) * virtual_width)];
	ImGui::Text("cursor color: %f %f %f",colors.x, colors.y, colors.z);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MyApp::Shutdown() {
	printf("total runtime: %f", total_time.elapsed());
	s.delete_scene();
	delete[] rays;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



