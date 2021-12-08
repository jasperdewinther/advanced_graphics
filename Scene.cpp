#include "precomp.h"
#include "Scene.h"


Scene::Scene() {

	spheres = {
		Sphere(float3(0, 2, 0), 1.5, Material::glass),
		Sphere(float3(3, 1, 2.5), 1, Material::red_glass),
		Sphere(float3(-3, 1, -1), 1, Material::cyan),
		Sphere(float3(-4, 2, -4), 1, Material::mirror),
		Sphere(float3(6, 4, -6), 4, Material::red_glass),
		Sphere(float3(0, 2, -6), 2, Material::red_glass),
		Sphere(float3(-4, 1, -6), 1, Material::red_glass),
		Sphere(float3(-6, 0.5, -6), 0.5, Material::red_glass)
	};

	planes = {
		Plane(float3(0, 1, 0), 0, Material::checkerboard),
		Plane(float3(-1, 0, 0), 20, Material::white),
		Plane(float3(0, 0, -1), 20, Material::white)
	};

	
	triangles = get_mesh_from_file("./assets/sheep.obj", 0.1f, float3(0, 2, 4), Material::glass);

	lights.push_back(new PointLight(float3(19,10,19), float3(1,1,1), 50000.0));
	lights.push_back(new SpotLight(float3(15, 10, 0), float3(0, -1, 0), 0.5f, float3(0.1, 0.5, 0.99), 30000.f));
	lights.push_back(new DirectionalLight(float3(1,-1, 0.5), float3(0.9,0.9,0.9), 0.7 ));
}

float3 Scene::trace_scene(Ray& r, int max_bounces) const {
	if (max_bounces == 0) {
		return float3(0, 0, 0);
	}
	find_intersection(r);

	if (r.hitptr != nullptr) {
		float3 hitPos = r.o + r.d * r.t;
		float3 normal = get_normal_hitptr(r, hitPos);
		bool leaving = false;
		if (dot(r.d, normal) > 0) { // if inside object the normal should be reversed
			normal *= -1;
			leaving = true;
		}
		Material mat = get_material_hitptr(r);
		MaterialData m = materials[(int)mat];

		float3 material_color = m.get_color(hitPos, normal);
		float3 refraction_color = float3(0, 0, 0);
		float3 direct_light = float3(0, 0, 0);
		float3 specular_color = float3(0, 0, 0);

		float d = 0;
		float s = 0;
		float i = 0;

		if (m.transparent < 1.f) {
			float angle_in = dot(normal, -r.d);
			float n1 = leaving ? m.refractive_index : 1.f;
			float n2 = leaving ? 1.f : m.refractive_index;
			float refractive_ratio = n1/n2;
			float k = 1.f - pow(refractive_ratio, 2.f) * (1 - pow(angle_in, 2.f));
			if ( k < 0.f) {
				// total internal reflection
				s = 1.0;
			} else {
				float3 new_dir = normalize(refractive_ratio * r.d + normal * (refractive_ratio * angle_in - sqrt(k)));

				Ray refracted_ray = Ray(hitPos + new_dir * 0.00001, new_dir);
				refraction_color = trace_scene(refracted_ray, max_bounces - 1);

				float angle_out = dot(-normal, new_dir);

				float Fr_par = pow((n1 * angle_in - n2 * angle_out) / (n1 * angle_in + n2 * angle_out), 2.f);
				float Fr_per = pow((n1 * angle_out - n2 * angle_in) / (n1 * angle_out + n2 * angle_in), 2.f);
				float Fr = (Fr_par + Fr_per) / 2.f;

				s = Fr;
				i = 1.f-Fr;
			}
		}
		else {
			d = 1 - m.specularity;
			s = m.specularity;
		}
		if (s > 0.f) {
			float3 new_dir = reflect(r.d, normal);
			Ray bounced_ray = Ray(hitPos + new_dir*0.00001, new_dir);
			specular_color = trace_scene(bounced_ray, max_bounces-1);
		}
		if (d > 0.f) {
			direct_light = find_direct_light_value(hitPos, normal);
		}
		if (leaving) {
			float3 color = material_color * ((s * specular_color) + (i * refraction_color));
			float3 absorbtion = (-material_color * m.transparent * r.t);
			color.x *= exp(absorbtion.x);
			color.y *= exp(absorbtion.y);
			color.z *= exp(absorbtion.z);
			return color;
		}
		return material_color* ((d * direct_light) + (s * specular_color)) + (i * refraction_color);
	}
	else {
		return float3(0,0,0);//rainbow sky float3(fabs(r.d.x), fabs(r.d.y), fabs(r.d.z));
	}
}

void Scene::find_intersection(Ray& r) const {
	for (int i = 0; i < spheres.size(); i++) {
		spheres[i].intersects(r);
	}
	for (int i = 0; i < planes.size(); i++) {
		planes[i].intersects(r);
	}
	for (int i = 0; i < triangles.size(); i++) {
		triangles[i].intersects(r);
	}
}

float3 Scene::find_direct_light_value(const float3& start_pos, const float3& normal) const {
	float3 l = float3(0, 0, 0);
	for (auto& obj : lights) {
		float3 offset_start_pos = (start_pos + normal * 0.00001);
		float3 vec_to_light = obj->pos - offset_start_pos;
		float3 dir = normalize(vec_to_light);
		
		Ray r = Ray(offset_start_pos, dir);

		find_intersection(r);
		if (r.hitptr != nullptr && r.t < sqrt(dot(vec_to_light, vec_to_light))) {
			continue;
		}
		l += obj->color * max(dot(dir, normal), 0.f) * min(obj->calculate_light_intensity(r), 1.f);
	}
	return l;
}

void Scene::delete_scene()
{
	for (auto p : lights) {
		delete p;
	}
}
