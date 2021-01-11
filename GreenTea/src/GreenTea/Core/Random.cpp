#include "Random.h"

#include <random>
#include <array>

static std::mt19937 s_Engine;
static std::uniform_int_distribution<std::mt19937::result_type> s_UniformDistrubution;

namespace GTE {

	void Random::Init()
	{
		std::random_device rd;
		std::array<int, std::mt19937::state_size> seed_data;
		std::generate_n(seed_data.data(), seed_data.size(), std::ref(rd));
		std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
		s_Engine = std::mt19937(seq);
	}

	float Random::Float(void)
	{
		return (float)s_UniformDistrubution(s_Engine) / (float)std::numeric_limits<uint32_t>::max();
	}

}