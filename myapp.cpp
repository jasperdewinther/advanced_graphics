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
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void MyApp::Tick( float deltaTime )
{
	total_time += deltaTime/1000;
	Vector3 camera_pos = Vector3(0, 0, 0);
	Vector3 camera_dir = Vector3(sin(total_time / 10), 0, cos(total_time / 10));
	std::vector<Ray> rays = generate_primary_rays(camera_pos, camera_dir, 90, screen->width, screen->height);
	printf("time: %f\n", deltaTime);
	// clear the screen to black
	screen->Clear( 0 );
	// print something to the console window
	//printf( "hello world!\n" );
	// plot some colors

	for( int x = 0; x < screen->width; x++ ) for( int y = 0; y < screen->height; y++ )
	{
		Ray r = rays[x + screen->width * y];
		uint red = (uint)(r.d.data[0]*255.0f);
		uint green = (uint)(r.d.data[1]*255.0f);
		uint blue = (uint)(r.d.data[2]*255.0f);
		screen->Plot( x, y, (red << 16) + (green << 8) + blue );
	}
	// plot a white pixel in the bottom right corner
	screen->Plot( SCRWIDTH - 2, SCRHEIGHT - 2, 0xffffff );
}