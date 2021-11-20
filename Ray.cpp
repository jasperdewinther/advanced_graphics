#include "precomp.h"
#include "Ray.h"

Ray::Ray(Vector3 origin, Vector3 direction):
	o(origin),
	d(direction),
	t(std::numeric_limits<float>::max()),
	hitptr(nullptr)
{}


std::vector<Ray> generate_primary_rays(const Vector3& camerapos, const Vector3& camera_direction, float fov, int width, int height) {
	float aspect_ratio = (float)width / (float)height;
	Vector3 screen_center = camerapos + (camera_direction * (float)((aspect_ratio / 2.0)/tan(fov*PI / 360.0)));
	Vector3 side = 
		Vector3(
			camera_direction.data[0], 
			camera_direction.data[1] == 0 ? (float)0.0000001 : 0, 
			camera_direction.data[2])
		.cross(camera_direction); //the very small value is used when camera_direction[1] == 0, to still find a perpendicular vector
	Vector3 up = camera_direction.cross(side);
	side.normalize();
	up.normalize();
	printf("%s %s %s\n", screen_center.to_string().c_str(), side.to_string().c_str(), up.to_string().c_str());
	if (side.data[0] < 0) side = side * -1;
	if (up.data[1] < 0) up = up * -1;
	printf("%s %s %s\n", screen_center.to_string().c_str(), side.to_string().c_str(), up.to_string().c_str());

	Vector3 P0 = screen_center - (side*aspect_ratio) + up; //top left of screen
	Vector3 P1 = screen_center + (side*aspect_ratio) + up; //top right of screen
	Vector3 P2 = screen_center - (side*aspect_ratio) - up; //bottom left of screen
	Vector3 P3 = screen_center + (side*aspect_ratio) - up; //bottom right of screen

	std::vector<Ray> rays = {};

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			rays.push_back(Ray(camerapos, ))
		}
	}
	return std::vector<Ray>();
}