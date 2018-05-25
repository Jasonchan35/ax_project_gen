#pragma once
#include "../common.h"
#include "Config.h"
#include "Project.h"
#include "../Generators/Generator.h"

namespace ax_gen {

class ExtraWorkspace {
public:
	void readJson(JsonReader& r);
	class Input {
	public:
		Vector<String>	projects;
		Vector<String>	groups;
		Vector<String>	exclude_projects;
		Vector<String>	exclude_groups;
	};
	Input	input;
	String	name;
	Vector<Project*> projects;

	struct GenData_vs2015 {
		String		sln;
	};
	GenData_vs2015 genData_vs2015;

	void resolve();
};

class Workspace : public NonCopyable {
public:
	Workspace();

	void readFile(const StrView& filename);
	void readJson(JsonReader& r);
	void readBuildDir(JsonReader& r);

	void readProjectFile(const StrView& filename);

	class Input {
	public:
		Vector<String>		config_list;
		String				build_dir;
		String				startup_project;
		bool				multithread_build = true;
		bool				unite_build = false;
		int					unite_filesize = 1 * 1024 * 1024;
		String				cuda_vs2015_props;
		String				cuda_vs2015_targets;
		String				visualc_PlatformToolset;
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

	Project*			startupProject{nullptr};

	StringDict<Project>	projects;
	ProjectGroupDict	projectGroups;

	String				axworkspaceFilename;
	String				axworkspaceDir;
	String				buildDir;
	
	ExtraWorkspace		masterWorkspace;
	StringDict<ExtraWorkspace> extraWorkspaces;

	ConditionResult checkCondition(StrView expr);

	bool os_has_objc();

	void dump(StringStream& s);
	void resolve();

	struct GenData_xcode {
		String		xcworkspace;
	};
	GenData_xcode genData_xcode;
};

extern Workspace* g_ws;

} //namespace
