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

//#define ax_fallthrough [[fallthrough]]
#define ax_fallthrough //nothing

#if ax_OS_Windows
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

} //namespace

//---------

#include "Util/Vector.h"
#include "Util/Dict.h"
#include "Util/FileUtil.h"
#include "Util/JsonReader.h"
#include "Util/JsonWriter.h"
#include "Util/XmlWriter.h"
#include "Util/System.h"

