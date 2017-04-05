#pragma once

#include "../common.h"

namespace ax_gen {

class App;
class Project;
class Workspace;
class FileEntry;
class Config;

class Generator : public NonCopyable {
public:
	Generator();
	virtual void generate() = 0;

	virtual void build	() { Log::warning("this generator doesn't support action [-build]"); }
	virtual void run	() { Log::warning("this generator doesn't support action [-run]"); }
	virtual void ide	() { Log::warning("this generator doesn't support action [-ide]"); }
};

} //namespace
