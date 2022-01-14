#pragma once
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>

struct xorshift_state {
	uint32_t a;
};
uint32_t xorshift32(struct xorshift_state* state);

std::vector<string> split(const std::string& s, char delim);

void run_multithreaded(int threads, int width, int height, bool reduce_hot_chunks, const std::function<void(int, int)>& f);
float3 rotate(const float3& data, float angle);