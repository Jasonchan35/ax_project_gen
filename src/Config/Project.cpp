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
	return *configs.find(g_ws->defaultConfigName());
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
	ax_dump(s, category);
	ax_dump(s, type);
	
	if (gui_app) ax_dump(s, gui_app);
	ax_dump(s, dependencies);
	ax_dump(s, pch_header);
	if (unite_build)   ax_dump(s, unite_build);
	ax_dump(s, unite_mega_byte_per_file);
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

	for (auto& c : configs) {
		c.dump(s);
	}
}

void Project::init(const StrView& name_) {
	name = name_;
	
	//carry workspace global setting as project default
	input.unite_build = g_ws->unite_build;
	input.unite_mega_byte_per_file = g_ws->unite_mega_byte_per_file;
	
	for (auto& src : g_ws->configs) {
		auto* dst = configs.add(src.name);
		dst->_project = this;
		dst->name = src.name;
		dst->_build_tmp_dir.set(g_ws->outDir, "_build_tmp/", src.name, '/', name, '/');
	}
}
	
void Project::readFile(const StrView& filename) {
	Path::getAbs(buildFilename, filename);
	buildFileDir = Path::dirname(buildFilename);
	buildFileDir += '/';

	String json;
	FileUtil::readTextFile(filename, json);

	JsonReader r(json, filename);
	readJson(r);
}

void Project::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		if (r.member("category",		input.category		)) continue;
		if (r.member("type",			input.type			)) continue;
		if (r.member("gui_app",			input.gui_app		)) continue;

		if (r.member("dependencies",	input.dependencies	)) continue;
		if (r.member("files",			input.files			)) continue;
		if (r.member("exclude_files",	input.exclude_files	)) continue;
		if (r.member("pch_header",		input.pch_header	)) continue;

		if (r.member("unite_build",		input.unite_build		)) continue;
		if (r.member("unite_mega_byte_per_file",	input.unite_mega_byte_per_file	)) continue;

		if (r.member("xcode_bundle_identifier",		input.xcode_bundle_identifier	)) continue;
		
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

		String memberName;
		r.getMemberName(memberName);
		r.error("Unknown member ", memberName);
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

	multithreadBuild = input.multithreadBuild;

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

	if (input.pch_header) {
		Path::makeFullPath(pch_header, buildFileDir, input.pch_header);
	}
}

void Project::resolve_files() {
	_generatedFileDir.clear();
	_generatedFileDir.append(g_ws->outDir, "_generated_/", name, "/");

	fileEntries.clear();

	Vector<String> globResult;
	String tmp;

	for (auto& f : input.files) {
		Path::makeFullPath(tmp, buildFileDir, f);
		Glob::search(globResult, tmp, false);

		for (auto& r : globResult) {
			if (fileEntries.find(r)) continue;
			auto* e = fileEntries.add(r);
			e->init(r);
		}
	}

	for (auto& f : input.exclude_files) {
		Path::makeFullPath(tmp, buildFileDir, f);
		Glob::search(globResult, tmp, false);

		for (auto& r : globResult) {
			fileEntries.remove(r);
		}
	}

	if (input.unite_build) {
		resolve_genUniteFiles(FileType::cpp_source, ".cpp");
		resolve_genUniteFiles(FileType::c_source,   ".c");
	}

	// virtual folder
	for (auto& f : fileEntries) {
		if (f.generated) {
			virtualFolders.add(g_ws->outDir, f);
		}else{
			virtualFolders.add(buildFileDir, f);
		}
	}
}

void Project::resolve_genUniteFiles(FileType targetType, const StrView& ext) {
	String code;
	const int unite_byte_per_file = (int)(input.unite_mega_byte_per_file * 1024 * 1024);

	for (auto& f : fileEntries) {
		if (f.excludedFromBuild) continue;
		if (f.type() != targetType) continue;

		if (!code) {
			code.append("//-- Auto Generated File for Unite Build\n");
		}

		code.append("#include \"", f.name(), "\"\n");

		if (code.size() > unite_byte_per_file) {
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
	String filename;

	char index[60+1];
	snprintf(index, 60, "%03d", _uniteFileCount);
	index[60] = 0;

	filename.append(_generatedFileDir, name, "-UNITE_", StrView_c_str(index), ext);
	FileUtil::writeTextFile(filename, code, g_app->options.verbose);
	
	_uniteFileCount++;
	
	auto* e = fileEntries.add(filename);
	e->init(filename);
	e->generated = true;
}


} //namespace

