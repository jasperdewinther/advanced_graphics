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