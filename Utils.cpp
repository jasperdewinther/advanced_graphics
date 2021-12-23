#include "precomp.h"
#include "Utils.h"


uint32_t xorshift32(struct xorshift_state* state)
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

vector<string> split(const string& s, char delim) {
    //https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    vector<string> result;
    stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

void run_multithreaded(int threads, int width, int height, bool reduce_hot_chunks, const std::function<void(int, int)>& f) {
	std::vector<std::thread> t = {};
	t.reserve(threads);
	if (reduce_hot_chunks) {
		float sqrt_t = (float)sqrt(threads);
		for (int i = 0; i < threads; i++)
			t.push_back(std::thread([i, &width, &f, &sqrt_t, &height]() {
				float x_start = i % (int)sqrt_t;
				for (float y = (float)i / sqrt_t; y < height; y += sqrt_t)
					for (float x = x_start; x < width; x += sqrt_t)
						f(x, y);
				}));
	} else {
		for (int i = 0; i < threads; i++) 
			t.push_back(std::thread([i, &threads, &height, &width, &f]() {
				for (int y = i; y < height; y += threads)
					for (int x = 0; x < width; x++)
						f(x, y);
		}));
	}

	for (auto& thread : t) {
		thread.join();
	}
}

float3 rotate(const float3& data, float angle) {
	return float3(data.x * cos(angle) + data.z * sin(angle), data.y, data.x * -sin(angle) + data.z * cos(angle));
}