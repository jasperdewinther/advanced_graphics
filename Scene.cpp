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

	active_rays = false;
	generate(screen_width, screen_height, camerapos, camera_direction, fov, rand, true);
	ray_count = screen_width * screen_height;

	Buffer b_top_bvh_nodes = Buffer(sizeof(BVHNode) * m_top_bvh_nodes.size() / 4, Buffer::DEFAULT, m_top_bvh_nodes.data());
	Buffer b_top_leaves = Buffer(sizeof(TopBVHNodeScene) * m_top_leaves.size() / 4, Buffer::DEFAULT, m_top_leaves.data());
	Buffer b_top_indices = Buffer(sizeof(uint) * m_top_indices.size() / 4, Buffer::DEFAULT, m_top_indices.data());
	Buffer b_bvh_nodes = Buffer(sizeof(BVHNode) * m_bvh_nodes.size() / 4, Buffer::DEFAULT, m_bvh_nodes.data());
	Buffer b_model_primitives_starts = Buffer(sizeof(uint) * m_model_primitives_starts.size() / 4, Buffer::DEFAULT, m_model_primitives_starts.data());
	Buffer b_model_bvh_starts = Buffer(sizeof(uint) * m_model_bvh_starts.size() / 4, Buffer::DEFAULT, m_model_bvh_starts.data());
	Buffer b_triangles = Buffer(sizeof(Triangle) * m_triangles.size() / 4, Buffer::DEFAULT, m_triangles.data());
	Buffer b_indices = Buffer(sizeof(uint) * m_indices.size() / 4, Buffer::DEFAULT, m_indices.data());
	Buffer b_rays_count = Buffer(sizeof(uint) / 4, Buffer::DEFAULT, m_rays_count.get());
	b_top_bvh_nodes.CopyToDevice();
	b_top_leaves.CopyToDevice();
	b_top_indices.CopyToDevice();
	b_bvh_nodes.CopyToDevice();
	b_model_primitives_starts.CopyToDevice();
	b_model_bvh_starts.CopyToDevice();
	b_triangles.CopyToDevice();
	b_indices.CopyToDevice();

	ray_extend_kernel->SetArgument(1, &b_top_bvh_nodes);
	ray_extend_kernel->SetArgument(2, &b_top_leaves);
	ray_extend_kernel->SetArgument(3, &b_top_indices);
	ray_extend_kernel->SetArgument(4, &b_bvh_nodes);
	ray_extend_kernel->SetArgument(5, &b_model_primitives_starts);
	ray_extend_kernel->SetArgument(6, &b_model_bvh_starts);
	ray_extend_kernel->SetArgument(7, &b_triangles);
	ray_extend_kernel->SetArgument(8, &b_indices);


	ray_shade_kernel->SetArgument(0, &b_rays_count);
	ray_shade_kernel->SetArgument(1, rays_buffer.get());
	ray_shade_kernel->SetArgument(2, rays2_buffer.get());
	ray_shade_kernel->SetArgument(3, &b_triangles);



	std::atomic<int> counter;
	for (int i = 0; i < bounces; i++) {

		Timer t = Timer();
		counter = 0;

		//extend
		if (i != 0) active_rays ? rays2_buffer->CopyToDevice() : rays_buffer->CopyToDevice();
		ray_extend_kernel->SetArgument(0, active_rays ? rays2_buffer.get() : rays_buffer.get());
		ray_extend_kernel->SetArgument(9, ray_count-1);
		ray_extend_kernel->Run(ray_count + (32 - (ray_count % 32)), 32);
		//active_rays ? rays2_buffer->CopyFromDevice() : rays_buffer->CopyFromDevice();

		//shade
		m_rays_count[0] = 0;
		b_rays_count.CopyToDevice();
		ray_shade_kernel->SetArgument(4, (int)active_rays);
		ray_shade_kernel->SetArgument(5, rand * (int)screen_width * (int)screen_height * i);
		ray_shade_kernel->SetArgument(6, ray_count-1);
		//ray_shade_kernel->Run(ray_count + (32 - (ray_count % 32)), 32);
		rays_buffer->CopyFromDevice();
		rays2_buffer->CopyFromDevice();
		b_rays_count.CopyFromDevice();
		//counter = m_rays_count[0];

		printf("%i gpu took %f seconds with %i rays left\n", i, t.elapsed(), ray_count);
		t = Timer();
		run_multithreaded(nthreads, ray_count, 1, [this, &counter, &screendata, &rand, screen_width, &screen_height](int x, int y) {
			//extend(x);
			shade(x, rand * (screen_width * screen_height) + x, counter);
			connect(screendata, x);
			});
		printf("%i cpu took %f seconds with %i rays left\n", i, t.elapsed(), ray_count);
		ray_count = counter;
		if (ray_count == 0) break;
		active_rays = active_rays ? false : true;
		

	}
	
}

