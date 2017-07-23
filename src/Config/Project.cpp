#include "../common.h"
#include "../App.h"
#include "Project.h"
#include "Workspace.h"

namespace ax_gen {

Project::Project() {
	_resolved  = false;
	_resolving = false;
}

Config& Project::defaultConfig() {
	auto& name = g_ws->defaultConfigName();
	auto* c = configs.find(name);
	if (!c) throw Error("Cannot get default config ", name);
	return *c;
}

Config& Project::configToBuild() {
	auto& name = g_app->options.config;
	auto* c = configs.find(name);
	if (!c) throw Error("Cannot find config ", name, " for -build / -run");
	return *c;
}

bool Project::type_is_cpp() {
	if (type == ProjectType::cpp_lib) return true;
	if (type == ProjectType::cpp_dll) return true;
	if (type == ProjectType::cpp_exe) return true;
	return false;
}

bool Project::type_is_c() {
	if (type == ProjectType::c_lib) return true;
	if (type == ProjectType::c_dll) return true;
	if (type == ProjectType::c_exe) return true;
	return false;
}

bool Project::type_is_exe() {
	if (type == ProjectType::c_exe) return true;
	if (type == ProjectType::cpp_exe) return true;
	return false;
}

bool Project::type_is_dll() {
	if (type == ProjectType::c_dll) return true;
	if (type == ProjectType::cpp_dll) return true;
	return false;
}

bool Project::type_is_lib() {
	if (type == ProjectType::c_lib) return true;
	if (type == ProjectType::cpp_lib) return true;
	return false;
}

bool Project::type_is_headers() {
	if (type == ProjectType::c_headers) return true;
	if (type == ProjectType::cpp_headers) return true;
	return false;
}

void Project::Input::dump(StringStream& s) {
	ax_dump(s, group);
	ax_dump(s, type);
	
	if (gui_app) ax_dump(s, gui_app);
	ax_dump(s, dependencies);
	ax_dump(s, pch_header);
	if (unite_build)   ax_dump(s, unite_build);
	ax_dump(s, unite_filesize);
}

void Project::dump(StringStream& s) {
	s << "\n======== Project [" << name << "] ===============\n";
	input.dump(s);

	{
		String tmp;
		int i = 0;
		tmp = "[";
		for (auto& p : _dependencies_inherit) {
			if (i>0) tmp += ", ";
			tmp += p->name;
			i++;
		}
		tmp += "]";
		ax_dump_(s, "dependencies._inherit", tmp);
	}

	if (g_app->options.verbose) {
		s << "                         files = ";
		int i = 0;
		for (auto f : fileEntries) {
			if (i > 0) {
				s << "\n                                 ";
			}
			s << f.path();
			i++;
		}
		s << "\n";

		s << "                virtualFolders = ";
		i = 0;
		for (auto f : virtualFolders.dict) {
			if (i > 0) {
				s << "\n                                 ";
			}
			s << f.path;
			i++;
		}
		s << "\n";
	}

	for (auto& c : configs) {
		c.dump(s);
	}
}

void Project::init(const StrView& name_) {
	name = name_;
	
	//carry workspace global setting as project default
	input.unite_build = g_ws->input.unite_build;
	input.unite_filesize = g_ws->input.unite_filesize;

	input.multithread_build = g_ws->input.multithread_build;
	
	for (auto& src : g_ws->configs) {
		auto* dst = configs.add(src.name);
		dst->init(this, &src, src.name);
	}
}
	
void Project::readFile(const StrView& filename) {
	Path::getAbs(axprojFilename, filename);
	axprojDir.set(Path::dirname(axprojFilename), '/');

	String json;
	FileUtil::readTextFile(filename, json);

	JsonReader r(json, filename);
	readJson(r);
}

void Project::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		#define ReadMember(T) r.member(#T, input.T);
			ReadMember(group);
			ReadMember(type);
			ReadMember(enable_cuda);
			ReadMember(gui_app);
			ReadMember(pch_header);
			ReadMember(unite_build);
			ReadMember(unite_filesize);
			ReadMember(multithread_build);
			ReadMember(xcode_bundle_identifier);
			ReadMember(visualc_PlatformToolset);
		#undef ReadMember

		if (r.member("dependencies")) {
			r.appendValue(input.dependencies);
		}

		if (r.member("files")) {
			Vector<String> arr;
			r.getValue(arr);

			Vector<String> globResult;
			String tmp;
			for (auto& a : arr) {
				bool isAbs = Path::isAbs(a);

				Path::makeFullPath(tmp, axprojDir, a);
				Glob::search(globResult, tmp, false);

				for (auto& q : globResult) {				
					if (isAbs) {
						tmp = q;
					}else{
						Path::getRel(tmp, q, axprojDir);
					}

					fileEntries.add(tmp, axprojDir, false);
				}
			}
		}

