__constant float epsilon = 0.0001;

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
	float4 color; //it either is a static color or gets calculated with the location and normal
	float specularity; //1.0 = 100% reflective
	float transparent; // 1 if not transparent at all, 0 if completely transparent, this has to do with beers law
	float refractive_index; 
	bool isLight;
};

__constant struct MaterialData materials[13] = {
	{{1,0,0,0}, 			0.0, 1.0, 0.0, false},
	{{1,1,1,0}, 			1.0, 1.0, 0.0, false},
	{{0.21,0.26,0,0}, 		0.0, 1.0, 0.0, false},
	{{0.1,1,1,0}, 			0.1, 1.0, 0.0, false},
	{{1,1,1,0}, 			0.0, 1.0, 0.0, false},
	{{1,1,1,0}, 			0.0, 0.0, 1.5, false}, //glass
	{{0.88,0.07,0.37,0}, 	0.0, 0.7, 1.2, false},
	{{0.1,0.6,0.9,0}, 		0.9, 1.0, 0.0, false},
	{{10,10,10,0}, 			0.0, 1.0, 0.0, true},
	{{0.31,0.78,0.47,0}, 	0.0, 0.3, 1.6, false},
	{{0.66, 0.86, 0.12,0}, 	0.0, 1.0, 0.0, false},
	{{0.88, 0.91, 0.13,0}, 	0.0, 1.0, 0.0, false},
	{{6.6, 8.6, 1.2,0}, 	0.0, 1.0, 0.0, true},
};


struct Triangle{
	float4 p0;
	float4 p1;
	float4 p2;
	float4 normal;
	enum Material m;
};


struct BVHNode {
	float minx;
	float miny;
	float minz;
	float maxx;
	float maxy;
	float maxz;
	int leftFirst;
	int count;
	int parent;
};

bool same_node(struct BVHNode n1, struct BVHNode n2) {
	return n1.parent == n2.parent && n1.leftFirst == n2.leftFirst && n1.count == n2.count;
}


struct TopBVHNode {
	float4 pos;
	uint obj;
};

struct Ray{
	float4 o; //origin
	float4 d; //direction
	float4 invDir;
	float t; //distance to closest intersection
	int pixel_id;
	float4 E;
	float4 T;
	uint hitptr; //a pointer to the hit object
};

struct xorshift_state {
	uint a;
};

uint xorshift32(struct xorshift_state* state)
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

float XorRandomFloat(struct xorshift_state* s){
	return xorshift32(s) * 2.3283064365387e-10f;
}

