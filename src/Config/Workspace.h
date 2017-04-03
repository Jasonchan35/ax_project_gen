#pragma once
#include "../common.h"
#include "Config.h"
#include "Project.h"
#include "../Generators/Generator.h"

namespace ax_gen {

class Workspace : public NonCopyable {
public:
	Workspace();

	void readFile(const StrView& filename);
	void readJson(JsonReader& r);

	void readProjectFile(const StrView& filename);

	Vector<String>		config_list;
	StringDict<Config>	configs;
	String				workspace_name;

	String&				defaultConfigName() { return config_list.back(); }

	bool				unite_build {true};
	double				unite_mega_byte_per_file {1};

	String				generator;

	String				host_os;
	String				host_cpu;

	String				os;
	String				compiler;
	String				cpu;
	String				_platformName;

	String				startup_project;
	Project*			_startup_project{nullptr};

	StringDict<Project>	projects;
	ProjectCategoryDict projectCategories;

	String				exe_target_suffix;
	String				dll_target_suffix;
	String				lib_target_suffix;

	String				buildFilename;
	String				buildFileDir;
	String				outDir;
	
	void dump(StringStream& s);
	void resolve();
};

extern Workspace* g_ws;

} //namespace
