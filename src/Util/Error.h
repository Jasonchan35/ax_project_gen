#pragma once

#include "../common.h"
#include "Log.h"

namespace ax_gen {

class Error : public std::exception {
public:
	template<typename... ARGS>
	Error(const ARGS&... args) {
		Log::error(args...);
	}
};

} //namespace