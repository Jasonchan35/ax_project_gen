#include "../common.h"
#include "../App.h"
#include "Generator_vs2015.h"

namespace ax_gen {

void Generator_vs2015::onInit() {
	if (vsForLinux()) {
		if (!g_ws->os ) g_ws->os  = "linux";
		if (!g_ws->cpu) g_ws->cpu = "x86_64";
		if (!g_ws->compiler) g_ws->compiler = "gcc";
	}else{
		if (!g_ws->compiler) g_ws->compiler = "vc";
	}
}

void Generator_vs2015::onGenerate() {
	if (vsForLinux()) {
		if (g_ws->os != "linux") {
			throw Error("Unsupported OS ", g_ws->os);
		}
	}else{
		if (g_ws->os != "windows") {
			throw Error("Unsupported OS ", g_ws->os);
		}
	}

	if (g_ws->cpu == "x86") {
		vcxproj_cpu = "Win32";
	}else if (g_ws->cpu == "x86_64") {
		vcxproj_cpu = "x64";
	}else{
		throw Error("Unsupported cpu type ", g_ws->cpu);
	}

	//cache file
	String cacheFilename;
	cacheFilename.append(g_ws->buildDir, "_ax_gen_cache.json");
	readCacheFile(cacheFilename);

	for (auto& p : g_ws->projects) {
		gen_project(p);
		gen_vcxproj_filters(p);
	}

	gen_workspace(g_ws->masterWorkspace);
	for (auto& ew : g_ws->extraWorkspaces) {
		gen_workspace(ew);
	}

	writeCacheFile(cacheFilename);

}

void Generator_vs2015::onIde() {
	System::shellOpen(g_ws->masterWorkspace.genData_vs2015.sln);
}

StrView Generator_vs2015::slnFileHeader() {
	return  StrView("Microsoft Visual Studio Solution File, Format Version 12.00\r\n"
					"# Visual Studio 14\r\n"
					"VisualStudioVersion = 14.0.25420.1\r\n"
					"MinimumVisualStudioVersion = 10.0.40219.1\r\n"
					"\r\n");
}

StrView Generator_vs2017::slnFileHeader() {
	return  StrView("Microsoft Visual Studio Solution File, Format Version 12.00\r\n"
					"# Visual Studio 15\r\n"
					"VisualStudioVersion = 15.0.26403.0\r\n"
					"MinimumVisualStudioVersion = 10.0.40219.1\r\n"
					"\r\n");
}

void Generator_vs2015::onBuild() {
	auto* proj = g_ws->startupProject;
	if (!proj) {
		Log::error("no startup project to build");
		return;
	}

	String target(proj->input.group);
	target.replaceChars('/', '\\');
	target.append('\\', proj->name);

	String args("\"", g_ws->masterWorkspace.genData_vs2015.sln, "\"",
				" /project \"", proj->name, "\"",
				" /ProjectConfig \"", g_app->options.config, "\"",
				" /build");

	int exit_code = System::createProcess("C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\IDE\\devenv.exe", args);
	if (exit_code != 0){
		throw Error("Error Build");
	}
}

void Generator_vs2015::readCacheFile(const StrView& filename) {
	if (!Path::fileExists(filename)) return;		

	String json;
	FileUtil::readTextFile(filename, json);

	JsonReader reader(json, filename);
	reader.beginObject();
	
	double lastGenIdTmp;
	
	while (!reader.endObject()) {
		if (reader.member("lastGenId", lastGenIdTmp)) {
			_lastGenId.v64 = static_cast<uint64_t>(lastGenIdTmp);
			continue;
		}
		if (reader.beginObject("projects")) {
			while (!reader.endObject()) {
				String m;
				reader.getMemberName(m);
				auto* proj = g_ws->projects.find(m);
				if (!proj) {
					reader.skipValue();
					continue;
				}
				
				reader.beginObject();
				while (!reader.endObject()) {
					if (reader.member("uuid", proj->genData_vs2015.uuid)) continue;
				}
			}
			continue;
		}
		if (reader.beginObject("groups")) {
			while (!reader.endObject()) {
				String m;
				reader.getMemberName(m);

				auto* cat = g_ws->projectGroups.dict.find(m);
				if (!cat) {
					reader.skipValue();
					continue;
				}

				reader.beginObject();
				while (!reader.endObject()) {
					if (reader.member("uuid", cat->genData_vs2015.uuid)) continue;
				}
			}
			continue;
		}
	}
}

void Generator_vs2015::writeCacheFile(const StrView& filename) {
	JsonWriter wr;
	{
		auto scope = wr.objectScope();
		wr.member("lastGenId", (double)_lastGenId.v64);
		{
			auto scope = wr.objectScope("projects");
			for (auto& proj : g_ws->projects) {
				auto scope = wr.objectScope(proj.name);
				wr.member("uuid", proj.genData_vs2015.uuid);
			}
		}
		{
			auto scope = wr.objectScope("groups");
			for (auto& group : g_ws->projectGroups.dict) {
				auto scope = wr.objectScope(group.path);
				wr.member("uuid", group.genData_vs2015.uuid);
			}
		}
	}
	FileUtil::writeTextFile(filename, wr.buffer());
}

ax_gen::StrView Generator_vs2015::_visualc_PlatformToolset(Project& proj) {
	if (proj.input.visualc_PlatformToolset) {
		return proj.input.visualc_PlatformToolset;
	}
	return visualc_PlatformToolset();
}

void Generator_vs2015::genUuid(String& outStr) {
	if (outStr) return;

	_lastGenId.v64++;

	char tmp[100+1];
	snprintf(tmp, 100, "{AAAAAAAA-0000-0000-%02X%02X-%02X%02X%02X%02X%02X%02X}",
						_lastGenId.c[7],
						_lastGenId.c[6],
						_lastGenId.c[5],
						_lastGenId.c[4],
						_lastGenId.c[3],
						_lastGenId.c[2],
						_lastGenId.c[1],
						_lastGenId.c[0]);
	tmp[100] = 0;

	outStr.set(StrView_c_str(tmp));
}

void Generator_vs2015::gen_project(Project& proj) {
	proj.genData_vs2015.vcxproj.set(g_ws->buildDir, proj.name, ".vcxproj");

	Log::info("gen_project ", proj.genData_vs2015.vcxproj);
	genUuid(proj.genData_vs2015.uuid);

	XmlWriter wr;
	{
		wr.writeHeader();
		auto tag = wr.tagScope("Project");

		wr.attr("DefaultTargets", "Build");
		wr.attr("ToolsVersion", vcxprojToolsVersion());
		wr.attr("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

		{
			auto tag = wr.tagScope("ItemGroup");
			wr.attr("Label", "ProjectConfigurations");

			for (auto& config : proj.configs) {
				auto tag = wr.tagScope("ProjectConfiguration");
				wr.attr("Include", String(config.name, "|", vcxproj_cpu));

				wr.tagWithBody("Configuration", config.name);
				wr.tagWithBody("Platform", vcxproj_cpu);
			}
		}
		{
			auto tag = wr.tagScope("PropertyGroup");
			wr.attr("Label", "Globals");
			wr.tagWithBody("ProjectGuid",	proj.genData_vs2015.uuid);
			wr.tagWithBody("Keyword",		"Win32Proj");
			wr.tagWithBody("RootNamespace", proj.name);

			if (vsForLinux()) {
				wr.tagWithBody("ApplicationType",			"Linux");
				wr.tagWithBody("ApplicationTypeRevision",	"1.0");
				wr.tagWithBody("TargetLinuxPlatform",		"Generic");
				wr.tagWithBody("LinuxProjectType",			"{D51BCBC9-82E9-4017-911E-C93873C4EA2B}");
			}else{
				wr.tagWithBody("WindowsTargetPlatformVersion", "8.1");
			}
		}

		gen_project_files(wr, proj);

		//-----------
		{
			auto tag = wr.tagScope("Import");
			wr.attr("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
		}
		{
			auto tag = wr.tagScope("PropertyGroup");
			wr.attr("Label", "Configuration");

			StrView productType;

			if (proj.type_is_headers() || proj.type_is_lib()) {
				productType = "StaticLibrary";
				
			}else if (proj.type_is_dll()) {
				productType = "DynamicLibrary";
				
			}else if (proj.type_is_exe()) {
				productType = "Application";
				
			}else{
				throw Error("Unhandled project type", proj.input.type);
			}
			
			wr.tagWithBody("ConfigurationType", productType);
		
			wr.tagWithBody("CharacterSet",    "Unicode");

			if (vsForLinux()) {
				wr.tagWithBody("PlatformToolset", "Remote_GCC_1_0");
				if (g_ws->compiler=="gcc") {
					wr.tagWithBody("RemoteCCompileToolExe","gcc");
					wr.tagWithBody("RemoteCppCompileToolExe","g++");
				}else if(g_ws->compiler=="clang") {
					wr.tagWithBody("RemoteCCompileToolExe","clang");
					wr.tagWithBody("RemoteCppCompileToolExe","clang++");
				}else{
					throw Error("Unsupported compiler ", g_ws->compiler);
				}

				String relBuildDir;
				Path::getRel(relBuildDir, g_ws->buildDir, g_ws->axworkspaceDir);

				String remoteRootDir("_vsForLinux/", g_ws->_platformName, "/", relBuildDir);
				wr.tagWithBody("RemoteRootDir", remoteRootDir);

				// vsForLinux will delete everything inside $(RemoteProjectDir) before build,
				// that's why we have to separate ProjectDir for each Project to avoid file get deleted by another project
				// and it caused another problem, because the solution and projects are in the same folder in local,
				// but on the remote it's on different folder, it makes Visual Studio cannot mapping local->remote file properly
				// so, we have to setup RemotePostBuildEvent to copy output lib/exe back to the correct output folder (g_ws->buildDir /lib or /bin)
				String projectDir("$(RemoteRootDir)/../ProjectDir_", proj.name);
				wr.tagWithBody("RemoteProjectDir", projectDir);


				//-- define our own source file mapping, because VS will strip out all "../" in the path
				String fileToCopy;
				for (auto& f : proj.fileEntries) {
					fileToCopy.append(f.path(), ":=", projectDir,"/", f.path(), ";");
				}

				wr.tagWithBody("SourcesToCopyRemotelyOverride", "");
				wr.tagWithBody("AdditionalSourcesToCopyMapping", fileToCopy);
//				wr.tagWithBody("LocalRemoteCopySources", "false");

			}else {
				wr.tagWithBody("PlatformToolset", _visualc_PlatformToolset(proj));
			}
		}

		//-----------
		{
			auto tag = wr.tagScope("Import");
			wr.attr("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
		}
		{
			auto tag = wr.tagScope("ImportGroup");
			wr.attr("Label", "PropertySheets");
		}

		if (vsForLinux()) {
			{
				auto tag = wr.tagScope("ImportGroup");
				wr.attr("Label", "ExtensionSettings");
			}

			{
				auto tag = wr.tagScope("ImportGroup");
				wr.attr("Label", "Shared");
			}
		}
		
		{
			auto tag = wr.tagScope("PropertyGroup");
			wr.attr("Label", "UserMacros");
		}

		//-----------
		for (auto& config : proj.configs) {
			gen_project_config(wr, proj, config);
		}

		//-----------
		{
			auto tag = wr.tagScope("Import");
			wr.attr("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
		}
		{
			auto tag = wr.tagScope("ImportGroup");
			wr.attr("Label", "ExtensionTargets");
		}

		if (proj.input.enable_cuda != ""){
			String path;
			{
				auto group_tag = wr.tagScope("ImportGroup");
				wr.attr("Label", "ExtensionSettings");
				auto import_tag = wr.tagScope("Import");
				path.set("$(VCTargetsPath)\\BuildCustomizations\\CUDA ", 
					proj.input.enable_cuda, 
					".props");
				wr.attr("Project", path);
			}
			{
				auto group_tag = wr.tagScope("ImportGroup");
				wr.attr("Label", "ExtensionTargets");
				auto import_tag = wr.tagScope("Import");
				path.set("$(VCTargetsPath)\\BuildCustomizations\\CUDA ", 
					proj.input.enable_cuda, 
					".targets");
				wr.attr("Project", path);
			}
		}
	}

	FileUtil::writeTextFile(proj.genData_vs2015.vcxproj, wr.buffer());
}

void Generator_vs2015::gen_project_files(XmlWriter& wr, Project& proj) {
	auto tag = wr.tagScope("ItemGroup");

	gen_project_pch(wr, proj);

	for (auto& f : proj.fileEntries) {
		StrView tagName;
		bool excludedFromBuild = false;

		switch (f.type()) {
			case FileType::c_source: ax_fallthrough
			case FileType::cpp_source: {
				tagName = "ClCompile";
				excludedFromBuild = f.excludedFromBuild;
			}break;
			case FileType::cu_header: ax_fallthrough
			case FileType::cu_source: {
				tagName = "CudaCompile";
				excludedFromBuild = f.excludedFromBuild;
			}break;

			default: { tagName = "ClInclude"; }break;
		}

		auto tag = wr.tagScope(tagName);
		wr.attr("Include", f.path());
		if (excludedFromBuild) {
			wr.tagWithBody("ExcludedFromBuild", "true");
		}

		//using our own file mapping to copy, so disable the default one
		wr.tagWithBody("RemoteCopyFile", "false");
	}
}

void Generator_vs2015::gen_project_pch(XmlWriter& wr, Project& proj) {
	if (!proj.pch_header) return;

	StrView pch_ext;

	if (proj.type_is_cpp()) {
		pch_ext = StrView(".cpp");
	}else if (proj.type_is_c()) {
		pch_ext = StrView(".c");
	}else{
		return;
	}
	
	String filename;
	filename.append(proj._generatedFileDir, proj.name, "-precompiledHeader", pch_ext);

	proj.pch_cpp.init(filename, false, true);

	String code;
	code.append("//-- Auto Generated File for Visual C++ precompiled header\n");

	String tmp;
	Path::getRel(tmp, proj.pch_header->absPath(), proj._generatedFileDir);

	code.append("#include \"", tmp, "\"\n");

	FileUtil::writeTextFile(filename, code);

	auto tag = wr.tagScope("ClCompile");
	wr.attr("Include", proj.pch_cpp.path());
	wr.tagWithBody("PrecompiledHeader", "Create");
}

void Generator_vs2015::gen_project_config(XmlWriter& wr, Project& proj, Config& config) {
	String cond("'$(Configuration)|$(Platform)'=='", config.name, "|", vcxproj_cpu, "'");
	{
		auto tag = wr.tagScope("PropertyGroup");
		wr.attr("Condition", cond);

		//------
		auto& outputTarget = config.outputTarget.path();
		String	targetDir;
		if (vsForLinux()) targetDir.append("../", g_ws->_platformName, "/");
		targetDir.append(Path::dirname(outputTarget));
		if (!targetDir) targetDir.append('.'); // for current dir, should using "./"
		targetDir.append('/');

		//------
		String intermediaDir;
		intermediaDir.append(config._build_tmp_dir.path());
		if (!intermediaDir) intermediaDir.append('.'); // for current dir, should using "./"
		intermediaDir.append('/');

		//------
		auto	targetName = Path::basename (outputTarget, false);
		auto	targetExt  = Path::extension(outputTarget);

		wr.tagWithBody("OutDir",		targetDir);
		wr.tagWithBody("IntDir",		intermediaDir);
		wr.tagWithBody("TargetName",	targetName);
		{
			String tmp;
			if (targetExt) {
				tmp.append(".", targetExt);
			}
			wr.tagWithBody("TargetExt", tmp);
		}
	}
	{
		auto tag = wr.tagScope("ItemDefinitionGroup");
		wr.attr("Condition", cond);
		{
			auto tag = wr.tagScope("ClCompile");
				
			wr.tagWithBody("PreprocessorDefinitions", "%(PreprocessorDefinitions)");

			if (vsForLinux()) {
				wr.tagWithBody("Verbose", "true");

				if (config.isDebug) {
					wr.tagWithBody("DebugInformationFormat",	"FullDebug");
					wr.tagWithBody("Optimization",				"Disabled");
					wr.tagWithBody("OmitFramePointers",			"false");

				}else{
					wr.tagWithBody("DebugInformationFormat",	"None");
					wr.tagWithBody("Optimization",				"Full");
					wr.tagWithBody("OmitFramePointers",			"true");
				}

			}else{
				wr.tagWithBody("SDLCheck", "true");
				wr.tagWithBodyBool("MultiProcessorCompilation", proj.multithread_build());

				if (config.isDebug) {
					wr.tagWithBody("Optimization",				"Disabled");
					wr.tagWithBody("DebugInformationFormat",	"ProgramDatabase");
					wr.tagWithBody("RuntimeLibrary",			"MultiThreadedDebugDLL");
					wr.tagWithBody("LinkIncremental",			"true");
				}
				else {
					wr.tagWithBody("Optimization",				"MaxSpeed");
					wr.tagWithBody("DebugInformationFormat",	"None");
					wr.tagWithBody("WholeProgramOptimization",	"true");
					wr.tagWithBody("RuntimeLibrary",			"MultiThreadedDLL");
					wr.tagWithBody("FunctionLevelLinking",		"true");
					wr.tagWithBody("IntrinsicFunctions",		"true");
					wr.tagWithBody("WholeProgramOptimization",	"true");
					wr.tagWithBody("BasicRuntimeChecks",		"Default");
				}
			}

			if (proj.pch_header) {
				wr.tagWithBody("PrecompiledHeader",		"Use");

				String pch("$(ProjectDir)", proj.pch_header->path());
				wr.tagWithBody("PrecompiledHeaderFile",	pch);
				wr.tagWithBody("ForcedIncludeFiles",	pch);
			}

			if (config.warning_level) {
				wr.tagWithBody("WarningLevel", config.warning_level);
			}

			wr.tagWithBodyBool("TreatWarningAsError", config.warning_as_error);

			gen_config_option(wr, "DisableSpecificWarnings",		config.disable_warning._final);
			gen_config_option(wr, "PreprocessorDefinitions",		config.cpp_defines._final);
			gen_config_option(wr, "AdditionalIncludeDirectories",	config.include_dirs._final);

			for (auto& s : config.vs2015_ClCompile.pairs()) {
				wr.tagWithBody(s.key, *s.value);
			}
		}
		{
			auto tag = wr.tagScope("Link");

			if (proj.type_is_exe_or_dll()) {
				gen_config_option(wr, "AdditionalLibraryDirectories",	config.link_dirs._final);

				if (vsForLinux()) {
					gen_config_option(wr, "AdditionalDependencies",		config.link_files._final, "$(RemoteRootDir)/");
				}else{
					if (proj.input.enable_cuda != ""){
						config.link_files._final.add("cudart.lib");
					}
					gen_config_option(wr, "AdditionalDependencies",		config.link_files._final);
				}
			}

			if (vsForLinux()) {
				wr.tagWithBody("VerboseOutput", "true");

				{
					String tmp;
					for (auto& p : config.link_flags._final) {
						tmp.append(" -Wl,", p.path());
					}
					wr.tagWithBody("AdditionalOptions", tmp);
				}

				if (config.isDebug) {
					wr.tagWithBody("DebuggerSymbolInformation", "true");
				}else{
					wr.tagWithBody("DebuggerSymbolInformation", "OmitAllSymbolInformation");
				}

			}else{
				wr.tagWithBody("SubSystem", "Console");
				wr.tagWithBody("GenerateDebugInformation", "true");

				{
					String tmp;
					for (auto& p : config.link_flags._final) {
						tmp.append(" ", p.path());
					}
					wr.tagWithBody("AdditionalOptions", tmp);
				}

				if (!config.isDebug) {
					wr.tagWithBodyBool("EnableCOMDATFolding", true);
					wr.tagWithBodyBool("OptimizeReferences",  true);
				}
			}

			for (auto& s : config.vs2015_Link.pairs()) {
				wr.tagWithBody(s.key, *s.value);
			}
		}

		if (config.outputTarget.path()) {
			auto tag = wr.tagScope("RemotePostBuildEvent");			
			String dir("$(RemoteRootDir)/", Path::dirname(config.outputTarget.path()));

			String cmd;
			cmd.append("mkdir -p \"", dir,"\";");
			cmd.append("cp -f \"$(RemoteProjectDir)/", config.outputTarget.path(), "\" \"$(RemoteRootDir)/", config.outputTarget.path(), "\"");
			wr.tagWithBody("Command", cmd);
		}
	}
}

void Generator_vs2015::gen_config_option(XmlWriter& wr, const StrView& name, Config::EntryDict& value, const StrView& relativeTo) {
	String tmp;
	for (auto& p : value) {
		tmp.append(relativeTo, p.path(), ";");
	}
	tmp.append("%(", name, ")");
	wr.tagWithBody(name, tmp);
}

void Generator_vs2015::gen_workspace(ExtraWorkspace& ws) {
	Log::info("gen_workspace ", ws.name);

	ws.genData_vs2015.sln.set(g_ws->buildDir, ws.name, ".sln");

	String o;
	o.append(slnFileHeader());

	{
		o.append("\n# ---- projects ----\n");
		for (auto& proj : ws.projects) {
			o.append("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = ",
					 "\"", proj->name, "\", \"", proj->name, ".vcxproj\", \"", proj->genData_vs2015.uuid, "\"\n");

			if (proj->_dependencies_inherit) {
				if (proj->type_is_exe_or_dll()) {
					o.append("\tProjectSection(ProjectDependencies) = postProject\n");
					for (auto& dp : proj->_dependencies_inherit) {
						o.append("\t\t", dp->genData_vs2015.uuid, " = ", dp->genData_vs2015.uuid, "\n");
					}
					o.append("\tEndProjectSection\n");
				}
			}
			o.append("EndProject\n");
		}
	}
	{
		auto* root = g_ws->projectGroups.root;
		o.append("\n# ---- Groups ----\n");
		for (auto& c : g_ws->projectGroups.dict) {
			if (&c == root) continue;
			genUuid(c.genData_vs2015.uuid);

			auto catName = Path::basename(c.path, true);
			o.append("Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"",
						catName, "\", \"", catName, "\", \"", c.genData_vs2015.uuid, "\"\n");
			o.append("EndProject\n");
		}

		o.append("\n# ----  (ProjectGroups) -> parent ----\n");
		o.append("Global\n");
		o.append("\tGlobalSection(NestedProjects) = preSolution\n");
		for (auto& c : g_ws->projectGroups.dict) {
			if (c.parent && c.parent != root) {
				o.append("\t\t", c.genData_vs2015.uuid, " = ", c.parent->genData_vs2015.uuid, "\n");
			}

			for (auto& proj : c.projects) {
				if (ws.projects.indexOf(proj) < 0) continue;
				o.append("\t\t", proj->genData_vs2015.uuid, " = ", c.genData_vs2015.uuid, "\n");
			}
		}
		o.append("\tEndGlobalSection\n");
		o.append("EndGlobal\n");
	}
	
	FileUtil::writeTextFile(ws.genData_vs2015.sln, o);
}

void Generator_vs2015::gen_vcxproj_filters(Project& proj) {
	XmlWriter wr;

	{
		wr.writeHeader();
		auto tag = wr.tagScope("Project");
		wr.attr("ToolsVersion", "4.0");
		wr.attr("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

		if (proj.pch_cpp.path()) {
			auto& f = proj.pch_cpp;
			proj.virtualFolders.add(g_ws->buildDir, f);
		}

		//------------
		{
			auto tag = wr.tagScope("ItemGroup");
			for (auto& d : proj.virtualFolders.dict) {
				if (d.path == ".") continue;
				auto tag = wr.tagScope("Filter");
				String winPath;
				Path::windowsPath(winPath, d.path);
				wr.attr("Include", winPath);
			}
		}

		{
			auto tag = wr.tagScope("ItemGroup");
			for (auto& pair : proj.virtualFolders.dict.pairs()) {
				for (auto f : pair.value->files) {
					if (!f) continue;

					auto type = StrView("ClInclude");
					switch (f->type()) {
						case FileType::c_source: ax_fallthrough
						case FileType::cpp_source: {
							type = "ClCompile"; 
						}break;
						default: break;
					}

					auto tag = wr.tagScope(type);
					wr.attr("Include", f->path());
					if (pair.key) {
						String winPath;
						Path::windowsPath(winPath, pair.key);
						wr.tagWithBody("Filter", winPath);
					}
				}
			}
		}
	}	

	String filename;
	filename.append(g_ws->buildDir, proj.name, ".vcxproj.filters");
	FileUtil::writeTextFile(filename, wr.buffer());
}

} //namespace

