#pragma once

#include "String.h"
#include "Vector.h"

namespace ax_gen {

struct Glob {

	static void search(Vector<String>& out_paths, const StrView& path, bool needDir = true, bool needFile = true, bool needHidden = false);

private:
	Glob() = delete;
};

} //namespace