void Scene::init_buffers(uint width, uint height){
	printf("rebuilding buffers\n");
	
	rays = std::make_unique<Ray[]>(width*height);
	rays2 = std::make_unique<Ray[]>(width * height);
	m_rays_count = std::make_unique<uint[]>(1);
	rays_buffer = std::make_unique<Buffer>(sizeof(Ray) * width * height / 4, Buffer::DEFAULT, rays.get());
	rays2_buffer = std::make_unique<Buffer>(sizeof(Ray) * width * height / 4, Buffer::DEFAULT, rays2.get());
	active_rays = false;


	m_top_bvh_nodes = std::vector<BVHNode>();
	m_top_leaves = std::vector<TopBVHNodeScene>();
	m_bvh_nodes = std::vector<BVHNode>();
	m_model_primitives_starts = std::vector<uint>();
	m_model_bvh_starts = std::vector<uint>();
	m_triangles = std::vector<Triangle>();
	m_indices = std::vector<uint>();

	

	uint model_start_index = 0;
	uint model_bvh_index = 0;
	for (int i = 0; i < triangles.size(); i++) {
		m_triangles.insert(m_triangles.end(), triangles[i].begin(), triangles[i].end()); // contains all triangles
		m_model_primitives_starts.push_back(model_start_index); // contains the start index in the primitive buffer for every model 
		m_model_bvh_starts.push_back(model_bvh_index); // contains the start index in the bvh buffer for every model
		m_indices.insert(m_indices.end(), bvhs[i].indices.get(), bvhs[i].indices.get() + triangles[i].size()); // contains all indices
		m_bvh_nodes.insert(m_bvh_nodes.end(), bvhs[i].pool.get(), bvhs[i].pool.get() + bvhs[i].elements_of_pool_used); // contains all bvh pools

		model_bvh_index += bvhs[i].elements_of_pool_used;
		model_start_index += triangles[i].size();
	}

	for (int i = 0; i < bvh.primitive_count; i++) {
		TopBVHNode og_node = bvh.primitives[i];
		uint index = 0;
		for (int j = 0; j < bvhs.size(); j++) {
			if (&bvhs[j] == og_node.obj) {
				index = j;
			}
		}
		TopBVHNodeScene node = TopBVHNodeScene{
			og_node.pos,
			index
		};
		m_top_leaves.push_back(node); // contains all top level bvh leaves, which point to bvh's
	}
	m_top_indices.insert(m_top_indices.begin(), bvh.indices.get(), bvh.indices.get() + bvh.primitive_count);
	m_top_bvh_nodes.insert(m_top_bvh_nodes.end(), bvh.pool.get(), bvh.pool.get() + bvh.elements_of_pool_used); // contains entire top level bvh pool
}

