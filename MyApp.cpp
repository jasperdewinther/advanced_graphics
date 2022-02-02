#include "precomp.h"
#include "myapp.h"

TheApp* CreateApp() { 
	Kernel::InitCL();
	return new MyApp();
}

void MyApp::Init()
{
	total_time = Timer();
	ctx = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	virtual_width = screen->width / upscaling;
	virtual_height = screen->height / upscaling;
	fix_buffers();

	printf("ray size: %i\n", sizeof(Ray));
	printf("ray triangle: %i\n", sizeof(Triangle));
	printf("bvh node: %i\n", sizeof(BVHNode));
	printf("aabb: %i\n", sizeof(aabb));
}


void MyApp::Tick( float deltaTime )
{
	Timer t = Timer();
	if (virtual_width != old_width || virtual_height != old_height) {
		fix_buffers();
	}
	set_progression();
	float3 camera_pos = float3(sin(scene_progress * PI*2)*distance_to_center, view_height, cos(scene_progress * PI*2)*distance_to_center);
	float3 camera_dir = normalize(float3(0,2,0) - camera_pos);

	t.reset();
	trace_rays(camera_pos, camera_dir);
	accumulation_count += 1;
	time_trace = t.elapsed();

	t.reset();
	apply_post_processing();
	post_processing = t.elapsed();

	t.reset();
	render_pixels();
	time_draw = t.elapsed();
}

void Tmpl8::MyApp::fix_buffers()
{
	delete[] accumulation_buffer;
	accumulation_buffer = (float3*)malloc(sizeof(float3) * virtual_width * virtual_height);
	accumulation_buffer_gpu.delete_buffer();
	accumulation_buffer_gpu = Buffer(sizeof(float3) * virtual_width * virtual_height / 4, Buffer::DEFAULT, accumulation_buffer);

	delete[] accumulation_buffer_2nd;
	accumulation_buffer_2nd = (float3*)malloc(sizeof(float3) * virtual_width * virtual_height);
	reset_image();

	delete[] pixel_colors;
	pixel_colors = (float3*)malloc(sizeof(float3) * virtual_width * virtual_height);
	pixel_colors_buffer.delete_buffer();
	pixel_colors_buffer = Buffer(sizeof(float3) * virtual_width * virtual_height / 4, Buffer::DEFAULT, pixel_colors);

	delete[] temp_image;
	temp_image = (float3*)malloc(sizeof(float3) * virtual_width * virtual_height);

	old_width = virtual_width;
	old_height = virtual_height;
}

void Tmpl8::MyApp::reset_image() {
	for (int i = 0; i < virtual_width * virtual_height; i++) accumulation_buffer[i] = float3(0, 0, 0);
	for (int i = 0; i < virtual_width * virtual_height; i++) accumulation_buffer_2nd[i] = float3(0, 0, 0);
	accumulation_count = 0;
}

void Tmpl8::MyApp::set_progression()
{
	if (!block_progress) {
		scene_progress += 0.01;
		scene_progress = scene_progress - (int)scene_progress;
	}
}


void Tmpl8::MyApp::trace_rays(const float3& camerapos, const float3& cameradir){

	run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
		temp_image[x + virtual_width * y] = float3(0, 0, 0);
		});

	s.trace_scene(temp_image, virtual_width, virtual_height, camerapos, cameradir, fov, bounces, rand(), nthreads, use_gpu);

	run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
		accumulation_buffer[x + virtual_width * y] += temp_image[x + virtual_width * y];
		});

}

