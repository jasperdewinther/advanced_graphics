#pragma once
#include "PrimitiveGeometry.h"
#include "Scene.h"

namespace Tmpl8
{

class MyApp : public TheApp
{
private:
	float total_time = 0;
	Scene s;
public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void Shutdown();
	// input handling
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { mousePos.x = x, mousePos.y = y; }
	void KeyUp( int key ) { /* implement if you want to handle keys */ }
	void KeyDown( int key ) { /* implement if you want to handle keys */ }
	// data members
	int2 mousePos;
};

} // namespace Tmpl8