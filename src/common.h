#pragma once

#include "Platform/Platform.h"

#define _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <string>
#include <memory>
#include <exception>
#include <assert.h>
#include <stdint.h>
#include <thread>
#include <algorithm>

//#define ax_fallthrough [[fallthrough]]
#define ax_fallthrough //nothing

#if AX_OS_WINDOWS
	#include <Windows.h>
	#include <Shlwapi.h> // wnsprintf
	#pragma comment(lib, "Shlwapi.lib")

#else
	#include <dirent.h>
	#include <sys/stat.h>
 	#include <unistd.h>	
	#include <stdlib.h>
	#include <stdint.h>
	#include <spawn.h>
	#include <poll.h>
#endif

namespace ax_gen {

const int ax_dump_padding = 30;

using std::unique_ptr;

class NonCopyable {
public:
	NonCopyable() {}
	NonCopyable		(const NonCopyable&) = delete;
	void operator=	(const NonCopyable&) = delete;
};

class StringStream : public std::stringstream {	
	using Base = std::stringstream;
public:
	inline void append() {}

	template<typename First, typename... ARGS> inline
	void append(const First& first, const ARGS&... args) {
		*this << first;
		append(args...);
	}
};

enum class ConditionResult {
	None,
	True,
	False,
};

} //namespace

//---------
#include "Util/String.h"
#include "Util/Vector.h"
#include "Util/Dict.h"
#include "Util/FileUtil.h"
#include "Util/JsonReader.h"
#include "Util/JsonWriter.h"
#include "Util/XmlWriter.h"
#include "Util/System.h"
#include "Util/Path.h"
#include "Util/Glob.h"

