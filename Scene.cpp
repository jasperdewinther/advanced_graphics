#include "precomp.h"
#include "Scene.h"


Scene::Scene() {
	primitives.push_back(new Sphere(float3(0, 2, 0), 2, Material::normal));
	primitives.push_back(new Sphere(float3(3, 0, 0), 1, Material::red));
	primitives.push_back(new Sphere(float3(3, 1, 2.5), 1, Material::magenta));
	primitives.push_back(new Sphere(float3(-3, 1, -1), 1, Material::cyan));
	primitives.push_back(new Sphere(float3(-4, 2, -4), 2, Material::mirror));

	primitives.push_back(new Plane(float3(0, 1, 0), 0, Material::checkerboard));
	primitives.push_back(new Plane(float3(0, -1, 0), 20, Material::white));
	primitives.push_back(new Plane(float3(1, 0, 0), 20, Material::white));
	primitives.push_back(new Plane(float3(-1, 0, 0), 20, Material::white));
	primitives.push_back(new Plane(float3(0, 0, 1), 20, Material::white));
	primitives.push_back(new Plane(float3(0, 0, -1), 20, Material::white));

	lights.push_back(new PointLight(float3(0,10,0), float3(1,1,1), 10000.0));
	//lights.push_back(new PointLight(float3(0, 2, 5), float3(1, 1, 1), 50.0));
}

float3 Scene::trace_scene(Ray& r, int max_bounces, const PrimitiveGeometry* to_ignore) const {
	if (max_bounces == 0) {
		return float3(0, 0, 0);
	}
	find_intersection(primitives, r, to_ignore);


	if (r.hitptr != nullptr) {
		float3 hitPos = r.o + r.d * r.t;
		float3 normal = r.hitptr->get_normal(hitPos);
		MaterialData m = materials[r.hitptr->m];

		float d = 1 - m.specularity;
		float s = m.specularity;

		float3 material_color = m.get_color(hitPos, normal);
		float3 direct_light = float3(0,0,0); 
		float3 specular_color = float3(0,0,0);

		if (s > 0.f) {
			Ray bounced_ray = Ray(hitPos, reflect(r.d, normal));
			specular_color = trace_scene(bounced_ray, max_bounces-1, r.hitptr);
		}
		if (d > 0.f) {
			direct_light = find_direct_light_value(primitives, hitPos, normal, r.hitptr);
		}
		return material_color * ((d * direct_light) + (s * specular_color));
	}
	else {
		return float3(0,0,0);//rainbow sky float3(fabs(r.d.x), fabs(r.d.y), fabs(r.d.z));
	}
}

void Scene::find_intersection(const std::vector<PrimitiveGeometry*>& scene, Ray& r, const PrimitiveGeometry* to_ignore) const {
	for (auto& obj : scene) {
		if (obj == to_ignore) {
			continue;
		}
		obj->intersects(r);
	}
}

float3 Scene::find_direct_light_value(const std::vector<PrimitiveGeometry*>& scene, const float3& start_pos, const float3& normal, const PrimitiveGeometry* to_ignore) const {
	float3 l = float3(0, 0, 0);
	for (auto& obj : lights) {
		float3 vec_to_light = obj->pos - start_pos;
		float3 dir = normalize(vec_to_light);
		
		Ray r = Ray(start_pos, dir);

		find_intersection(scene, r, to_ignore);
		if (r.hitptr != nullptr && r.t < sqrt(dot(vec_to_light, vec_to_light))) {
			continue;
		}
		l += obj->color * max(dot(dir, normal), 0.f) * obj->calculate_light_intensity(r);
	}
	return l;
}

void Scene::delete_scene()
{
	/*for (auto p : scene) {
		delete p;
	}*/
}
