#include "precomp.h"
#include "Ray.h"

Ray::Ray(Vector3 origin, Vector3 direction):
	o(origin),
	d(direction),
	t(std::numeric_limits<float>::max())
{}
