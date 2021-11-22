#include "precomp.h"
#include "myapp.h"
#include "Ray.h"
#include "Utils.h"
#include <format>

TheApp* CreateApp() { return new MyApp(); }

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void MyApp::Init()
{
	// anything that happens only once at application start goes here
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
		uint red = (uint)(r.d.x*255.0f);
		uint green = (uint)(r.d.y*255.0f);
		uint blue = (uint)(r.d.z*255.0f);
		screen->Plot( x, y, (red << 16) + (green << 8) + blue );
	}
	printf("top left -- %f %f %f\n", rays[0].d.x, rays[0].d.y, rays[0].d.z);
	printf("top rigt -- %f %f %f\n", rays[screen->width - 1].d.x, rays[screen->width - 1].d.y, rays[screen->width - 1].d.z);
	printf("bot left -- %f %f %f\n", rays[screen->height * (screen->width - 1) + 1].d.x, rays[screen->height * (screen->width - 1) + 1].d.y, rays[screen->height * (screen->width - 1) + 1].d.z);
	printf("top rigt -- %f %f %f\n", rays[screen->height * screen->width - 1].d.x, rays[screen->height * screen->width - 1].d.y, rays[screen->height * screen->width - 1].d.z);

	// plot a white pixel in the bottom right corner
	screen->Plot( SCRWIDTH - 2, SCRHEIGHT - 2, 0xffffff );
}