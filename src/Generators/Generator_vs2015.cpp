#include "../common.h"
#include "../App.h"
#include "Generator_vs2015.h"

namespace ax_gen {

Generator_vs2015::Generator_vs2015() {
	if (!g_ws->generator) g_ws->generator = "vs2015";
	if (!g_ws->compiler) g_ws->compiler = "vc";
}

void Generator_vs2015::generate() {
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

	gen_workspace();
}

void Generator_vs2015::ide() {
	Log::info("=========== Open IDE ===============");
	System::shellOpen(g_ws->genData_vs2015.sln);
}

StrView Generator_vs2015::slnFileHeader() {
	return  "Microsoft Visual Studio Solution File, Format Version 12.00\r\n"
			"# Visual Studio 14\r\n"
			"VisualStudioVersion = 14.0.25420.1\r\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\r\n"
			"\r\n";
}

StrView Generator_vs2017::slnFileHeader() {
	return  "Microsoft Visual Studio Solution File, Format Version 12.00\r\n"
			"# Visual Studio 15\r\n"
			"VisualStudioVersion = 15.0.26403.0\r\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\r\n"
			"\r\n";
}

void Generator_vs2015::build() {
	Log::info("=========== Build ===============");

	auto* proj = g_ws->_startup_project;
	if (!proj) {
		Log::error("no startup project to build");
		return;
	}

	String target(proj->input.group);
	target.replaceChars('/', '\\');
	target.append('\\', proj->name);

	StrView configName = g_ws->defaultConfigName();

	String args("\"", g_ws->genData_vs2015.sln, "\"",
				" /project \"", proj->name, "\"",
				" /ProjectConfig \"", configName, "\"",
				" /build");

	System::createProcess("C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\IDE\\devenv.exe", args);
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
		wr.write("lastGenId", (double)_lastGenId.v64);
		{
			auto scope = wr.objectScope("projects");
			for (auto& proj : g_ws->projects) {
				auto scope = wr.objectScope(proj.name);
				wr.write("uuid", proj.genData_vs2015.uuid);
			}
		}
		{
			auto scope = wr.objectScope("groups");
			for (auto& group : g_ws->projectGroups.dict) {
				auto scope = wr.objectScope(group.path);
				wr.write("uuid", group.genData_vs2015.uuid);
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
	proj.genData_vs2015.vcxproj.set(g_ws->outDir, proj.name, ".vcxproj");

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
	}

	FileUtil::writeTextFile(proj.genData_vs2015.vcxproj, wr.buffer());
}

void Generator_vs2015::gen_project_files(XmlWriter& wr, Project& proj) {
	auto tag = wr.tagScope("ItemGroup");

	gen_project_pch(wr, proj);

	for (auto& f : proj.fileEntries) {
		switch (f.type()) {
			case FileType::cpp_header: {
				auto tag = wr.tagScope("ClInclude");
				wr.attr("Include", f.name());
			}break;

			case FileType::c_source: ax_fallthrough
			case FileType::cpp_source: {
				auto tag = wr.tagScope("ClCompile");
				wr.attr("Include", f.name());
				if (f.excludedFromBuild) {
					wr.tagWithBody("ExcludedFromBuild", "true");
				}
			}break;
			default: {
				auto tag = wr.tagScope("None");
				wr.attr("Include", f.name());
			}break;
		}
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

	proj.pch_cpp.init(filename);
	proj.pch_cpp.generated = true;

	String code;
	code.append("//-- Auto Generated File for Visual C++ precompiled header\n");
	code.append("#include \"", proj.pch_header, "\"\n");

	FileUtil::writeTextFile(filename, code);

	auto tag = wr.tagScope("ClCompile");
	wr.attr("Include", filename);
	wr.tagWithBody("PrecompiledHeader", "Create");
}

void Generator_vs2015::gen_project_config(XmlWriter& wr, Project& proj, Config& config) {
	String cond("'$(Configuration)|$(Platform)'=='", config.name, "|", vcxproj_cpu, "'");
	{
		auto tag = wr.tagScope("PropertyGroup");
		wr.attr("Condition", cond);
		wr.tagWithBody("IntDir", config._build_tmp_dir);

		String	targetDir  = Path::dirname  ( config.outputTarget);
		auto	targetName = Path::basename (config.outputTarget, false);
		auto	targetExt  = Path::extension(config.outputTarget);

		targetDir.append('/');
		wr.tagWithBody("OutDir",	 targetDir);
		wr.tagWithBody("TargetName", targetName);
		{
			String tmp;
			tmp.append(".", targetExt);
			wr.tagWithBody("TargetExt", tmp);
		}
	}
	{
		auto tag = wr.tagScope("ItemDefinitionGroup");
		wr.attr("Condition", cond);

		{
			auto tag = wr.tagScope("ClCompile");
				
			wr.tagWithBody("PreprocessorDefinitions", "%(PreprocessorDefinitions)");

			if (!vsForLinux()) {

				wr.tagWithBody("DebugInformationFormat", "ProgramDatabase");
				wr.tagWithBody("SDLCheck", "true");

				wr.tagWithBodyBool("MultiProcessorCompilation", proj.multithread_build());

				if (config.isDebug) {
					wr.tagWithBody("Optimization", "Disabled");
					wr.tagWithBody("MinimalRebuild", "true");
					wr.tagWithBody("RuntimeLibrary", "MultiThreadedDebugDLL");
					wr.tagWithBody("LinkIncremental", "true");
				}
				else {
					wr.tagWithBody("Optimization", "MaxSpeed");
					wr.tagWithBody("MinimalRebuild", "false");
					wr.tagWithBody("WholeProgramOptimization", "true");
					wr.tagWithBody("RuntimeLibrary", "MultiThreadedDLL");
					wr.tagWithBody("FunctionLevelLinking", "true");
					wr.tagWithBody("IntrinsicFunctions", "true");
					wr.tagWithBody("WholeProgramOptimization", "true");
					wr.tagWithBody("BasicRuntimeChecks", "Default");
				}
			}

			if (proj.pch_header) {				
				wr.tagWithBody("PrecompiledHeader",		"Use");
				wr.tagWithBody("PrecompiledHeaderFile",	proj.pch_header);
				wr.tagWithBody("ForcedIncludeFiles",	proj.pch_header);
			}

			if (config.warning_level) {
				wr.tagWithBody("WarningLevel", config.warning_level);
			}

			wr.tagWithBodyBool("TreatWarningAsError", config.warning_as_error);

			gen_config_option(wr, "DisableSpecificWarnings",		config.disable_warning._final);
			gen_config_option(wr, "PreprocessorDefinitions",		config.cpp_defines._final);
			gen_config_option(wr, "AdditionalIncludeDirectories",	config.include_dirs._final);
		}
		{
			auto tag = wr.tagScope("Link");
			wr.tagWithBody("SubSystem", "Console");
			wr.tagWithBody("GenerateDebugInformation", "true");

			if (proj.type_is_exe_or_dll()) {
				gen_config_option(wr, "AdditionalLibraryDirectories",	config.link_dirs._final);
				gen_config_option(wr, "AdditionalDependencies",			config.link_files._final);
			}

			if (!config.isDebug) {
				wr.tagWithBodyBool("EnableCOMDATFolding", true);
				wr.tagWithBodyBool("OptimizeReferences",  true);
			}
		}
	}
}

void Generator_vs2015::gen_config_option(XmlWriter& wr, const StrView& name, Vector<String>& value) {
	String tmp;
	for (auto& p : value) {
		tmp.append(p, ";");
	}
	tmp.append("%(", name, ")");
	wr.tagWithBody(name, tmp);
}

void Generator_vs2015::gen_workspace() {
	Log::info("gen_workspace ", g_ws->workspace_name);

	g_ws->genData_vs2015.sln.set(g_ws->outDir, g_ws->workspace_name, ".sln");

	//cache file
	String cacheFilename;
	cacheFilename.append(g_ws->outDir, "_ax_gen_cache.json");

	readCacheFile(cacheFilename);

	for (auto& p : g_ws->projects) {
		gen_project(p);
		gen_vcxproj_filters(p);
	}

	String o;
	o.append(slnFileHeader());

	{
		o.append("\n# ---- projects ----\n");
		for (auto& proj : g_ws->projects) {
			o.append("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = ",
					 "\"", proj.name, "\", \"", proj.name, ".vcxproj\", \"", proj.genData_vs2015.uuid, "\"\n");

			if (proj._dependencies_inherit) {
				if (proj.type_is_exe_or_dll()) {
					o.append("\tProjectSection(ProjectDependencies) = postProject\n");
					for (auto& dp : proj._dependencies_inherit) {
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
				o.append("\t\t", proj->genData_vs2015.uuid, " = ", c.genData_vs2015.uuid, "\n");
			}
		}
		o.append("\tEndGlobalSection\n");
		o.append("EndGlobal\n");
	}
	
	writeCacheFile(cacheFilename);

	FileUtil::writeTextFile(g_ws->genData_vs2015.sln, o);
}

void Generator_vs2015::gen_vcxproj_filters(Project& proj) {
	XmlWriter wr;

	{
		wr.writeHeader();
		auto tag = wr.tagScope("Project");
		wr.attr("ToolsVersion", "4.0");
		wr.attr("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

		if (proj.pch_cpp.name()) {
			auto& f = proj.pch_cpp;
			proj.virtualFolders.add(g_ws->outDir, f);
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
					wr.attr("Include", f->name());
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
	filename.append(g_ws->outDir, proj.name, ".vcxproj.filters");
	FileUtil::writeTextFile(filename, wr.buffer());
}

} //namespace