bool same_node(const BVHNode& n1, const BVHNode& n2) {
	return n1.parent == n2.parent && n1.leftFirst == n2.leftFirst && n1.count == n2.count;
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
	generate_primary_rays(camerapos, camera_direction, fov, screen_width, screen_height, ray_gen_kernel.get(), rays_buffer.get(), rand);
}
void Scene::extend(uint i) {
	Ray& r = active_rays ? rays2[i] : rays[i];
	intersect_top(r);
}
void Scene::shade(uint i, const int rand, std::atomic<int>& new_ray_index) {
	Ray& r = active_rays ? rays2[i] : rays[i];
	if (r.hitptr == 4294967295) return; //if default value
	MaterialData material = materials[m_triangles[r.hitptr].m];
	float3 albedo = material.color;
	if (material.isLight) {
		r.E = r.T * albedo;
		return;
	};
	float3 N = m_triangles[r.hitptr].get_normal();
	float3 I = r.o + r.d * r.t;

	xorshift_state rand_state = { rand };
	XorRandomFloat(&rand_state);

	if (material.transparent < 1.f) {
		bool leaving = dot(r.d, N) > 0;

		transparency_calc_result result = calc_transparency(r.d, N, I, material, leaving, rand_state);
		if (result.reflection) {
			Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T * albedo); // todo check E and T calculations
			if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
			return;
		}
		else {
			if (leaving) {
				float3 color = albedo;
				float3 absorbtion = (-albedo * material.transparent * r.t);
				color.x *= exp(absorbtion.x);
				color.y *= exp(absorbtion.y);
				color.z *= exp(absorbtion.z);
				Ray new_r = Ray(result.ray_pos, result.ray_dir, r.pixel_id, r.E, r.T * color);
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
		Ray new_r = Ray(I + specular_dir * epsilon, specular_dir, r.pixel_id, r.E, r.T * albedo); // todo check E and T calculations
		if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
		return;
	}
	else {
		float3 R = CosineWeightedDiffuseReflection(N, rand);
		float3 BRDF = albedo / PI;
		float PDF = dot(N, R) / PI;
		Ray new_r = Ray(I + R * epsilon, R, r.pixel_id, r.E, BRDF * (r.T * dot(N, R) / PDF)); // todo check E and T calculations
		if (active_rays) rays[new_ray_index++] = new_r; else rays2[new_ray_index++] = new_r;
		return;
	}
}

void Scene::connect(float3* screendata, uint i){
	Ray& r = active_rays ? rays2[i] : rays[i];
	screendata[r.pixel_id] = r.E;
}

void Scene::intersect_top(Ray& r) const { //assumes ray intersects
	//https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
	BVHNode last;
	BVHNode current = m_top_bvh_nodes[0];
	BVHNode near_node;
	BVHNode far_node;
	float2 intersection_test_result = r.intersects_aabb(current);
	if (intersection_test_result.x >= intersection_test_result.y || intersection_test_result.x > r.t) return; // now we know that the root is intersected and partly closer than the furthest already hit object
	
	for (int step = 0; step < 100000; step++) {
		if (current.count) { // if in leaf
			for (int i = current.leftFirst; i < current.leftFirst + current.count; i++) {
				TopBVHNodeScene node = m_top_leaves[m_top_indices[i]];
				r.o -= node.pos;
				intersect_bot(r, node.obj_index);
				r.o += node.pos;
			}
			last = current;
			if (current.parent == -1) return;
			current = m_top_bvh_nodes[current.parent];
			continue;
		}


		float dist_left = abs(m_top_bvh_nodes[current.leftFirst].minx - r.o.x) + 
			abs(m_top_bvh_nodes[current.leftFirst].miny - r.o.y) + 
			abs(m_top_bvh_nodes[current.leftFirst].minz - r.o.z);
		float dist_right = abs(m_top_bvh_nodes[current.leftFirst + 1].minx - r.o.x) +
			abs(m_top_bvh_nodes[current.leftFirst + 1].miny - r.o.y) +
			abs(m_top_bvh_nodes[current.leftFirst + 1].minz - r.o.z);
		if (dist_left < dist_right) {
			near_node = m_top_bvh_nodes[current.leftFirst];
			far_node = m_top_bvh_nodes[current.leftFirst + 1];
		}
		else {
			near_node = m_top_bvh_nodes[current.leftFirst + 1];
			far_node = m_top_bvh_nodes[current.leftFirst];
		}


		if (same_node(last, far_node)) { //just went up
			last = current;
			if (current.parent == -1) return;
			current = m_top_bvh_nodes[current.parent];
			continue;
		}

		// either last node is near or parent

		BVHNode try_child;
		if (current.parent == -1) {
			try_child = (!same_node(last,near_node)) ? near_node : far_node;
		} else {
			try_child = (same_node(last, m_top_bvh_nodes[current.parent])) ? near_node : far_node;
		}

		intersection_test_result = r.intersects_aabb(try_child);
		if (intersection_test_result.x < intersection_test_result.y) { // if intersection is found
			last = current;
			current = try_child;
		} else { //either move to far or up
			if (same_node(try_child, near_node)) { // move to far
				last = near_node;
			} else { // move up
				last = current;
				if (current.parent == -1) return;
				current = m_top_bvh_nodes[current.parent];
			}
		}
	}
	throw exception("too low steps constant during bvh traversal");
}

void Scene::intersect_bot(Ray& r, int obj_index) const { //assumes ray intersects
	//https://www.sci.utah.edu/~wald/Publications/2011/StackFree/sccg2011.pdf
	const uint model_start = m_model_bvh_starts[obj_index];

	BVHNode last;
	BVHNode current = m_bvh_nodes[model_start];
	BVHNode near_node;
	BVHNode far_node;
	//float2 intersection_test_result = r.intersects_aabb(current->bounds);
	//if (intersection_test_result.x >= intersection_test_result.y || intersection_test_result.x > r.t) return; // now we know that the root is intersected and partly closer than the furthest already hit object

	for (int step = 0; step < 100000; step++) {
		if (current.count) { // if in leaf
			for (int i = current.leftFirst; i < current.leftFirst + current.count; i++) {
				const uint prim_start = m_model_primitives_starts[obj_index];
				uint triangle_index = prim_start + m_indices[prim_start + i];
				const Triangle& t = m_triangles[triangle_index];
				intersect_triangle(t, r, triangle_index);
			}
			last = current;
			if (current.parent == -1) return;
			current = m_bvh_nodes[model_start + current.parent];
			continue;
		}


		float dist_left = abs(m_bvh_nodes[model_start + current.leftFirst].minx - r.o.x) +
			abs(m_bvh_nodes[model_start + current.leftFirst].miny - r.o.y) +
			abs(m_bvh_nodes[model_start + current.leftFirst].minz - r.o.z);
		float dist_right = abs(m_bvh_nodes[model_start + current.leftFirst + 1].minx - r.o.x) +
			abs(m_bvh_nodes[model_start + current.leftFirst + 1].miny - r.o.y) +
			abs(m_bvh_nodes[model_start + current.leftFirst + 1].minz - r.o.z);
		if (dist_left < dist_right) {
			near_node = m_bvh_nodes[model_start + current.leftFirst];
			far_node = m_bvh_nodes[model_start + current.leftFirst + 1];
		}
		else {
			near_node = m_bvh_nodes[model_start + current.leftFirst + 1];
			far_node = m_bvh_nodes[model_start + current.leftFirst];
		}


		if (same_node(last, far_node)) { //just went up
			last = current;
			if (current.parent == -1) return;
			current = m_bvh_nodes[model_start + current.parent];
			continue;
		}

		// either last node is near or parent

		BVHNode try_child;
		if (current.parent == -1) {
			try_child = (!same_node(last, near_node)) ? near_node : far_node;
		}
		else {
			try_child = (same_node(last, m_bvh_nodes[model_start + current.parent])) ? near_node : far_node;
		}

		float2 intersection_test_result = r.intersects_aabb(try_child);
		if (intersection_test_result.x < intersection_test_result.y) { // if intersection is found
			last = current;
			current = try_child;
		}
		else { //either move to far or up
			if (same_node(try_child, near_node)) { // move to far
				last = near_node;
			}
			else { // move up
				last = current;
				if (current.parent == -1) return;
				current = m_bvh_nodes[model_start + current.parent];
			}
		}
	}
	throw exception("too low steps constant during bvh traversal");
}

void Scene::intersect_triangle(const Triangle& tri, Ray& ray, uint triangle_index) const {
	//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	const float EPSILON = 0.0000001;
	float3 edge1 = tri.p1 - tri.p0;
	float3 edge2 = tri.p2 - tri.p0;
	float3 h = cross(ray.d, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) return;    // This ray is parallel to this triangle.
	float f = 1.f / a;
	float3 s = ray.o - tri.p0;
	float u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) return;
	float3 q = cross(s, edge1);
	float v = f * dot(ray.d, q);
	if (v < 0.0 || u + v > 1.0) return;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * dot(edge2, q);
	if (t > 0.f && ray.t > t) {
		ray.t = t;
		ray.hitptr = triangle_index;
	}
}