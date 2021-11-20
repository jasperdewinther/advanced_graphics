#include "precomp.h"
#include "myapp.h"
#include "Vector.h"
#include "Ray.h"

TheApp* CreateApp() { return new MyApp(); }

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void MyApp::Init()
{
	// anything that happens only once at application start goes here
	Vector3 camera_pos = Vector3(0, 0, 0);
	Vector3 camera_dir = Vector3(0, 0, 1);
	generate_primary_rays(camera_pos, camera_dir, 60, 100, 100);
	generate_primary_rays(camera_pos, camera_dir, 120, 1920, 1080);
	generate_primary_rays(camera_pos, camera_dir, 60, 1920, 1080);
	generate_primary_rays(camera_pos, camera_dir, 30, 1920, 1080);
	camera_pos = Vector3(1, 1, 0);
	camera_dir = Vector3(0, 1, 1);
	camera_dir.normalize();
	generate_primary_rays(camera_pos, camera_dir, 60, 1920, 1080);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	total_time += deltaTime/1000;
	//printf("time: %f\n", total_time);
	// clear the screen to black
	screen->Clear( 0 );
	// print something to the console window
	//printf( "hello world!\n" );
	// plot some colors
	for( int red = 0; red < 256; red++ ) for( int green = 0; green < 256; green++ )
	{
		int x = red, y = green;
		screen->Plot( x + 200, y + 100, (red << 16) + (green << 8) );
	}
	// plot a white pixel in the bottom right corner
	screen->Plot( SCRWIDTH - 2, SCRHEIGHT - 2, 0xffffff );
}