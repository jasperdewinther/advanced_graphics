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
public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void Shutdown();
	// input handling
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { mousePos.x = x, mousePos.y = y; }
	void KeyUp(int key);
	void KeyDown(int key);
	// data members
	int2 mousePos;
};

} // namespace Tmpl8