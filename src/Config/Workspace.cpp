#include "../common.h"
#include "Workspace.h"

namespace ax_gen {

Workspace* g_ws;

Workspace::Workspace() {
	g_ws = this;

//======== CPU ========
#if ax_CPU_x86
	host_cpu = "x86";

#elif ax_CPU_x86_64
	host_cpu = "x86_64";

#elif ax_CPU_ARM
	host_cpu = "arm";

#else
	#error "Unknown CPU"
#endif

//======== OS ==========
#if ax_OS_Windows
	host_os = "windows";
	generator = "vs2015";

#elif ax_OS_Linux
	host_os = "linux";
	generator = "makefile";

#elif ax_OS_MacOSX
	host_os = "macosx";
	generator = "xcode";

#elif ax_OS_MinGW
	host_os = "mingw";
	generator = "makefile";

#elif ax_OS_Cygwin
	host_os = "cygwin";
	generator = "makefile";

#elif ax_OS_FreeBSD
	host_os = "freebsd";
	generator = "makefile";

#else
	#error "Unknown OS"
#endif
}

void Workspace::dump(StringStream& s) {
	s << "\n======== Workspace [" << workspace_name << "] ===============\n";
	ax_dump(s, input.build_dir);
	ax_dump(s, input.config_list);
	ax_dump(s, generator);
	ax_dump(s, compiler);
	ax_dump(s, os);
	ax_dump(s, cpu);
	ax_dump(s, _platformName);

	if (input.unite_build) {
		ax_dump(s, input.unite_build);
		ax_dump(s, input.unite_filesize);
	}

	ax_dump(s, input.startup_project);

	for (auto& c : configs) {
		c.dump(s);
	}

	for (auto& p : projects) {
		p.dump(s);
	}
}

void Workspace::readFile(const StrView& filename) {
	workspace_name = Path::basename(filename, false);

	Path::getAbs(axworkspaceFilename, filename);
	axworkspaceDir.set(Path::dirname(axworkspaceFilename), '/');


	_platformName.clear();

	if (generator == "android") {
		_platformName.append(workspace_name, '-', generator);
	} else {
		_platformName.append(workspace_name, '-', generator, '-', compiler, '-', os, '-', cpu);
	}

	String json;
	FileUtil::readTextFile(filename, json);

	{
		JsonReader r(json, axworkspaceFilename);
		readBuildDir(r);
	}

	{// set build_dir and log file
		String tmp;
		tmp.append(axworkspaceDir, input.build_dir, '/', _platformName);
		Path::getAbs(buildDir, tmp);
		buildDir += '/';

		String logFilename;
		logFilename.append(buildDir, "_ax_gen_log.txt");
		Log::createLogFile(logFilename);
	}

	{
		JsonReader r(json, axworkspaceFilename);
		readJson(r);
	}
}

void Workspace::readProjectFile(const StrView& filename) {
	auto projName = Path::basename(filename, false);
	auto* proj = projects.add(projName);
	proj->init(projName);
	proj->readFile(filename);
}

void Workspace::readBuildDir(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		if (r.member("build_dir",	input.build_dir )) return;
		r.skipValue();
	}

	input.build_dir = "_build"; //using default
}