void Tmpl8::MyApp::apply_post_processing(){

	run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
		float3 old_color = accumulation_buffer[x + virtual_width * y] / (float)accumulation_count;
		if (old_color.x > 1.f || old_color.y > 1.f || old_color.z > 1.f) old_color = old_color / max(old_color.x, max(old_color.y, old_color.z));
		pixel_colors[x + virtual_width * y] = float3(
			max(min(0.99f, old_color.x), 0.01f), 
			max(min(0.99f, old_color.y), 0.01f), 
			max(min(0.99f, old_color.z), 0.01f));
		});
	/*uint pixel_count = virtual_width * virtual_height;
	accumulation_buffer_gpu.CopyToDevice();
	if (!use_gpu) {
		s.b_normals_image.CopyToDevice();
		s.b_hitpos_image.CopyToDevice();
	}
	bilateral_filter_kernel->SetArgument(0, &accumulation_buffer_gpu);
	bilateral_filter_kernel->SetArgument(1, &pixel_colors_buffer);
	bilateral_filter_kernel->SetArgument(2, &s.b_normals_image);
	bilateral_filter_kernel->SetArgument(3, &s.b_hitpos_image);
	bilateral_filter_kernel->SetArgument(4, (int)accumulation_count);
	bilateral_filter_kernel->SetArgument(5, virtual_width * virtual_height);
	bilateral_filter_kernel->SetArgument(6, virtual_width);
	bilateral_filter_kernel->Run(pixel_count + (256 - (pixel_count % 256)), 256);
	pixel_colors_buffer.CopyFromDevice();*/
	

	color_counter = float3(0, 0, 0);
	run_multithreaded(1, virtual_width, virtual_height, [this](int x, int y) {
		float3 old_color = accumulation_buffer[x + virtual_width * y] / (float)accumulation_count;
		color_counter += old_color;
		});

	if (vignetting > 0.01) {
		run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
			float3 old_color = pixel_colors[x + virtual_width * y];
			float w = (float)virtual_width;
			float h = (float)virtual_height;
			float reduction = sqrt(pow((float)x / w - 0.5, 2.0) + pow((float)y / h - 0.5, 2.0)) / 0.7071067811865475244; //maximum distance is exactly 1.0 now
			reduction = 1.0 - lerp(0.f, 1.f, pow(reduction, 100 - vignetting * 100));
			pixel_colors[x + virtual_width * y] = float3(max(old_color.x * reduction, 0.f), max(old_color.y * reduction, 0.f), max(old_color.z * reduction, 0.f));
			});
	}
	if (gamma_correction > 1.01 || gamma_correction < 0.99) {
		run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
			float3 old_color = pixel_colors[x + virtual_width * y];
			pixel_colors[x + virtual_width * y] = float3(pow(old_color.x, gamma_correction), pow(old_color.y, gamma_correction), pow(old_color.z, gamma_correction));
			});
	}
	if (chromatic_aberration != 0) {
		run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
			float r;
			if (x + chromatic_aberration < virtual_width && x + chromatic_aberration >= 0) {
				r = pixel_colors[x + chromatic_aberration + virtual_width * y].x;
			}
			else {
				r = pixel_colors[x + virtual_width * y].x;
			}
			float g = pixel_colors[x + virtual_width * y].y;
			float b;
			if (x - chromatic_aberration < virtual_width && x - chromatic_aberration >= 0) {
				b = pixel_colors[x - chromatic_aberration + virtual_width * y].z;
			}
			else {
				b = pixel_colors[x + virtual_width * y].z;
			}
			temp_image[x + virtual_width * y] = float3(r,g,b);
			});
		run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
			pixel_colors[x + virtual_width * y] = temp_image[x + virtual_width * y];
			});
	}
}

uint Tmpl8::MyApp::color_to_uint(const float3& color) {
	uint red = min((uint)(color.x  * 255.0f), 255u);
	uint green = min((uint)(color.y * 255.0f), 255u);
	uint blue = min((uint)(color.z * 255.0f), 255u);
	return  (red << 16) + (green << 8) + blue;
}

void Tmpl8::MyApp::render_pixels() {
	run_multithreaded(nthreads, virtual_width, virtual_height, [this](int x, int y) {
		float3 color = pixel_colors[x + virtual_width * y];
		for (int y2 = 0; y2 < upscaling; y2++) {
			for (int x2 = 0; x2 < upscaling; x2++) {
				screen->Plot(x * upscaling + x2, y * upscaling + y2, color_to_uint(color));
			}
		}
		});
}
void Tmpl8::MyApp::PostDraw(){
	
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("settings");
	if (ImGui::Checkbox("Multithreading", &multithreading)) nthreads = multithreading ? (int)std::thread::hardware_concurrency() : 1;
	ImGui::Checkbox("use gpu", &use_gpu);
	ImGui::Text("number of threads used: %i", nthreads);
	if (ImGui::SliderInt("bounces", &bounces, 0, 20)) reset_image();
	if (ImGui::SliderFloat("scene progress", &scene_progress, 0.f, 1.f)) reset_image();
	if (ImGui::SliderInt("fov", &fov, 1, 180)) reset_image();
	if (ImGui::SliderFloat("view height", &view_height, 0.1f, 19.9f)) reset_image();
	if (ImGui::SliderInt("upscaling", &upscaling, 1, 8)) {
		virtual_width = screen->width / upscaling; 
		virtual_height = screen->height / upscaling; 
		fix_buffers(); 
		reset_image();
	}
	ImGui::SliderFloat("gamma correction", &gamma_correction, 0.f, 5.f);
	ImGui::SliderFloat("vignetting", &vignetting, 0.f, 1.f);
	ImGui::SliderInt("chromatic aberration", &chromatic_aberration, -10, 10);
	if (ImGui::SliderFloat("distance to center", &distance_to_center, 1.f, 100.f)) reset_image();
	ImGui::Checkbox("block scene progress", &block_progress);
	ImGui::Text("last frame time: %f", ImGui::GetIO().DeltaTime);
	float3 colors = pixel_colors[(int)(mousePos.x / upscaling + (mousePos.y / upscaling) * virtual_width)];
	ImGui::Text("cursor color: %f %f %f",colors.x, colors.y, colors.z);
	ImGui::Text("trace: %f", time_trace);
	ImGui::Text("post_processing: %f", post_processing);
	ImGui::Text("draw: %f", time_draw);
	ImGui::Text("accumulated: %i", accumulation_count);
	ImGui::Text("r: %f", color_counter.x);
	ImGui::Text("g: %f", color_counter.y);
	ImGui::Text("b: %f", color_counter.z);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MyApp::Shutdown() {
	printf("total runtime: %f", total_time.elapsed());
	delete[] accumulation_buffer;
	delete[] pixel_colors;
	delete[] temp_image;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



