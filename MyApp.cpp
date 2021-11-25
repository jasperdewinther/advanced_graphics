#include "precomp.h"
#include "myapp.h"
#include "Ray.h"
#include "Utils.h"
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
	scene.push_back(new Sphere(float3(0, 1, 0), 1, Material::normal));
	scene.push_back(new Sphere(float3(3, 0, 0), 1, Material::red));
	scene.push_back(new Plane(float3(0, 1, 0), 0, Material::checkerboard));
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	Timer t = Timer();
	total_time += deltaTime/1000;
	float3 camera_pos = float3(sin(total_time / 10)*10, 3, cos(total_time / 10)*10);
	float3 camera_dir = -camera_pos;
	camera_dir = normalize(camera_dir);
	std::vector<Ray> rays = generate_primary_rays(camera_pos, camera_dir, 90 + sin(total_time / 10)*60, screen->width, screen->height);
	float t_ray_generation = t.elapsed();
	t.reset();
	
	
	// clear the screen to black
	screen->Clear( 0 );

	for( int x = 0; x < screen->width; x++ ) for( int y = 0; y < screen->height; y++ )
	{
		Ray r = rays[x + screen->width * y];
		for (auto& obj : scene) {
			obj->intersects(r);
		}
		if (r.hitptr != nullptr) {
			float3 hitPos = r.o + r.d * r.t;
			float3 normal =  r.hitptr->get_normal(hitPos);
			MaterialData m = materials[r.hitptr->m];
			float3 color = m.get_color(hitPos, normal);
			
			uint red = (uint)(color.x*255.0f);
			uint green = (uint)(color.y*255.0f);
			uint blue = (uint)(color.z*255.0f);
			screen->Plot(x, y, (red << 16) + (green << 8) + blue);
		}
		else {
			uint red = (uint)(fabs(r.d.x) * 255.0f);
			uint green = (uint)(fabs(r.d.y) * 255.0f);
			uint blue = (uint)(fabs(r.d.z) * 255.0f);
			screen->Plot(x, y, (red << 16) + (green << 8) + blue);
		}


	}
	float t_drawing_pixels = t.elapsed();
	printf("ray gen: %f draw: %f\n", t_ray_generation, t_drawing_pixels);


	float3 top_left = rays[0].d;
	float3 top_right = rays[screen->width - 1].d;
	float3 bot_left = rays[screen->height * (screen->width - 1) + 1].d;
	float3 bot_right = rays[screen->height * screen->width - 1].d;
	printf("dir: %f %f %f\n", camera_dir.x, camera_dir.y, camera_dir.z);
	printf("dot tops: %f\n", dot(top_left, top_right));
	printf("dot bots: %f\n", dot(bot_left, bot_right));
	printf("top left -- %f %f %f\n", top_left.x, top_left.y, top_left.z);
	printf("top rigt -- %f %f %f\n", top_right.x, top_right.y, top_right.z);
	printf("bot left -- %f %f %f\n", bot_left.x, bot_left.y, bot_left.z);
	printf("bot rigt -- %f %f %f\n", bot_right.x, bot_right.y, bot_right.z);
}

void MyApp::Shutdown() {
	for (auto p : scene) {
		delete p;
	}
}