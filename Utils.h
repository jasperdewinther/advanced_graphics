#pragma once
#include "Materials.h"
#include <cstdint>
#include <vector>
#include <sstream>

struct xorshift_state {
	uint32_t a;
};
uint32_t xorshift32(struct xorshift_state* state);

std::vector<string> split(const std::string& s, char delim);

enum Primitive {
	sphere = 0,
	plane = 1,
	triangle = 2,
	nothing = 3
};


void run_multithreaded(int threads, int width, int height, bool reduce_hot_chunks, const std::function<void(int, int)>& f);
float3 rotate(const float3& data, float angle);