#pragma once

#include "../common.h"
#include "Log.h"

namespace ax_gen {

class Error : public std::exception {
public:
	template<typename... ARGS>
	Error(ARGS&&... args) {
		Log::error(std::forward<ARGS>(args)...);
	}
};

} //namespace