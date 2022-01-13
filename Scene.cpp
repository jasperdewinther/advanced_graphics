#include "precomp.h"
#include "Scene.h"


const float epsilon = 0.0001;

Scene::Scene()
{

}

float RandomFloat(xorshift_state* s)
{
	return xorshift32(s) * 2.3283064365387e-10f;
}

float3 CosineWeightedDiffuseReflection(float3 normal, int random_number)
{
	xorshift_state rand = { random_number };
	RandomFloat(&rand);
	float r0 = RandomFloat(&rand), r1 = RandomFloat(&rand);
	float r = sqrt(r0);
	float theta = 2 * PI * r1;
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	float3 dir = normalize(float3(x, y, sqrt(1 - r0)));
	return  dot(dir, normal) > 0.f ? dir : dir * -1;
}

float3 DiffuseReflection(float3 normal, int random_number) {
	xorshift_state rand = { random_number };
	float3 direction;
	while (true) {
		direction = float3(
			(RandomFloat(&rand) * 2.f) - 1.f,
			(RandomFloat(&rand) * 2.f) - 1.f,
			(RandomFloat(&rand) * 2.f) - 1.f
		);
		float l = length(direction);
		direction = normalize(direction);
		if (length(direction) >= l) break;
	}

	return dot(direction, normal) > 0.f ? direction : direction * -1;
}

float3 Scene::trace_scene(Ray& r, uint bounces, bool complexity_view, int rand) const {

	float3 T = float3(1, 1, 1);
	float3 E = float3(0, 0, 0);
	if (bounces == 0) return E;
	//while (1)
	//{
		find_intersection(r);
		if (r.hitptr == nullptr) return float3(0,0,0);
		float3 I = r.o + r.d * r.t;
		float3 N = r.hit_normal;
		MaterialData material = get_material_hitptr(r);
		float3 albedo = material.get_color(I, N);
		if (material.isLight) return albedo;
		
		xorshift_state rand_state = { rand };

		if (material.transparent < 1.f) {
			bool leaving = dot(r.d, N) > 0;

			float angle_in = leaving ? dot(N, r.d) : dot(N, -r.d);
			float n1 = leaving ? material.refractive_index : 1.f;
			float n2 = leaving ? 1.f : material.refractive_index;
			float refractive_ratio = n1 / n2;
			float k = 1.f - pow(refractive_ratio, 2.f) * (1 - pow(angle_in, 2.f));
			if (k < 0.f) {
				// total internal reflection
				float3 specular_dir = leaving ? reflect(r.d, -N) : reflect(r.d, N);
				float3 specular_Ei = trace_scene(Ray(I + specular_dir * epsilon, specular_dir), bounces - 1, complexity_view, rand);
				return albedo * specular_Ei;
			}
			else {
				float3 new_dir = normalize(refractive_ratio * r.d + N * (refractive_ratio * angle_in - sqrt(k)));


				float angle_out = leaving ? dot(N, new_dir) : dot(-N, new_dir);

				float Fr_par = pow((n1 * angle_in - n2 * angle_out) / (n1 * angle_in + n2 * angle_out), 2.f);
				float Fr_per = pow((n1 * angle_out - n2 * angle_in) / (n1 * angle_out + n2 * angle_in), 2.f);
				float Fr = (Fr_par + Fr_per) / 2.f;


				if (RandomFloat(&rand_state) < Fr) {
					float3 specular_dir = leaving ? reflect(r.d, -N) : reflect(r.d, N);
					float3 specular_Ei = trace_scene(
						Ray(I + specular_dir * epsilon, specular_dir), 
						bounces - 1, 
						complexity_view, 
						rand); //set bounces to 0 to prevent recursion
					return albedo * specular_Ei;
				}
				else {
					Ray refracted_ray = Ray(I + new_dir * epsilon, new_dir);
					float3 refraction_Ei = trace_scene(refracted_ray, bounces - 1, complexity_view, rand);
					if (leaving) {
						float3 color = albedo * refraction_Ei;
						float3 absorbtion = (-albedo * material.transparent * r.t);
						color.x *= exp(absorbtion.x);
						color.y *= exp(absorbtion.y);
						color.z *= exp(absorbtion.z);
						return color;
					}
					return refraction_Ei;
				}
			}
		}
		if (RandomFloat(&rand_state) < material.specularity) {
			float3 specular_dir = reflect(r.d, N);
			float3 specular_Ei = trace_scene(Ray(I + specular_dir * epsilon, specular_dir), material.specularity != 0.f ? bounces - 1 : 0, complexity_view, rand); //set bounces to 0 to prevent recursion
			return albedo * specular_Ei;
		}
		else {
			float3 R = CosineWeightedDiffuseReflection(N, rand);
			//float3 R = DiffuseReflection(N, rand);
			float3 BRDF = albedo / PI;
			//float PDF = 1/ (2*PI);
			float PDF = (dot(N, R)) / PI;
			float3 diffuse_Ei = trace_scene(Ray(I + R * epsilon, R), material.specularity == 0.f ? bounces - 1 : 0, complexity_view, rand) * (dot(N, R)) / PDF;
			return BRDF * diffuse_Ei;
		}
}

void Scene::find_intersection(Ray& r) const {
	for (int i = 0; i < spheres.size(); i++) {
		spheres[i].intersects(r);
	}
	for (int i = 0; i < planes.size(); i++) {
		planes[i].intersects(r);
	}
	bvh.intersects(r);
}


void Scene::delete_scene()
{}
