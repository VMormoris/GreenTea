#include "Random.h"

#include <array>
#include <algorithm>
#include <random>

static std::mt19937 sEngine;
static std::uniform_int_distribution<std::mt19937::result_type> sUniformDistrubution;

namespace gte {

	void Random::Init()
	{
		std::random_device rd;
		std::array<int, std::mt19937::state_size> seed_data;
		std::generate_n(seed_data.data(), seed_data.size(), std::ref(rd));
		std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
		sEngine = std::mt19937(seq);
	}

	float Random::Float(void)
	{
		return (float)sUniformDistrubution(sEngine) / (float)std::numeric_limits<uint32>::max();
	}

}