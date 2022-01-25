#include "common.cl"

enum Material {
	red = 0,
	mirror = 1,
	magenta = 2,
	cyan = 3,
	white = 4,
	glass = 5,
	red_glass = 6,
	reflective_blue = 7,
	white_light = 8,
	emerald = 9,
	bright_green = 10,
	yellow = 11,
	green_light = 12,
};

struct MaterialData{
	float3 color; //it either is a static color or gets calculated with the location and normal
	float specularity; //1.0 = 100% reflective
	float transparent; // 1 if not transparent at all, 0 if completely transparent, this has to do with beers law
	float refractive_index; 
	bool isLight;
};

__constant struct MaterialData materials[13] = {
	{{1,0,0}, 				0.0, 1.0, 0.0, false},
	{{1,1,1}, 				1.0, 1.0, 0.0, false},
	{{0.21,0.26,0}, 		0.0, 1.0, 0.0, false},
	{{0.1,1,1}, 			0.1, 1.0, 0.0, false},
	{{1,1,1}, 				0.0, 1.0, 0.0, false},
	{{1,1,1}, 				0.0, 0.0, 1.5, false}, //glass
	{{0.88,0.07,0.37}, 		0.0, 0.7, 1.2, false},
	{{0.1,0.6,0.9}, 		0.9, 1.0, 0.0, false},
	{{10,10,10}, 			0.0, 1.0, 0.0, true},
	{{0.31,0.78,0.47}, 		0.0, 0.3, 1.6, false},
	{{0.66, 0.86, 0.12}, 	0.0, 1.0, 0.0, false},
	{{0.88, 0.91, 0.13}, 	0.0, 1.0, 0.0, false},
	{{6.6, 8.6, 1.2}, 		0.0, 1.0, 0.0, true},
};


struct Triangle{
	float3 p0;
	float3 p1;
	float3 p2;
	float3 normal;
	enum Material m;
};

void intersect_primitive(const Triangle& tri, Ray& ray)
{
	//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	const float EPSILON = 0.0000001;
	float3 edge1 = tri.p1 - tri.p0;
	float3 edge2 = tri.p2 - tri.p0;
	float3 h = cross(ray.d, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) return;    // This ray is parallel to this triangle.
	float f = 1.0 / a;
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
		ray.hitptr = &tri;
	}
}

struct AABB{
	float minx;
	float miny;
	float minz;
	float maxx;
	float maxy;
	float maxz;
};

struct BVHNode {
	struct AABB bounds;
	int leftFirst;
	int count;
};


struct TopBVHNode {
	long long obj;
	float3 pos;
};

void intersect_primitive(const TopBVHNode& node, Ray& ray) {
	ray.o -= node.pos;
	node.obj->intersects(ray);
	ray.o += node.pos;
}



void intersects(Ray& r) const {
	instantiated_intersect(r);
}

template<typename T>
void BVH<T>::instantiated_intersect(Ray& r) const {
	BVHNode* n = &pool[0];
	// more from slides
	float2 result = r.intersects_aabb(n->bounds);
	if (result.x > result.y || result.x > r.t) return;
	intersect_internal(r);
}

template<typename T>
void BVH<T>::intersect_internal(Ray& r, int node_index) const { //assumes ray intersects
	BVHNode* n = &pool[node_index];
	if (n->count) {
		for (int i = n->leftFirst; i < n->leftFirst + n->count; i++) {
			intersect_primitive(primitives[indices[i]], r);
		}
	}
	else
	{
		float2 result_left = r.intersects_aabb(pool[n->leftFirst].bounds);
		float2 result_right = r.intersects_aabb(pool[n->leftFirst + 1].bounds);
		if (result_left.x < result_left.y && result_left.x < r.t) {
			if (result_right.x < result_right.y && result_right.x < r.t) {
				if (result_left.x < result_right.x) {
					intersect_internal(r, n->leftFirst);
					if (result_right.x < r.t) {
						intersect_internal(r, n->leftFirst + 1);
					}
				}
				else {
					intersect_internal(r, n->leftFirst + 1);
					if (result_left.x < r.t) {
						intersect_internal(r, n->leftFirst);
					}
				}
			}
			else {
				intersect_internal(r, n->leftFirst);
			}
		}
		else if (result_right.x <= result_right.y && result_right.x < r.t) intersect_internal(r, n->leftFirst + 1);
	}
}




__kernel void extend(
	__global struct Ray* ray_data,
	__global struct 
	
	) {
	uint i = get_global_id( 0 );
	Ray& r = ray_data[i];
	find_intersection(r);
}