		if (r.member("exclude_files")) {
			Vector<String> arr;
			r.getValue(arr);

			Vector<String> globResult;
			String tmp;
			for (auto& f : arr) {
				Path::makeFullPath(tmp, axprojDir, f);
				Glob::search(globResult, tmp, false);

				for (auto& key : globResult) {
					fileEntries.remove(key);
				}
			}
		}

		if (r.member("config")) {
			if (!configs.size()) {
				r.error("please specify config_list before config");
			}

			for (auto& c : configs) {
				auto cr = r.clone();
				c.readJson(cr);
			}
			r.skipValue();
			continue;
		}

		//----- condition check -----
		String memberName;
		if (r.peekMemberName(memberName)) {
			switch (g_ws->checkCondition(memberName)) {
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

void Project::resolve() {
	if (_resolved) return;
	_resolved = true;
	
	if (_resolving) {
		throw Error("Cycle dependencies in project ", name);
	}
	_resolving = true;
	resolve_internal();
	_resolving = false;
}

void Project::resolve_internal() {
	resolve_files();

	if (     input.type == "cpp_exe"    ) { hasOutputTarget = true; type = ProjectType::cpp_exe; }
	else if (input.type == "c_exe"      ) { hasOutputTarget = true; type = ProjectType::c_exe; }
	//------
	else if (input.type == "cpp_dll"    ) { hasOutputTarget = true; type = ProjectType::cpp_dll; }
	else if (input.type == "c_dll"      ) { hasOutputTarget = true; type = ProjectType::c_dll; }
	//------
	else if (input.type == "cpp_lib"    ) { hasOutputTarget = true; type = ProjectType::cpp_lib; }
	else if (input.type == "c_lib"      ) { hasOutputTarget = true; type = ProjectType::c_lib; }
	//------
	else if (input.type == "c_headers"  ) { type = ProjectType::c_headers; }
	else if (input.type == "cpp_headers") { type = ProjectType::cpp_headers; }
	//------
	else{
		throw Error("Unknown project type ", input.type, " from project ", name);
	}

	{
		int i = 0;
		for (auto& c : configs) {
			c.inherit(g_ws->configs[i]);
			i++;
		}
	}

	for (auto& d : input.dependencies) {
		auto* dp = g_ws->projects.find(d);
		if (!dp) {
			throw Error("Cannot find dependency project '", d, "' for project '", name, "'");
		}
		if (dp == this) {
			throw Error("project depends on itself, project='", name, "'");
		}

		_dependencies.uniqueAppend(dp);
		dp->resolve();

		{
			int i = 0;
			for (auto& c : configs) {
				c.inherit(dp->configs[i]);
				i++;
			}
		}

		for (auto& dpdp : dp->_dependencies_inherit) {
			_dependencies_inherit.uniqueAppend(dpdp);
		}
		_dependencies_inherit.uniqueAppend(dp);
	}

	//config.final
	{
		int i = 0;
		for (auto& c : configs) {
			c.computeFinal();
			i++;
		}
	}
}

void Project::resolve_files() {
	_generatedFileDir.set(g_ws->buildDir, "_generated_/", name, "/");

	if (input.pch_header) {
		pch_header = fileEntries.add(input.pch_header, axprojDir, false);
	}

	if (input.unite_build) {
		resolve_genUniteFiles(FileType::cpp_source, ".cpp");
		resolve_genUniteFiles(FileType::c_source,   ".c");
	}

	// virtual folder
	for (auto& f : fileEntries) {
		if (f.generated) {
			virtualFolders.add(g_ws->buildDir, f);
		}else{
			virtualFolders.add(axprojDir, f);
		}
	}
}

void Project::resolve_genUniteFiles(FileType targetType, const StrView& ext) {
	String code;
	String filename;

	for (auto& f : fileEntries) {
		if (f.excludedFromBuild) continue;
		if (f.type() != targetType) continue;

		if (!code) {
			code.append("//-- Auto Generated File for Unite Build\n");
		}

		Path::getRel(filename, f.absPath(), _generatedFileDir);

		code.append("#include \"", filename, "\"\n");

		if (code.size() > input.unite_filesize) {
			write_uniteFile(code, ext);
			code.clear();
		}

		f.excludedFromBuild = true;
	}

	if (code) {
		write_uniteFile(code, ext);
	}
}

void Project::write_uniteFile(const StrView& code, const StrView& ext) {

	char index[60+1];
	snprintf(index, 60, "%03d", _uniteFileCount);
	index[60] = 0;

	String filename(name, "-UNITE_", StrView_c_str(index), ext);
	String fullpath(_generatedFileDir, filename);

	FileUtil::writeTextFile(fullpath, code, g_app->options.verbose);
	
	_uniteFileCount++;
	
	fileEntries.add(filename, _generatedFileDir, true);
}

} //namespace

