#include "../common.h"
#include "Workspace.h"

namespace ax_pjgen {

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

#else
	#error "Unknown OS"
#endif

//====================
	os  = host_os;
	cpu = host_cpu;
}

void Workspace::dump(StringStream& s) {
	s << "\n======== Workspace [" << workspace_name << "] ===============\n";
	ax_dump(s, config_list);

	ax_dump(s, generator);
	ax_dump(s, compiler);
	ax_dump(s, os);
	ax_dump(s, cpu);
	ax_dump(s, _platformName);

	if (unite_build)	 ax_dump(s, unite_build);

	ax_dump(s, unite_mega_byte_per_file);

	ax_dump(s, startup_project);

	for (auto& c : configs) {
		c.dump(s);
	}

	for (auto& p : projects) {
		p.dump(s);
	}
}

void Workspace::readFile(const StrView& filename) {
	Path::getAbs(buildFilename, filename);
	buildFileDir = Path::dirname(buildFilename);
	buildFileDir += '/';

	workspace_name = Path::basename(filename, false);

	_platformName.clear();
	_platformName.append(generator, '-', compiler, '-', os, '-', cpu);

	{
		String tmp;
		tmp.append(buildFileDir, "_local_build/", _platformName);
		Path::getAbs(outDir, tmp);
		outDir += '/';
	}

	{
		String logFilename;
		logFilename.append(outDir, "_ax_build_log.txt");
		Log::createLogFile(logFilename);
	}

	String json;
	FileUtil::readTextFile(filename, json);

	JsonReader r(json, buildFilename);
	readJson(r);
}

void Workspace::readProjectFile(const StrView& filename) {
	auto projName = Path::basename(filename, false);
	auto* proj = projects.add(projName);
	proj->init(projName);
	proj->readFile(filename);
}

void Workspace::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		if (r.member("startup_project", startup_project)) continue;
		if (r.member("unite_build",		unite_build)) continue;
		if (r.member("unite_mega_byte_per_file", unite_mega_byte_per_file )) continue;

		if (r.member("config_list", config_list)) {
			for (auto& config_name : config_list) {
				auto* c = configs.add(config_name);
				c->name = config_name;
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

		String memberName;
		r.getMemberName(memberName);
		r.error("Unknown member ", memberName);
	}
}

void Workspace::resolve() {	
	if (startup_project) {
		_startup_project = projects.find(startup_project);
	}

	if (os == "windows") {
		exe_target_suffix = ".exe";
		dll_target_suffix = ".dll";
	}else{
		exe_target_suffix = "";
		dll_target_suffix = ".so";
	}

	if (compiler == "msvc") {
		lib_target_suffix = ".lib";
	}else{
		lib_target_suffix = ".a";
	}

	for (auto& c : configs) {
		c.computeFinal();
	}

	for (auto& p : projects) {
		projectCategories.add(p);
		p.resolve();
	}
}


} //namespace
