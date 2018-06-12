#pragma once

#include "../common.h"
#include "../Config/Workspace.h"

namespace ax_gen {

class App;
class Project;
class Workspace;
class FileEntry;
class Config;

class Generator : public NonCopyable {
public:
	Generator();
	virtual ~Generator() {}

			void init		();
			void generate	();
			void ide		();
			void build		();
			void run		();


	virtual void onInit	() = 0;
	virtual void onGenerate	() = 0;
	virtual void onBuild	() { Log::warning("this generator doesn't support action [-build]"); }
	virtual void onIde		() { Log::warning("this generator doesn't support action [-ide]"); }
			void onRun		();
};

} //namespace
