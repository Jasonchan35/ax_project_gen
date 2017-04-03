#pragma once

#include "../common.h"

namespace ax_pjgen {

class App;
class Project;
class Workspace;
class FileEntry;
class Config;

class Generator : public NonCopyable {
public:
	Generator();
	virtual void generate() = 0;

	virtual void build	() { Log::warning("this generator doesn't support command [build]"); }
	virtual void run	() { Log::warning("this generator doesn't support command [run]"); }
	virtual void ide	() { Log::warning("this generator doesn't support command [ide]"); }
};

} //namespace
