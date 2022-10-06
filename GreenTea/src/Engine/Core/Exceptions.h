#pragma once
#include "Engine.h"
#include <exception>

namespace gte::internal {

	class ENGINE_API Exception : public std::runtime_error {
	public:
		const char* what(void) const throw() { return "Runtime Exception"; }
	};

}