#include "precomp.h"
#include "Ray.h"

Ray::Ray(float3 origin, float3 direction):
	o(origin),
	d(direction),
	t(std::numeric_limits<float>::max()),
	hitptr(nullptr)
{}


std::vector<Ray> generate_primary_rays(const float3& camerapos, const float3& camera_direction, float fov, int width, int height) {
	float aspect_ratio = (float)width / (float)height;
	float half_aspect_ratio = aspect_ratio / 2;
	float3 screen_center = camerapos + (camera_direction * (float)(half_aspect_ratio/tan(fov*PI / 360.0)));
	float3 side =
		cross(float3(
			camera_direction.x, 
			camera_direction.y == 0 ? 0.0000001f : 0, 
			camera_direction.z)
		,camera_direction); //the very small value is used when camera_direction[1] == 0, to still find a perpendicular vector
	float3 up = cross(camera_direction,side);
	side = normalize(side);
	up = normalize(up);
	//if (side.x < 0) side = side * -1;
	//if (up.y < 0) up = up * -1;

	std::vector<Ray> rays = {};
	rays.reserve(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float px = ((float)x / (float)width);
			float py = ((float)y / (float)height);
			float3 dir = screen_center + (side * ((px-0.5f) * aspect_ratio)) + (up * ((py*-1)+0.5f));
			dir = dir - camerapos;
			dir = normalize(dir);
			rays.push_back(Ray(camerapos, dir));
			//printf("%s %s %s %s %s %i %i %s\n", camerapos.to_string().c_str(), camera_direction.to_string().c_str(), screen_center.to_string().c_str(), side.to_string().c_str(), up.to_string().c_str(), x, y, dir.to_string().c_str());
		}
	}

	return rays;
}

