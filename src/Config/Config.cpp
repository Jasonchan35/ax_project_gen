#include "../common.h"
#include "../App.h"
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
		if (p->readItem(r)) return true;
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
}

void Config::computeFinal() {
	resolve();

	int i = 0;
	for (auto& p : _settings) {
		p->computeFinal();
		i++;
	}

	if (outputTarget) {
		link_files._inherit.uniqueAppend(outputTarget);
	}
}

void Config::resolve() {
	if (_resolved) return;
	_resolved = true;

	outputTarget.clear();

	if (name == "Debug") isDebug = true;
	if (!_project) return;

	cpp_defines._final.uniqueAppend(String("ax_GEN_CPU_",				g_ws->cpu));
	cpp_defines._final.uniqueAppend(String("ax_GEN_OS_",				g_ws->os));
	cpp_defines._final.uniqueAppend(String("ax_GEN_GENERATOR_",			g_ws->generator));
	cpp_defines._final.uniqueAppend(String("ax_GEN_COMPILER_",			g_ws->compiler));
	cpp_defines._final.uniqueAppend(String("ax_GEN_CONFIG_",			this->name));
	cpp_defines._final.uniqueAppend(String("ax_GEN_PLATFORM_NAME=\"",	g_ws->_platformName,"\""));

	if (_project) {
		auto& proj = *_project;
		cpp_defines._final.uniqueAppend(String("ax_GEN_PROJECT_",		proj.name));
		cpp_defines._final.uniqueAppend(String("ax_GEN_TYPE_",			proj.input.type));

		if (proj.type_is_exe()) {
			outputTarget.append(g_ws->outDir, "bin/", name, "/", proj.name, g_ws->exe_target_suffix);
		}else if (proj.type_is_dll()) {
			outputTarget.append(g_ws->outDir, "bin/", name, "/", proj.name, g_ws->dll_target_suffix);
		}else if (proj.type_is_lib()) {
			outputTarget.append(g_ws->outDir, "lib/", name, "/", proj.name, g_ws->lib_target_suffix);
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
		String memberNameStr;
		r.getMemberName(memberNameStr);

		auto memberName = StrView(memberNameStr);

		if (auto v = memberName.getFromPrefix("config==")) {
			if (v != name) { 
				r.skipValue();
			}else{
				readJson(r);
			}
			continue;
		}

		if (auto v = memberName.getFromPrefix("os==")) {
			if (v != g_ws->os) { 
				r.skipValue();
			}else{				
				readJson(r);
			}
			continue;
		}

		if (auto v = memberName.getFromPrefix("compiler==")) {
			if (v != g_ws->compiler) {
				r.skipValue();
			}else{
				readJson(r);
			}
			continue;
		}

		if (auto v = memberName.getFromPrefix("generator==")) {
			if (v != g_ws->generator) { 
				r.skipValue();
			}else{
				readJson(r);
			}
			continue;
		}
	}
}

void Config::Setting::_readValue(JsonReader& r, Vector<String>& v) {
	r.getValue(v);
	if (isPath) {		
		String tmp;
		for (auto& it : v) {
			auto dir = Path::dirname(r.filename());
			Path::makeFullPath(tmp, dir, it);
			it = tmp;
		}
	}
}

bool Config::Setting::readItem(JsonReader& r) {
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

void Config::Setting::inherit(const Setting& rhs) {
	_inherit.uniqueExtend(rhs._inherit);
}

void Config::Setting::computeFinal() {
	_inherit.uniqueExtend(add);
//	_inherit.remove_if(rhs.remove);

	_final.uniqueExtend(_inherit);
	_final.uniqueExtend(localAdd);
	//localRemove
}

void Config::Setting::dump(StringStream& s) {
	dumpItem(s, _inherit, "._inherit");
	dumpItem(s, add, "");
	dumpItem(s, remove, ".remove");
	dumpItem(s, localAdd, ".local");
	dumpItem(s, localRemove, ".localRemove");
	dumpItem(s, _final, "._final");
}

} //namespace
