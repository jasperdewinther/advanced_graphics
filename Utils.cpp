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

void run_multithreaded(int threads, int width, int height, const std::function<void(int, int)>& f) {
	std::vector<std::thread> t = {};
	t.reserve(threads);

	for (int i = 0; i < threads; i++) 
		t.push_back(std::thread([i, &threads, &height, &width, &f]() {
			for (int y = 0; y < height; y++)
				for (int x = i; x < width; x+= threads)
					f(x, y);
	}));

	for (auto& thread : t) {
		thread.join();
	}
}

float3 rotate(const float3& data, float angle) {
	return float3(data.x * cos(angle) + data.z * sin(angle), data.y, data.x * -sin(angle) + data.z * cos(angle));
}