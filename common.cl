

struct Ray{
	float4 o; //origin
	float4 d; //direction
	float4 invDir;
	float t; //distance to closest intersection
	int pixel_id;
	float4 E;
	float4 T;
	long long hitptr; //a pointer to the hit object
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