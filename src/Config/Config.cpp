#include "../common.h"
#include "../App.h"
#include "FileEntry.h"
#include "Config.h"
#include "Project.h"
#include "Workspace.h"

namespace ax_gen {

Config::Config() {
	#define InitSetting(V) \
		V.name = #V; \
		_settings.append(&V); \
	//------
		InitSetting(cpp_defines);
		InitSetting(cpp_flags);
		InitSetting(include_dirs);
		InitSetting(include_files);
		InitSetting(link_dirs);
		InitSetting(link_files);
		InitSetting(link_flags);
		InitSetting(disable_warning);
	#undef InitSetting

	include_dirs.isPath  = true;
	include_files.isPath = true;
	link_dirs.isPath     = true;
	link_files.isPath    = true;

	if (g_ws->os == "windows") {
		exe_target_suffix = ".exe";
		dll_target_suffix = ".dll";
	}else{
		exe_target_suffix = "";
		dll_target_suffix = ".so";
	}

	if (g_ws->compiler == "vc") {
		lib_target_suffix = ".lib";
	}else{
		lib_target_suffix = ".a";
	}
}

void Config::dump(StringStream& s) {
	if (!g_app->options.verbose) return;

	s << "\n-- Config [";
	if (_project) {
		s << _project->name;
	}else{
		s << "<Workspace>";
	}
	s << "." << name << "] --\n";

	ax_dump(s, outputTarget);
	if (warning_as_error) ax_dump(s, warning_as_error);

	for (auto& p : _settings) {
		p->dump(s);
	}
}


bool Config::_readSettings(JsonReader& r) {
	for (auto& p : _settings) {
		if (p->readEntry(r)) return true;
	}
	return false;
}

void Config::inherit(const Config& rhs) {
	resolve();

	int i = 0;
	for (auto& p : _settings) {
		p->inherit(*rhs._settings[i]);
		i++;
	}

	auto& t = rhs.outputTarget.path();
	if (t) {
		link_files._inherit.add(t, g_ws->buildDir);
	}
}

void Config::computeFinal() {
	resolve();

	int i = 0;
	for (auto& p : _settings) {
		p->computeFinal();
		i++;
	}
}

void Config::resolve() {
	if (_resolved) return;
	_resolved = true;

	if (name == "Debug") isDebug = true;
	if (!_project) return;

	cpp_defines._final.add(String("ax_GEN_CPU_",				g_ws->cpu));
	cpp_defines._final.add(String("ax_GEN_OS_",					g_ws->os));
	cpp_defines._final.add(String("ax_GEN_GENERATOR_",			g_ws->generator));
	cpp_defines._final.add(String("ax_GEN_COMPILER_",			g_ws->compiler));
	cpp_defines._final.add(String("ax_GEN_CONFIG_",				this->name));
	cpp_defines._final.add(String("ax_GEN_PLATFORM_NAME=\"",	g_ws->_platformName,"\""));

	if (_project) {
		auto& proj = *_project;
		cpp_defines._final.add(String("ax_GEN_PROJECT_",		proj.name));
		cpp_defines._final.add(String("ax_GEN_TYPE_",			proj.input.type));

		String tmp;

		if (proj.type_is_exe()) {
			tmp.set(g_ws->buildDir, "bin/", name, "/", exe_target_prefix, proj.name, exe_target_suffix);
		}else if (proj.type_is_dll()) {
			tmp.set(g_ws->buildDir, "bin/", name, "/", dll_target_prefix, proj.name, dll_target_suffix);
		}else if (proj.type_is_lib()) {
			tmp.set(g_ws->buildDir, "lib/", name, "/", lib_target_prefix, proj.name, lib_target_suffix);
		}

		if (tmp) {
			outputTarget.init(tmp, false, false);
		}
	}
}

void Config::readJson(JsonReader& r) {
	r.beginObject();
	while (!r.endObject()) {
		if (_readSettings(r)) continue;

		//---------
		#define ReadMember(Value) if (r.member(#Value, Value)) continue;
		ReadMember(warning_as_error);
		ReadMember(warning_level);
		ReadMember(exe_target_prefix);
		ReadMember(exe_target_suffix);
		ReadMember(lib_target_prefix);
		ReadMember(lib_target_suffix);
		ReadMember(dll_target_prefix);
		ReadMember(dll_target_suffix);
		#undef ReadMember

		if (r.member("xcode_settings")) {
			r.beginObject();
			while (!r.endObject()) {
				String key, value;
				r.getMemberName(key);
				auto* v	= xcode_settings.add(key);
				r.getValue(*v);
			}
			continue;
		}

		//----------
		String memberName;
		if (r.peekMemberName(memberName)) {
			if (auto v = memberName.getFromPrefix("config==")) {
				if (v != name) { 
					r.skipValue();
				}else{
					r.skipMemberName();
					readJson(r);
				}
				continue;
			}

			if (auto v = memberName.getFromPrefix("os==")) {
				if (v != g_ws->os) { 
					r.skipValue();
				}else{				
					r.skipMemberName();
					readJson(r);
				}
				continue;
			}

			if (auto v = memberName.getFromPrefix("compiler==")) {
				if (v != g_ws->compiler) {
					r.skipValue();
				}else{
					r.skipMemberName();
					readJson(r);
				}
				continue;
			}

			if (auto v = memberName.getFromPrefix("generator==")) {
				if (v != g_ws->generator) { 
					r.skipValue();
				}else{
					r.skipMemberName();
					readJson(r);
				}
				continue;
			}
		}
	}
}

void Config::Setting::_readValue(JsonReader& r, EntryDict& v) {
	Vector<String> arr;
	r.getValue(arr);

	if (!isPath) {
		for (auto& q : arr) {
			v.add(q);
		}
	}else{

		String tmp;
		for (auto& q : arr) {
			if (Path::isAbs(q)) {
				v.add(q);

			}else{
				auto dir = Path::dirname(r.filename());
				v.add(q, dir);
			}
		}
	}
}

bool Config::Setting::readEntry(JsonReader& r) {
	if (r.member(name)) {
		_readValue(r, add);
		return true;
	}
	
	if(auto v = r.memberWithPrefix(name)) {
		if (v == ".remove") {
			_readValue(r, remove);
			return true;
		}
		
		if(v == ".local") {
			_readValue(r, localAdd);
			return true;
		}
		
		if(v == ".localRemove") {
			_readValue(r, localRemove);
			return true;
		}
	}

	return false;
}

void Config::Setting::inherit(Setting& rhs) {
	_inherit.extend(rhs._inherit);
}

void Config::Setting::computeFinal() {
	_inherit.extend(add);
//	_inherit.remove_if(rhs.remove);

	_final.extend(_inherit);
	_final.extend(localAdd);
	//localRemove
}

void Config::Setting::dump(StringStream& s) {
	dumpEntries(s, _inherit, "._inherit");
	dumpEntries(s, add, "");
	dumpEntries(s, remove, ".remove");
	dumpEntries(s, localAdd, ".local");
	dumpEntries(s, localRemove, ".localRemove");
	dumpEntries(s, _final, "._final");
}

void Config::Entry::init(const StrView& absPath, bool isAbs) {
	_isAbs = isAbs;
	_absPath = absPath;
	if (isAbs) {
		_path = absPath;
	}else{
		Path::getRel(_path, absPath, g_ws->buildDir);
	}
}

Config::Entry* Config::EntryDict::add(const StrView& value, const StrView& fromDir) {
	String key;
	bool isAbs = true;

	if (!fromDir) {
		key = value;
	}else{
		isAbs = Path::isAbs(value);
		if (isAbs) {
			key = value;
		}else{
			Path::makeFullPath(key, fromDir, value);
		}
	}

	auto* e = _dict.find(key);
	if (!e) {
		e = _dict.add(key);
	}
	e->init(key, isAbs);
	return e;
}

void Config::EntryDict::add(Entry& v) {
	auto& key = v.absPath();

	auto* e = _dict.find(key);
	if (!e) {
		e = _dict.add(key);
	}
	*e = v;
}

void Config::EntryDict::extend(EntryDict& rhs) {
	for (auto& q : rhs) {
		add(q);
	}
}

} //namespace
