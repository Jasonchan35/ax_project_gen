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

	class Input {
	public:
		Vector<String>		config_list;
		String				build_dir;
		String				startup_project;
		bool				unite_build {true};
		double				unite_mega_byte_per_file {1};
	};
	Input	input;

	String				workspace_name;
	StringDict<Config>	configs;

	String&				defaultConfigName() { return input.config_list.back(); }

	String				generator;

	String				host_os;
	String				host_cpu;

	String				os;
	String				compiler;
	String				cpu;
	String				_platformName;

	Project*			_startup_project{nullptr};

	StringDict<Project>	projects;
	ProjectGroupDict	projectGroups;

	String				exe_target_suffix;
	String				dll_target_suffix;
	String				lib_target_suffix;

	String				axworkspaceFilename;
	String				axworkspaceDir;
	String				outDir;
	
	void dump(StringStream& s);
	void resolve();
};

extern Workspace* g_ws;

} //namespace
