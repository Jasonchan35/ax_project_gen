#pragma once

#include "String.h"

namespace ax_gen {

struct FileUtil {
	static const int kPathMax = 512;

	static void readTextFile (const StrView& filename, String& out_str);
	static void writeTextFile(const StrView& filename, const StrView& text, bool verbose = true);

private:
	FileUtil() = delete;
};

} //namespace