void Workspace::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		#define ReadMember(T) if (r.member(#T, input.T)) continue;
			ReadMember(build_dir)
			ReadMember(startup_project)
			ReadMember(multithread_build)
			ReadMember(unite_build)
			ReadMember(unite_filesize)
			ReadMember(cuda_vs2015_props)
			ReadMember(cuda_vs2015_targets)
			ReadMember(visualc_PlatformToolset)
		#undef ReadMember

		if (r.member("config_list", input.config_list)) {
			for (auto& config_name : input.config_list) {
				auto* c = configs.add(config_name);
				c->init(nullptr, nullptr, config_name);
			}
			continue;
		}

		if (r.member("projects")) {
			r.beginArray();

			auto dir = Path::dirname(r.filename());
			while (!r.endArray()) {
				String filename;
				r.getValue(filename);

				String fullpath;
				Path::makeFullPath(fullpath, dir, filename);

				auto ext = Path::extension(fullpath);
				if (ext == "axproj") {
					readProjectFile(fullpath);
				}else{
					r.error("Unknown file type ", fullpath);
				}
			}
			continue;
		}

		if (r.member("extra_workspaces")) {
			r.beginObject();
			while (!r.endObject()) {
				String name;
				r.getMemberName(name);

				if (!name) {
					r.error("extra_workspace name cannot be empty");
				}

				if (name.equals(workspace_name, true)) {
					r.error("extra_workspace name cannot be as same as master workspace name");
				}

				auto* ew = extraWorkspaces.add(name);
				ew->name = name;
				ew->readJson(r);
			}
			continue;
		}

		if (r.member("config")) {
			if (!configs.size()) {
				r.error("please specify config_list before config");
			}

			for (auto& c : configs) {
				auto tr = r.clone();
				c.readJson(tr);
			}
			r.skipValue();
			continue;
		}

		//----- condition check -----
		String memberName;
		if (r.peekMemberName(memberName)) {
			switch (checkCondition(memberName)) {
				case ConditionResult::None: break;
				case ConditionResult::True: {
					r.skipMemberName();
					readJson(r);
					continue;
				}
				case ConditionResult::False: {
					r.skipValue();
					continue;
				}
			}
		}
	}
}

void Workspace::resolve() {	
	if (input.startup_project) {
		startupProject = projects.find(input.startup_project);
	}

	for (auto& c : configs) {
		c.computeFinal();
	}

	for (auto& p : projects) {
		projectGroups.add(p);
		p.resolve();
	}

	//-------
	masterWorkspace.name = g_ws->workspace_name;
	masterWorkspace.projects.reserve(g_ws->projects.size());
	for (auto& p : g_ws->projects) {
		masterWorkspace.projects.append(&p);
	}

	for (auto& ew : extraWorkspaces) {
		ew.resolve();
	}
}


ConditionResult Workspace::checkCondition(StrView expr) {
	if (auto v = expr.removePrefix("os==")) {
		return (v == os) ? ConditionResult::True : ConditionResult::False;
	}

	if (auto v = expr.removePrefix("compiler==")) {
		return (v == compiler) ? ConditionResult::True : ConditionResult::False;
	}

	if (auto v = expr.removePrefix("cpu==")) {
		return (v == cpu) ? ConditionResult::True : ConditionResult::False;
	}

	if (auto v = expr.removePrefix("generator==")) {
		return (v == generator) ? ConditionResult::True : ConditionResult::False;
	}

	return ConditionResult::None;
}

bool Workspace::os_has_objc() {
	if (os=="macosx")	return true;
	if (os=="ios")		return true;
	return false;
}


void ExtraWorkspace::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		r.member("projects", input.projects);
		r.member("groups",   input.groups);
		r.member("exclude_projects", input.exclude_projects);
		r.member("exclude_groups",   input.exclude_groups);
	}
}

void ExtraWorkspace::resolve() {
	Vector<Project*> projectToAdd;
	Vector<Project*> projectToRemove;

	for (auto& name : input.projects) {
		for (auto& p : g_ws->projects) {
			if (p.name.matchWildcard(name, true)) {
				projectToAdd.uniqueAppend(&p);
			}
		}
	}

	for (auto& name : input.groups) {
		for (auto& g : g_ws->projectGroups.dict.values()) {
			if (g.path.matchWildcard(name, true)) {
				for (auto& gp : g.projects) {
					projectToAdd.uniqueAppend(gp);
				}
			}
		}
	}


	for (auto& name : input.exclude_projects) {
		for (auto& p : g_ws->projects) {
			if (p.name.matchWildcard(name, true)) {
				projectToRemove.uniqueAppend(&p);
			}
		}
	}

	for (auto& name : input.exclude_groups) {
		for (auto& g : g_ws->projectGroups.dict.values()) {
			if (g.path.matchWildcard(name, true)) {
				for (auto& gp : g.projects) {
					projectToRemove.uniqueAppend(gp);
				}
			}
		}
	}

	for (auto& p : projectToAdd) {
		if (projects.indexOf(p) >= 0) continue;
		if (projectToRemove.indexOf(p) >= 0) continue;
		projects.append(p);
	}
}

} //namespace
