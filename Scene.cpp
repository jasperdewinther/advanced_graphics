#include "precomp.h"
#include "Scene.h"


const float epsilon = 0.0001;

Scene::Scene()
{

}

float XorRandomFloat(xorshift_state* s)
{
	return xorshift32(s) * 2.3283064365387e-10f;
}

float3 CosineWeightedDiffuseReflection(float3 normal, int random_number)
{
	xorshift_state rand = { random_number };
	XorRandomFloat(&rand); //TODO find out why these two initialising randoms are needed for proper distribution
	XorRandomFloat(&rand);
	float r1 = XorRandomFloat(&rand), r0 = XorRandomFloat(&rand);
	float r = sqrt(r0);
	float theta = 2 * PI * r1;
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	float3 dir = normalize(float3(x, y, sqrt(1 - r0)));
	return  dot(dir, normal) > 0.f ? dir : dir * -1;
}

struct transparency_calc_result {
	bool reflection;
	float3 ray_pos;
	float3 ray_dir;
};

transparency_calc_result calc_transparency(const float3& ray_dir, const float3& normal, const float3& intersection, const MaterialData& material, bool leaving, xorshift_state& rand_state) {

	float angle_in = leaving ? dot(normal, ray_dir) : dot(normal, ray_dir*-1);
	float n1 = leaving ? material.refractive_index : 1.f;
	float n2 = leaving ? 1.f : material.refractive_index;
	float refractive_ratio = n1 / n2;
	float k = 1.f - pow(refractive_ratio, 2.f) * (1 - pow(angle_in, 2.f));
	if (k < 0.f) {
		// total internal reflection
		float3 specular_dir = leaving ? reflect(ray_dir, normal*-1) : reflect(ray_dir, normal);
		return transparency_calc_result{ true, intersection + specular_dir * epsilon, specular_dir};
	}
	else {
		float3 new_dir = normalize(refractive_ratio * ray_dir + normal * (refractive_ratio * angle_in - sqrt(k)));

		float angle_out = leaving ? dot(normal, new_dir) : dot(normal*-1, new_dir);

		float Fr_par = pow((n1 * angle_in - n2 * angle_out) / (n1 * angle_in + n2 * angle_out), 2.f);
		float Fr_per = pow((n1 * angle_out - n2 * angle_in) / (n1 * angle_out + n2 * angle_in), 2.f);
		float Fr = (Fr_par + Fr_per) / 2.f;


		if (XorRandomFloat(&rand_state) < Fr) {
			float3 specular_dir = leaving ? reflect(ray_dir, normal*-1) : reflect(ray_dir, normal);
			return transparency_calc_result{ true, intersection + specular_dir * epsilon, specular_dir };
		}
		else {
			return transparency_calc_result{ false, intersection + new_dir * epsilon, new_dir };
		}
	}
}


void Scene::trace_scene(
	float3* screendata,
	const uint screen_width,
	const uint screen_height,
	const float3& camerapos,
	const float3& camera_direction,
	const float fov,
	const uint bounces,
	const int rand,
	const uint nthreads
) {
	if (rays_buffer.get() == nullptr || rays_buffer.get()->size != sizeof(Ray) * screen_width * screen_height / 4) init_buffers(screen_width, screen_height);

	generate(screen_width, screen_height, camerapos, camera_direction, fov, rand, true);
	ray_count = screen_width * screen_height;

	std::atomic<int> counter;
	for (int i = 0; i < bounces; i++) {
		counter = 0;

		run_multithreaded(nthreads, ray_count, 1, false, [this, &counter, &screendata, &rand, screen_width, &screen_height](int x, int y) {
			extend(x);
			shade(x, rand*(screen_width * screen_height) + (x + screen_width * y), counter);
			connect(screendata, x);
			});
		
		ray_count = counter;
		if (ray_count == 0) break;
		active_rays = active_rays ? false : true;
	}
}

void Scene::init_buffers(uint width, uint height){
	if (rays != nullptr) delete[] rays;
	if (rays != nullptr) delete[] rays2;
	rays = (Ray*)malloc(sizeof(Ray) * width * height);
	rays2 = (Ray*)malloc(sizeof(Ray) * width * height);
	rays_buffer = std::make_unique<Buffer>(sizeof(Ray) * width * height / 4, Buffer::DEFAULT, rays);
}

void Scene::generate(
	const uint screen_width,
	const uint screen_height,
	const float3& camerapos,
	const float3& camera_direction,
	const float fov,
	const int rand,
	const bool primary
) {
	generate_primary_rays(camerapos, camera_direction, fov, screen_width, screen_height, rays, ray_gen_kernel.get(), rays_buffer.get(), rand);
}
void Scene::extend(uint i) {
	Ray& r = active_rays ? rays2[i] : rays[i];
	find_intersection(r);
}
void Scene::shade(uint i, const int rand, std::atomic<int>& new_ray_index) {
	Ray& r = active_rays ? rays2[i] : rays[i];
	if (r.hitptr == nullptr) return;
	MaterialData material = materials[r.hitptr->m];
	float3 albedo = material.color;
	if (material.isLight) {
		r.E += r.T * albedo;
		return;
	};
	float3 N = r.hitptr->get_normal();
	float3 I = r.o + r.d * r.t;

	xorshift_state rand_state = { rand };
	XorRandomFloat(&rand_state);

	if (material.transparent < 1.f) {
		bool leaving = dot(r.d, N) > 0;

		transparency_calc_result result = calc_transparency(r.d, N, I, material, leaving, rand_state);
		if (result.reflection) {
			Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E + r.T * albedo, r.T * albedo); // todo check E and T calculations
			if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
			return;
		}
		else {
			if (leaving) {
				float3 color = albedo * r.T;
				float3 absorbtion = (-albedo * material.transparent * r.t);
				color.x *= exp(absorbtion.x);
				color.y *= exp(absorbtion.y);
				color.z *= exp(absorbtion.z);
				Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E + r.T * color, r.T * color);
				if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
				return;
			}
			Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T);
			if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
			return;
		}
	}
	if (XorRandomFloat(&rand_state) < material.specularity) {
		float3 specular_dir = reflect(r.d, N);
		Ray new_r = Ray(I + specular_dir * epsilon, specular_dir, r.pixel_id, r.E + r.T * albedo, r.T * albedo); // todo check E and T calculations
		if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
		return;
	}
	else {
		float3 R = CosineWeightedDiffuseReflection(N, rand);
		float3 BRDF = albedo / PI;
		float PDF = dot(N, R) / PI;
		Ray new_r = Ray(I + R * epsilon, R, r.pixel_id, r.E /* + r.T * BRDF * (dot(N, R) / PDF)*/, r.T * BRDF * (dot(N, R) / PDF)); // todo check E and T calculations
		if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
		return;
	}
}

void Scene::connect(float3* screendata, uint i){
	Ray& r = active_rays ? rays2[i] : rays[i];
	Ray& r2 = active_rays ? rays2[i+1] : rays[i+1];
	Ray& r3 = active_rays ? rays2[i + 2] : rays[i + 2];
	screendata[r.pixel_id] += r.E;
}

void Scene::find_intersection(Ray& r) const {
	bvh.intersects(r);
}