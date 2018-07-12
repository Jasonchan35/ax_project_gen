#include "Generator_makefile.h"

namespace ax_gen {

void Generator_makefile::onInit() {
	if (!g_ws->compiler) g_ws->compiler = "gcc";
}

void Generator_makefile::onBuild() {
#if ax_OS_Windows
#else
	int ret = ::system(String("make -C \"", g_ws->buildDir, "\" config=\"", g_app->options.config,"\"").c_str());
	if (ret != 0) {
		throw Error("Error Build");
	}
#endif
}

void Generator_makefile::onGenerate() {
	gen_workspace();
}

void Generator_makefile::gen_helper_batch_file() {
	String filename(g_ws->buildDir, "mkdir.bat");
	String o("@ECHO OFF\n IF exist \"%1\" (echo directory \"%1\" exists) ELSE ( mkdir \"%1\" && echo directory \"%1\" created)");
	FileUtil::writeTextFile(filename, o);
}

void Generator_makefile::gen_workspace() {
	Log::info("gen_workspace ", g_ws->workspace_name);

	gen_helper_batch_file();

	String o;
	o.append("# Generated by ax_gen");

	gen_common_var(o);

	o.append(".PHONY:");
	o.append("\\\n\t", "clean");
	o.append("\\\n\t", "build");
	o.append("\\\n\t", "run");

	for (auto& proj : g_ws->projects) {
		gen_project(proj);
		o.append("\\\n\t", proj.name);
	}
	o.append("\n\n");

	//----- build: ----------
	o.append("build:" );
	for (auto& proj : g_ws->projects) {
		o.append("\\\n\t", escapeString(proj.name));
	}
	o.append("\n\n");

	o.append("all: build\n");
	o.append("\n");

	o.append("build:\n" );
	o.append("\t@echo \"--- Build Finish ---\"\n");
	o.append("\n");

	//------ run: -------
	o.append("run: build \n");
	o.append("\t@echo \"==============================================================\"\n");
	if (!g_ws->startupProject) {
		o.append("\t@echo \"no startup_project specified\"\n");
	}else{
		o.append("\t$(MAKE) -f \"", g_ws->startupProject->genData_makefile.makefile, "\" run $(MFLAGS)\n");
	}
	o.append("\n");

	//------ clean: -------
	o.append("clean: \n");
	o.append("\t@echo \"==============================================================\"\n");
	for (auto& proj : g_ws->projects) {
		o.append("\t$(MAKE) -f \"", proj.genData_makefile.makefile, "\" clean $(MFLAGS)\n");
	}
	o.append("\t@echo \"===== clean finish ===== \"\n");
	o.append("\n");

	//---- project target ------
	for (auto& proj : g_ws->projects) {
		o.append(escapeString(proj.name), ":");
		for (auto& dp : proj._dependencies_inherit) {
			o.append(" ", escapeString(dp->name));
		}		
		o.append("\n");

		if (!proj.hasOutputTarget) continue;

		String mt_build;
		if (proj.multithread_build()) {
			mt_build.append(" -j ");
		}

		o.append("\t@echo \"==============================================================\"\n");
		o.append("\t@echo \"[build project] ", proj.name, "\"\n");
		o.append("\t$(MAKE)", mt_build," -f \"", proj.genData_makefile.makefile, "\" build $(MFLAGS)\n");
		o.append("\n");
	}

	String filename(g_ws->buildDir, "GNUmakefile");
	FileUtil::writeTextFile(filename, o);

	{
		String bsd_makefile_txt;
		bsd_makefile_txt.append(".PHONY: all clean build run:\n");
		bsd_makefile_txt.append("all clean build run:\n");
		bsd_makefile_txt.append("\tgmake $@ $(MFLAGS)\n");
		FileUtil::writeTextFile(String(g_ws->buildDir, "Makefile"), bsd_makefile_txt);
	}
}

void Generator_makefile::gen_common_var(String& o) {
	o.append("#!! Unix Makefile		\n");
	o.append("#!! Works for FreeBSD make or Linux gmake	\n");
	o.append("#!! \n");
	o.append("config ?= ", g_ws->defaultConfigName(), "\n");
	o.append("\n");
	o.append("\n");

	if (g_ws->host_os == "windows") {
		o.append("cmd_mkdir := cmd.exe /c mkdir.bat","\n");
		o.append("cmd_rmdir := rm -rf",   "\n");
		o.append("cmd_rm    := rm -f",    "\n");
		o.append("cmd_copy  := cp -f",    "\n");
	}else{
		o.append("cmd_mkdir := mkdir -p", "\n");
		o.append("cmd_rmdir := rm -rf",   "\n");
		o.append("cmd_rm    := rm -f",    "\n");
		o.append("cmd_copy  := cp -f",    "\n");
	}

	o.append("\n");
}

String Generator_makefile::escapeString(const StrView& v) {
	String o;
	for (auto ch : v) {
		switch (ch) {
			case ':': ax_fallthrough
			case ' ': {
				o.append('\\', ch);
			}break;
			default:	o.append(ch);		break;
		}
	}
	return o;
}

String Generator_makefile::quotePath(const StrView& v) {
	String o;

	o.append('\"');
	if (!Path::isAbs(v)) {
		o.append("$(pwd)/");
	}

	for (auto ch : v) {
		switch (ch) {
			case '\"':	o.append("\\\"");	break;
			default:	o.append(ch);		break;
		}
	}
	o.append('\"');
	return o;
}

void Generator_makefile::gen_project(Project& proj) {
	if (!proj.hasOutputTarget) return;

	proj.genData_makefile.makefile.set(proj.name, ".make");
	Log::info("gen_project ", proj.genData_makefile.makefile);

	String o;
	gen_common_var(o);
	{
		pch_suffix = ".gch";
		auto cpp_std = proj.defaultConfig().cpp_std;

		if (g_ws->compiler == "clang") {
			pch_suffix = ".pch";
			o.append("cmd_cpp   := clang++ -std=", cpp_std, "\n");
			o.append("cmd_c     := clang",  "\n");
			o.append("cmd_link  := clang++","\n");
		}else if (g_ws->compiler == "gcc") {
			o.append("cmd_cpp   := g++ -std=", cpp_std, "\n");
			o.append("cmd_c     := gcc",  "\n");
			o.append("cmd_link  := g++",  "\n");
		}else{
			throw Error("Unsupported compiler ", g_ws->compiler);
		}

		o.append("cmd_ar    := ar rcs \n");
	}

	if (g_ws->os_has_objc() && proj.input.cpp_as_objcpp) {
		o.append("pch_header_compiler_language = objective-c++-header\n");
		o.append("cpp_source_compiler_language = objective-c++\n");
		o.append("c_source_compiler_language   = objective-c\n");
	}else{
		o.append("pch_header_compiler_language = c++-header\n");
		o.append("cpp_source_compiler_language = c++\n");
		o.append("c_source_compiler_language   = c\n");
	}
	o.append("\n");
	o.append("pwd=$$(pwd)\n");
	o.append("\n");
	o.append(".PHONY:");
	o.append("\\\n\t", "clean");
	o.append("\\\n\t", "build");
	o.append("\\\n\t", "run");
	for (auto& config : proj.configs) {
		o.append("\\\n\t", config.name, "__build");
		o.append("\\\n\t", config.name, "__clean");
		o.append("\\\n\t", config.name, "__run");
	}
	o.append("\n\n");
	o.append("all: build\n\n");

	for (auto& config : proj.configs) {
		gen_project_config(o, config);
	}

	o.append("#-----------\n");
	o.append("BUILD_TMP_DIR      = $($(config)__BUILD_TMP_DIR)\n");
	o.append("PCH_HEADER_SRC     = $($(config)__PCH_HEADER_SRC)\n");
	o.append("PCH_HEADER_PCH     = $($(config)__PCH_HEADER_PCH)\n");
	o.append("PCH_HEADER_DEP     = $($(config)__PCH_HEADER_DEP)\n");
	o.append("PCH_CC_FLAGS       = $($(config)__PCH_CC_FLAGS)\n");
	o.append("\n");
	o.append("CPP_INCLUDE_DIRS   = $($(config)__CPP_INCLUDE_DIRS)\n");
	o.append("CPP_INCLUDE_FILES  = $($(config)__CPP_INCLUDE_FILES)\n");
	o.append("CPP_FLAGS          = $($(config)__CPP_FLAGS)\n");
	o.append("CPP_DEFINES        = $($(config)__CPP_DEFINES)\n");
	o.append("C_FLAGS            = $($(config)__C_FLAGS)\n");
	o.append("C_DEFINES          = $($(config)__C_DEFINES)\n");
	o.append("LINK_FLAGS         = $($(config)__LINK_FLAGS)\n");
	o.append("LINK_LIB           = $($(config)__LINK_LIBS)\n");
	o.append("LINK_FILES         = $($(config)__LINK_FILES)\n");
	o.append("CPP_OBJ_FILES      = $($(config)__CPP_OBJ_FILES)\n");
	o.append("\n");
	o.append("build: $(config)__build\n");
	o.append("clean: $(config)__clean\n");
	o.append("run:   $(config)__run\n");
	o.append("\n");

	String filename(g_ws->buildDir, proj.genData_makefile.makefile);
	FileUtil::writeTextFile(filename, o);
}

String Generator_makefile::get_obj_file(Config& config, FileEntry& f) {
	return String(config.genData_makefile.cpp_obj_dir, Path::basename(f.path(), true), "_obj");
}

void Generator_makefile::gen_project_config(String& o, Config& config) {
	Project& proj = *config._project;

	o.append("\n\n");
	o.append("#===== ", config.name, " ======================\n");
	o.append("#!!!\n");
	o.append("#!!! Make cannot handle file path contain space in variable\n");
	o.append("#!!! therefore we try to unroll all file dependencies directly in makefile instead of using makefile variable\n");
	o.append("#!!!\n");

	config.genData_makefile.cpp_obj_dir.set(config._build_tmp_dir.path(), "/cpp_objs/");

	o.append(config.name, "__build: ", escapeString(config.outputTarget.path()), "\n\n");

	o.append(config.name, "__clean: \n");
	o.append("\t$(cmd_rmdir) \"", config._build_tmp_dir.path(), "\"\n");
	if (config.outputTarget.path()) {
		o.append("\t$(cmd_rm) \"", config.outputTarget.path(), "\"\n");
	}
	o.append("\n");

	String pch_header_dep;
	String pch_header_pch;
	String pch_header_pch_dir;
	String pch_basename;
	String pch_cc_flags;

	//------- pre-compiled header
	if (proj.pch_header) {
		pch_basename = Path::basename(proj.pch_header->absPath(), true);

		pch_header_pch.set(config._build_tmp_dir.path(), "/cpp_pch/", pch_basename, pch_suffix);
		pch_header_dep.set(pch_header_pch, ".d");

		pch_header_pch_dir = Path::dirname(pch_header_pch);
		pch_cc_flags.append("\\\n\t", "-I", quotePath(pch_header_pch_dir));
		pch_cc_flags.append("\\\n\t", "-include ", quotePath(proj.pch_header->path()));

		//makefile optional include cpp_dep		
		o.append("-include ", escapeString(pch_header_dep), "\n");
		//---------
		o.append("#--- pch_header dependencies ------\n");
		o.append(escapeString(pch_header_pch), ": ", escapeString(proj.pch_header->path()), "\n");
		o.append("\t@echo \"-------------------------------------------------------------\"\n");
		o.append("\t@echo \"[precompiled header] $< => $@\"\n");
		o.append("\t$(cmd_mkdir) ", quotePath(pch_header_pch_dir), "\n");
		o.append("\t$(cmd_cpp) -x $(pch_header_compiler_language) $(CPP_DEFINES) $(CPP_FLAGS) $(CPP_INCLUDE_DIRS) \\\n");
		o.append("\t\t-o \"$@\" -c ", quotePath(proj.pch_header->path()), " \\\n");
		o.append("\t\t-MMD -MQ \"$@\" -MF ", quotePath(pch_header_dep), " \\\n");
		o.append("\n");

		o.append("\n\n");
	}

	o.append("\n");
	o.append("#-------------------\n");

	String cpp_defines;
	String cpp_flags;
	String link_flags;
	String link_libs;
	String link_files;
	String include_files;
	String include_dirs;
	String cpp_obj_files;

	for (auto& q : config.cpp_defines._final) {
		cpp_defines.append("\\\n\t-D", q.path());
	}
	for (auto& q : config.cpp_flags._final) {
		cpp_flags.append("\\\n\t", q.path());
	}
	for (auto& q : config.link_flags._final) {
		link_flags.append("\\\n\t", q.path());
	}
	for (auto& q : config.link_dirs._final) {
		link_libs.append("\\\n\t-L", quotePath(q.path()));
	}
	for (auto& q : config.link_libs._final) {
		link_libs.append("\\\n\t-l", q.path());
	}
	for (auto& q : config.link_files._final) {
		link_files.append("\\\n\t", quotePath(q.path()));
	}
	for (auto& q : config.include_files._final) {
		include_files.append("\\\n\t-include ", quotePath(q.path()));
	}
	for (auto& q : config.include_dirs._final) {
		include_dirs.append("\\\n\t-I", quotePath(q.path()));
	}

	for (auto q : proj.fileEntries) {
		if (q.excludedFromBuild) continue;
		cpp_obj_files.append("\\\n\t", escapeString(get_obj_file(config, q)));
	}

	o.append(config.name, "__BUILD_TMP_DIR  = ", escapeString(config._build_tmp_dir.path()));
	if (proj.pch_header) {
		o.append(config.name, "__PCH_HEADER        = ", escapeString(proj.pch_header->path()), "\n");
		o.append(config.name, "__PCH_HEADER_PCH    = ", escapeString(pch_header_pch), "\n");
		o.append(config.name, "__PCH_HEADER_DEP    = ", escapeString(pch_header_dep), "\n");
	}

	o.append(config.name, "__PCH_CC_FLAGS      = ", pch_cc_flags,  "\n");
	o.append(config.name, "__CPP_INCLUDE_DIRS  = ", include_dirs,  "\n");
	o.append(config.name, "__CPP_INCLUDE_FILES = ", include_files, "\n");
	o.append(config.name, "__CPP_FLAGS         = ", cpp_flags,     "\n");
	o.append(config.name, "__CPP_DEFINES       = ", cpp_defines,   "\n");		
	o.append(config.name, "__LINK_FLAGS        = ", link_flags,    "\n");
	o.append(config.name, "__LINK_LIBS         = ", link_libs,     "\n");
	o.append(config.name, "__LINK_FILES        = ", link_files,    "\n");
	o.append(config.name, "__CPP_OBJ_FILES     = ", cpp_obj_files, "\n");

	o.append("\n#--- ", config.name, " cpp_obj dependencies ------\n");

	for (auto& f : proj.fileEntries) {
		if (f.excludedFromBuild) continue;

		String cpp_obj = get_obj_file(config, f);
		String cpp_dep(cpp_obj, ".d");
		String cpp_src(f.path());
	
		if (f.type_is_c()) {
			//makefile optional include cpp_dep
			o.append("-include ", escapeString(cpp_dep), "\n");

			o.append(escapeString(cpp_obj), ":", escapeString(cpp_src), "\n");
			o.append("\t@echo \"-------------------------------------------------------------\"\n");
			o.append("\t@echo \"[compile c] => $@\"\n");
			o.append("\t$(cmd_mkdir) ", quotePath(Path::dirname(cpp_obj)), "\n");
			o.append("\t$(cmd_c) -x $(c_source_compiler_language) $(CPP_DEFINES) $(CPP_FLAGS) $(CPP_INCLUDE_DIRS) $(CPP_INCLUDE_FILES) \\\n");
			o.append("\t\t-o \"$@\" -c ", quotePath(cpp_src), "\\\n");
			o.append("\t\t-MMD -MQ \"$@\" -MF ", quotePath(cpp_dep), "\\\n");
			o.append("\n");
		}else if (f.type_is_cpp()) {
			if (proj.pch_header) {
				o.append(escapeString(cpp_obj), ": ", escapeString(pch_header_pch), "\n");
			}
			//makefile optional include cpp_dep
			o.append("-include ", escapeString(cpp_dep), "\n");
			//--------
			o.append(escapeString(cpp_obj), ":", escapeString(cpp_src), "\n");
			o.append("\t@echo \"-------------------------------------------------------------\"\n");
			o.append("\t@echo \"[compile cpp] => $@\"\n");
			o.append("\t$(cmd_mkdir) ", quotePath(Path::dirname(cpp_obj)), "\n");
			o.append("\t$(cmd_cpp) -x $(cpp_source_compiler_language) $(PCH_CC_FLAGS) $(CPP_DEFINES) $(CPP_FLAGS) $(CPP_INCLUDE_DIRS) $(CPP_INCLUDE_FILES) \\\n");
			o.append("\t\t-o \"$@\" -c ", quotePath(cpp_src), "\\\n");
			o.append("\t\t-MMD -MQ \"$@\" -MF ", quotePath(cpp_dep), "\\\n");
			o.append("\n");
		}
	}
	o.append("\n");
	//-------------------------------
	o.append("#----- ", config.name, " output target ----------\n");

	auto& outputTarget = config.outputTarget.path();
	String outputTargetDir = Path::dirname(outputTarget);

	if (proj.type == ProjectType::cpp_exe || proj.type == ProjectType::c_exe) {
		o.append(escapeString(outputTarget), ": $(LINK_FILES)\n");
		o.append("\t@echo \"-------------------------------------------------------------\"\n");
		o.append("\t@echo \"[cpp_exe] $@\"\n");
		o.append("\t$(cmd_mkdir) ", quotePath(outputTargetDir), "\n"); //gmake cannot handle path contain 'space' in function $(@D)
		o.append("\t$(cmd_link) -o \"$@\" $(CPP_OBJ_FILES) -lstdc++ -Wl,--start-group $(LINK_FILES) $(LINK_LIBS) -Wl,--end-group $(LINK_FLAGS)\n");
		o.append("\n");
		o.append(config.name, "__run: ", escapeString(outputTarget), "\n");
		o.append("\t", quotePath(outputTarget), "\n");
		o.append("\n");
	} else if (proj.type == ProjectType::cpp_lib || proj.type == ProjectType::c_lib) {
		o.append(escapeString(outputTarget), ": $(LINK_FILES)\n");
		o.append("\t@echo \"-------------------------------------------------------------\"\n");
		o.append("\t@echo \"[cpp_lib] $@\"\n");
		o.append("\t$(cmd_mkdir) ", quotePath(outputTargetDir), "\n"); // gmake cannot handle path contain 'space' in function $(@D)
		o.append("\t$(cmd_ar) \"$@\" $(CPP_OBJ_FILES)\n");
		o.append("\n");
		o.append("run_", config.name, ": ", escapeString(outputTarget), "\n");
		o.append("\t @echo cannot run cpp_lib ", quotePath(outputTarget), "\n");
		o.append("\n");
	}else if (proj.type == ProjectType::cpp_headers || proj.type == ProjectType::c_headers) {
		//nothing build is needed
	}else{
		throw Error("unknown project.type ", proj.input.type, "\n");
	}

	o.append("#----- ", config.name, " output target dependencies ----------\n");
	o.append(escapeString(outputTarget), ":\\\n");

	for (auto& f : proj.fileEntries) {
		if (f.excludedFromBuild) continue;
		o.append("\t", escapeString(get_obj_file(config, f)), "\\\n");
	}
	for (auto& f : config.link_files._final) {
		o.append("\t", escapeString(f.path()), "\\\n" );
	}

	o.append("\n");
}

} //namespace
