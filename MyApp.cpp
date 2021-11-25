#include "precomp.h"
#include "myapp.h"
#include "Ray.h"
#include "Utils.h"
#include <format>
#include <Sphere.h>



TheApp* CreateApp() { return new MyApp(); }

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void MyApp::Init()
{
	// anything that happens only once at application start goes here
	scene.push_back(Sphere(float3(0, 0, 5), 1));
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	total_time += deltaTime/1000;
	float3 camera_pos = float3(0, 0, 0);
	float3 camera_dir = float3(sin(total_time / 10), 0, cos(total_time / 10));
	std::vector<Ray> rays = generate_primary_rays(camera_pos, camera_dir, 90, screen->width, screen->height);
	printf("time: %f dir: %f %f %f\n", deltaTime, camera_dir.x, camera_dir.y, camera_dir.z);
	
	// clear the screen to black
	screen->Clear( 0 );
	// print something to the console window
	//printf( "hello world!\n" );
	// plot some colors

	for( int x = 0; x < screen->width; x++ ) for( int y = 0; y < screen->height; y++ )
	{
		Ray r = rays[x + screen->width * y];
		for (auto& obj : scene) {
			obj.intersects(r);
		}
		if (r.hitptr != nullptr) {
			float3 normal =  r.hitptr->get_normal(r.o + r.d * r.t);
			MaterialData m = materials[r.hitptr->m];

			uint red = (uint)(m.x*255);
			uint green = (uint)();
			uint blue = (uint)(r.d.z * 255.0f);
			screen->Plot(x, y, (red << 16) + (green << 8) + blue);
		}
		else {
			uint red = (uint)(r.d.x * 255.0f);
			uint green = (uint)(r.d.y * 255.0f);
			uint blue = (uint)(r.d.z * 255.0f);
			screen->Plot(x, y, (red << 16) + (green << 8) + blue);
		}


	}
	float3 top_left = rays[0].d;
	float3 top_right = rays[screen->width - 1].d;
	float3 bot_left = rays[screen->height * (screen->width - 1) + 1].d;
	float3 bot_right = rays[screen->height * screen->width - 1].d;
	printf("dot tops: %f\n", dot(top_left, top_right));
	printf("dot bots: %f\n", dot(bot_left, bot_right));
	printf("top left -- %f %f %f\n", top_left.x, top_left.y, top_left.z);
	printf("top rigt -- %f %f %f\n", top_right.x, top_right.y, top_right.z);
	printf("bot left -- %f %f %f\n", bot_left.x, bot_left.y, bot_left.z);
	printf("bot rigt -- %f %f %f\n", bot_right.x, bot_right.y, bot_right.z);

	// plot a white pixel in the bottom right corner
	screen->Plot( SCRWIDTH - 2, SCRHEIGHT - 2, 0xffffff );
}