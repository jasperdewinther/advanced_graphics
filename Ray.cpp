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
	float half_aspect_ratio = aspect_ratio / 2;
	Vector3 screen_center = camerapos + (camera_direction * (float)(half_aspect_ratio/tan(fov*PI / 360.0)));
	Vector3 side = 
		Vector3(
			camera_direction.data[0], 
			camera_direction.data[1] == 0 ? 0.0000001f : 0, 
			camera_direction.data[2])
		.cross(camera_direction); //the very small value is used when camera_direction[1] == 0, to still find a perpendicular vector
	Vector3 up = camera_direction.cross(side);
	side.normalize();
	up.normalize();
	if (side.data[0] < 0) side = side * -1;
	if (up.data[1] < 0) up = up * -1;

	std::vector<Ray> rays = {};

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float px = ((float)x / (float)width);
			float py = ((float)y / (float)width);
			Vector3 dir = screen_center + (side * (px * aspect_ratio)) + (up * py);
			dir = dir - camerapos;
			dir.normalize();
			rays.push_back(Ray(camerapos, dir));
			//printf("%s %s %s %s %s %i %i %s\n", camerapos.to_string().c_str(), camera_direction.to_string().c_str(), screen_center.to_string().c_str(), side.to_string().c_str(), up.to_string().c_str(), x, y, dir.to_string().c_str());
		}
	}

	return rays;
}