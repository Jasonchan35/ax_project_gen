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
		InitSetting(link_libs);
		InitSetting(link_files);
		InitSetting(link_flags);
		InitSetting(disable_warning);
	#undef InitSetting

	include_dirs.isPath  = true;
	include_files.isPath = true;
	link_dirs.isPath     = true;
	link_libs.isPath	 = false;
	link_files.isPath    = true;

	cpp_std = "c++14";

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
		lib_target_prefix = "lib";
		lib_target_suffix = ".a";
	}
}

void Config::init(Project* proj, Config* source, StrView name_) {
	_project = proj;
	name = name_;
	if (name == "Debug") {
		isDebug = true;
	}

	if (proj) {
		_build_tmp_dir.init(String(g_ws->buildDir, "_build_tmp/", name, '/', proj->name), false, true);
	}

	if (source) {
		cpp_std				= source->cpp_std;
		cpp_enable_modules	= source->cpp_enable_modules;
		warning_as_error	= source->warning_as_error;
		warning_level		= source->warning_level;
		xcode_settings		= source->xcode_settings;
		vs2015_ClCompile	= source->vs2015_ClCompile;
	} else {
		_init_xcode_settings();
		_init_vs2015_settings();
	}
}

void Config::_init_xcode_settings() {
	if (g_ws->os == "ios") {
		xcode_settings.add("SDKROOT"                   )->set("iphoneos");
		xcode_settings.add("SUPPORTED_PLATFORMS"       )->set("iphonesimulator iphoneos");
	//	xcode_settings.add("VALID_ARCHS"               )->set("arm64 armv7 armv7s");
		xcode_settings.add("IPHONEOS_DEPLOYMENT_TARGET")->set("10.1");
	}else if (g_ws->os == "macosx"){
		xcode_settings.add("SDKROOT"                   )->set("macosx");
		xcode_settings.add("SUPPORTED_PLATFORMS"       )->set("macosx");
		xcode_settings.add("MACOSX_DEPLOYMENT_TARGET"  )->set("10.10"); // c++11 require 10.10+
	}
	
//-----------
	if (isDebug) {
		xcode_settings.add("DEBUG_INFORMATION_FORMAT"      )->set("dwarf");
		xcode_settings.add("GCC_GENERATE_DEBUGGING_SYMBOLS")->set("YES");

		// 0: None[-O0], 1: Fast[-O1],  2: Faster[-O2], 3: Fastest[-O3], s: Fastest, Smallest[-Os], Fastest, Aggressive Optimizations [-Ofast]
		xcode_settings.add("GCC_OPTIMIZATION_LEVEL"        )->set("0");
		xcode_settings.add("ONLY_ACTIVE_ARCH"              )->set("YES");
		xcode_settings.add("ENABLE_TESTABILITY"            )->set("YES");
		
	}else{
		xcode_settings.add("DEBUG_INFORMATION_FORMAT"      )->set("dwarf-with-dsym");
		xcode_settings.add("GCC_GENERATE_DEBUGGING_SYMBOLS")->set("NO");
		
		// 0: None[-O0], 1: Fast[-O1],  2: Faster[-O2], 3: Fastest[-O3], s: Fastest, Smallest[-Os], Fastest, Aggressive Optimizations [-Ofast]
		xcode_settings.add("GCC_OPTIMIZATION_LEVEL"        )->set("s");

		xcode_settings.add("ONLY_ACTIVE_ARCH"              )->set("NO");
		xcode_settings.add("ENABLE_TESTABILITY"            )->set("YES");
		xcode_settings.add("LLVM_LTO"		               )->set("YES"); //link time optimization
		xcode_settings.add("DEAD_CODE_STRIPPING"           )->set("YES");
		xcode_settings.add("STRIP_STYLE"                   )->set("all");
	}
	
//-----------
	xcode_settings.add("CODE_SIGN_IDENTITY"				)->set("-");
	xcode_settings.add("ALWAYS_SEARCH_USER_PATHS"		)->set("NO");
	xcode_settings.add("CLANG_ENABLE_OBJC_ARC"			)->set("YES");
	xcode_settings.add("GCC_SYMBOLS_PRIVATE_EXTERN"		)->set("YES");
	xcode_settings.add("ENABLE_STRICT_OBJC_MSGSEND"		)->set("YES");

	// clang warning flags
	xcode_settings.add("CLANG_ANALYZER_LOCALIZABILITY_NONLOCALIZED"            )->set("YES");
	xcode_settings.add("CLANG_WARN_BOOL_CONVERSION"                            )->set("YES");
	xcode_settings.add("CLANG_WARN_CONSTANT_CONVERSION"                        )->set("YES");
	xcode_settings.add("CLANG_WARN_EMPTY_BODY"                                 )->set("YES");
	xcode_settings.add("CLANG_WARN_ENUM_CONVERSION"                            )->set("YES");
	xcode_settings.add("CLANG_WARN_INFINITE_RECURSION"                         )->set("YES");
	xcode_settings.add("CLANG_WARN_INT_CONVERSION"                             )->set("YES");
	xcode_settings.add("CLANG_WARN_SUSPICIOUS_MOVE"                            )->set("YES");
	xcode_settings.add("CLANG_WARN_UNREACHABLE_CODE"                           )->set("YES");
	xcode_settings.add("CLANG_WARN__DUPLICATE_METHOD_MATCH"                    )->set("YES");
	xcode_settings.add("CLANG_WARN_IMPLICIT_SIGN_CONVERSION"                   )->set("YES");
	xcode_settings.add("CLANG_WARN_ASSIGN_ENUM"                                )->set("YES");
	xcode_settings.add("CLANG_WARN_SUSPICIOUS_IMPLICIT_CONVERSION"             )->set("YES");
	xcode_settings.add("CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING"                )->set("YES");
	xcode_settings.add("CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF"                  )->set("YES");
	xcode_settings.add("CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS"            )->set("YES");
	xcode_settings.add("CLANG_WARN_RANGE_LOOP_ANALYSIS"                        )->set("YES");
	xcode_settings.add("CLANG_WARN_STRICT_PROTOTYPES"                          )->set("YES");
	xcode_settings.add("CLANG_WARN_COMMA"                                      )->set("YES");
	
	// gcc warning flags
	xcode_settings.add("GCC_WARN_FOUR_CHARACTER_CONSTANTS"                     )->set("YES");
	xcode_settings.add("GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED"              )->set("YES");
	xcode_settings.add("GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS"             )->set("YES");
	xcode_settings.add("GCC_WARN_SIGN_COMPARE"                                 )->set("YES");
	xcode_settings.add("GCC_TREAT_INCOMPATIBLE_POINTER_TYPE_WARNINGS_AS_ERRORS")->set("YES");
	xcode_settings.add("GCC_TREAT_IMPLICIT_FUNCTION_DECLARATIONS_AS_ERRORS"    )->set("YES");
	xcode_settings.add("GCC_WARN_UNUSED_LABEL"                                 )->set("YES");
//	xcode_settings.add("GCC_WARN_ABOUT_MISSING_PROTOTYPES"                     )->set("YES");

	xcode_settings.add("GCC_WARN_64_TO_32_BIT_CONVERSION"                      )->set("YES");
	xcode_settings.add("GCC_NO_COMMON_BLOCKS"                                  )->set("YES");
	xcode_settings.add("GCC_WARN_ABOUT_RETURN_TYPE"                            )->set("YES");
	xcode_settings.add("GCC_WARN_UNDECLARED_SELECTOR"                          )->set("YES");
	xcode_settings.add("GCC_WARN_UNINITIALIZED_AUTOS"                          )->set("YES");
	xcode_settings.add("GCC_WARN_UNUSED_FUNCTION"                              )->set("YES");
	xcode_settings.add("GCC_WARN_UNUSED_VARIABLE"                              )->set("YES");
}

