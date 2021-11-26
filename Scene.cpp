#include "precomp.h"
#include "Scene.h"


Scene::Scene() {
	scene.push_back(new Sphere(float3(0, 2, 0), 2, Material::normal));
	scene.push_back(new Sphere(float3(3, 0, 0), 1, Material::red));
	scene.push_back(new Sphere(float3(3, 1, 2.5), 1, Material::magenta));
	scene.push_back(new Sphere(float3(-3, 1, -1), 1, Material::cyan));
	scene.push_back(new Sphere(float3(-4, 2, -4), 2, Material::mirror));
	scene.push_back(new Plane(float3(0, 1, 0), 0, Material::checkerboard));
}

float3 Scene::trace_scene(Ray& r, float3& energy, const float3& sun_dir, int max_bounces) {
	if (max_bounces == 0) {
		return float3(0, 0, 0);
	}
	find_intersection(scene, r);


	if (r.hitptr != nullptr) {
		float3 hitPos = r.o + r.d * r.t;
		float3 normal = r.hitptr->get_normal(hitPos);
		MaterialData m = materials[r.hitptr->m];

		float d = 1 - m.specularity;
		float s = m.specularity;

		float3 material_color = m.get_color(hitPos, normal);
		Ray sun_ray = Ray(hitPos + sun_dir*0.00001, sun_dir);
		find_intersection(scene, sun_ray);
		float diffuse_contrib = 0;
		if (max_bounces == 1) {
			diffuse_contrib = 1.f;
		} else if (sun_ray.hitptr == nullptr) {
			diffuse_contrib = max(dot(sun_dir, normal), 0.f); //TODO: add distance illumination
		}




		/*energy = energy - (d * (1 - diffuse_color)); //TODO implement energy 
		energy.x = max(energy.x, 0.0f);
		energy.y = max(energy.y, 0.0f);
		energy.z = max(energy.z, 0.0f);*/
		float3 specular_color = float3(0, 0, 0);
		if (m.specularity > 0.f) {
			float3 new_dir = reflect(r.d, normal);
			Ray bounced_ray = Ray(hitPos + new_dir*0.1f, new_dir);
			specular_color = trace_scene(bounced_ray, energy, sun_dir, max_bounces-1);
		}
		return material_color * ((d * diffuse_contrib) + (s * specular_color));
	}
	else {
		return float3(0.6,0.8,0.95);//rainbow sky float3(fabs(r.d.x), fabs(r.d.y), fabs(r.d.z));
	}
}

void Scene::find_intersection(const std::vector<PrimitiveGeometry*>& scene, Ray& r) {
	for (auto& obj : scene) {
		obj->intersects(r);
	}
}

void Scene::delete_scene()
{
	/*for (auto p : scene) {
		delete p;
	}*/
}
