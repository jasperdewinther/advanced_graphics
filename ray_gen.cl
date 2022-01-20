
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

__kernel void ray_gen( 
    __global float* ray_data, 
    int width, 
    int height, 
    float upx, 
    float upy,
    float upz,
    float sidex, 
    float sidey,
    float sidez,  
    float screen_centerx, 
    float screen_centery,
    float screen_centerz,
    float aspect_ratio, 
    float cameraposx,
    float cameraposy,
    float cameraposz,
    int ray_size,
    int noise
    )
{
    uint x = get_global_id( 0 );
    uint y = get_global_id( 1 );

    struct xorshift_state rand = {.a=noise==0?12:noise}; //12 is an arbitrary number

	float px = ((float)x + (float)xorshift32(&rand) / (float)4294967295) / (float)width;
	float py = ((float)y + (float)xorshift32(&rand) / (float)4294967295) / (float)height;
		
    float dirx = screen_centerx + (sidex * ((px - 0.5f) * aspect_ratio)) + (upx * ((py * -1) + 0.5f));
    float diry = screen_centery + (sidey * ((px - 0.5f) * aspect_ratio)) + (upy * ((py * -1) + 0.5f));
    float dirz = screen_centerz + (sidez * ((px - 0.5f) * aspect_ratio)) + (upz * ((py * -1) + 0.5f));
    dirx = dirx - cameraposx;
    diry = diry - cameraposy;
    dirz = dirz - cameraposz;
	float3 dir = (float3)(dirx, diry, dirz);
	dir = normalize(dir);
    uint pixel_index = x + (width * y);
	ray_data[pixel_index * ray_size] = cameraposx;
    ray_data[pixel_index * ray_size + 1] = cameraposy;
    ray_data[pixel_index * ray_size + 2] = cameraposz;
    ray_data[pixel_index * ray_size + 4] = dir.x;
    ray_data[pixel_index * ray_size + 5] = dir.y;
    ray_data[pixel_index * ray_size + 6] = dir.z;
    ray_data[pixel_index * ray_size + 8] = 1.f/dir.x;
    ray_data[pixel_index * ray_size + 9] = 1.f/dir.y;
    ray_data[pixel_index * ray_size + 10] = 1.f/dir.z;
    ray_data[pixel_index * ray_size + 11] = 0;
    ray_data[pixel_index * ray_size + 12] = 999999.f; //distance to intersection
    ray_data[pixel_index * ray_size + 13] = *((float*)&(pixel_index)); //pixel id
    ray_data[pixel_index * ray_size + 14] = 0;
    ray_data[pixel_index * ray_size + 15] = 0; // E
    ray_data[pixel_index * ray_size + 16] = 0;
    ray_data[pixel_index * ray_size + 17] = 0;
    ray_data[pixel_index * ray_size + 20] = 1; // T
    ray_data[pixel_index * ray_size + 21] = 1;
    ray_data[pixel_index * ray_size + 22] = 1;
    ray_data[pixel_index * ray_size + 24] = 0; // hitptr
}