void Config::_init_vs2015_settings() {
	vs2015_ClCompile.add("MinimalRebuild")->set("false");	
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
	ax_dump(s, outputLib);

	if (cpp_std)			ax_dump(s, cpp_std);
	if (cpp_enable_modules)	ax_dump(s, cpp_enable_modules);
	if (warning_as_error)	ax_dump(s, warning_as_error);
	if (xcode_settings)		ax_dump(s, xcode_settings);
	if (vs2015_ClCompile)	ax_dump(s, vs2015_ClCompile);
	if (vs2015_Link)		ax_dump(s, vs2015_Link);

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

	auto& t = rhs.outputLib.path();
	if (t && g_ws->generator != "android") {
		link_files._inherit.add(t, g_ws->buildDir);
	}

	xcode_settings.uniqueExtend(rhs.xcode_settings);
	vs2015_ClCompile.uniqueExtend(rhs.vs2015_ClCompile);
	vs2015_Link.uniqueExtend(rhs.vs2015_Link);
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

	if (!_project) return;

	cpp_defines._final.add(String("AX_GEN_CPU_",				g_ws->cpu));
	cpp_defines._final.add(String("AX_GEN_OS_",					g_ws->os));
	cpp_defines._final.add(String("AX_GEN_GENERATOR_",			g_ws->generator));
	cpp_defines._final.add(String("AX_GEN_COMPILER_",			g_ws->compiler));
	cpp_defines._final.add(String("AX_GEN_CONFIG_",				this->name));
	cpp_defines._final.add(String("AX_GEN_PLATFORM_NAME=\"",	g_ws->_platformName,"\""));

	if (_project) {
		auto& proj = *_project;
		cpp_defines._final.add(String("AX_GEN_PROJECT_",		proj.name));
		cpp_defines._final.add(String("AX_GEN_TYPE_",			proj.input.type));

		String tmp;

		if (proj.type_is_exe()) {
			tmp.set(g_ws->buildDir, "bin/", name, "/", out_target_dir, exe_target_prefix, proj.name, exe_target_suffix);

		}else if (proj.type_is_dll()) {
			tmp.set(g_ws->buildDir, "bin/", name, "/", out_target_dir, dll_target_prefix, proj.name, dll_target_suffix);
			if (g_ws->os == "windows") {
				outputLib.init(String(g_ws->buildDir, "lib/", name, "/", lib_target_prefix, proj.name, lib_target_suffix), false, false);
			} else {
				outputLib.init(String(g_ws->buildDir, "bin/", name, "/", out_target_dir, dll_target_prefix, proj.name, dll_target_suffix), false, false);
			}
		}else if (proj.type_is_lib()) {
			tmp.set(g_ws->buildDir, "lib/", name, "/", out_target_dir, lib_target_prefix, proj.name, lib_target_suffix);
			outputLib.init(tmp, false, false);
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
		ReadMember(cpp_enable_modules);
		ReadMember(cpp_std);
		ReadMember(warning_as_error);
		ReadMember(warning_level);
		ReadMember(out_target_dir);
		ReadMember(exe_target_prefix);
		ReadMember(exe_target_suffix);
		ReadMember(lib_target_prefix);
		ReadMember(lib_target_suffix);
		ReadMember(dll_target_prefix);
		ReadMember(dll_target_suffix);
		#undef ReadMember

		#define ReadDictMember(Value)		\
			if (r.member(#Value)) {			\
				r.beginObject();			\
				while (!r.endObject()) {	\
					String key;				\
					r.getMemberName(key);	\
					auto* v	= xcode_settings.getOrAdd(key);	\
					r.getValue(*v);			\
				}	\
				continue;	\
			} \
		//---------
			ReadDictMember(xcode_settings);
			ReadDictMember(vs2015_ClCompile);
			ReadDictMember(vs2015_Link);
		#undef ReadDictMember

		//----------
		String memberName;
		if (r.peekMemberName(memberName)) {
			if (auto v = memberName.removePrefix("config==")) {
				if (v != name) { 
					r.skipValue();
				}else{
					r.skipMemberName();
					readJson(r);
				}
				continue;
			}

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
