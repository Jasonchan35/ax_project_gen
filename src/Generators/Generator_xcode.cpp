#include "Generator_xcode.h"
#include "../Config/Workspace.h"

namespace ax_gen {

const StrView Generator_xcode::build_phase_sources_uuid		= "B0000000B0000000B0000002";
const StrView Generator_xcode::build_phase_frameworks_uuid	= "B0000000B0000000B0000003";
const StrView Generator_xcode::build_phase_headers_uuid 	= "B0000000B0000000B0000004";

const StrView Generator_xcode::main_group_uuid   			= "C0000000B0000000B0000001";
const StrView Generator_xcode::product_group_uuid			= "C0000000B0000000B0000002";
const StrView Generator_xcode::dependencies_group_uuid		= "C0000000B0000000B0000003";
const StrView Generator_xcode::kSourceTreeGroup				= "\"<group>\"";
const StrView Generator_xcode::kSourceTreeAbsolute			= "\"<absolute>\"";


void Generator_xcode::onInit() {
	if (!g_ws->compiler) g_ws->compiler = "clang";
	if (!g_ws->os) g_ws->os = "macosx";
}

void Generator_xcode::onGenerate() {
	if (g_ws->os == "macosx") {
		//
	}else if (g_ws->os == "ios") {
		//
	}else{
		throw Error("Unsupported os ", g_ws->os);
	}
	gen_workspace();
}

void Generator_xcode::onBuild() {
	auto* proj = g_ws->startupProject;
	if (!proj) {
		Log::error("no startup project to build");
		return;
	}

	String args(" -workspace \"", g_ws->genData_xcode.xcworkspace, "\"",
				" -scheme \"", proj->name, "\"",
				" -configuration \"", g_app->options.config, "\"",
				" build");
	int exit_code = System::createProcess("/usr/bin/xcodebuild", args);
	if (exit_code != 0){
		throw Error("Error Build");
	}
}

void Generator_xcode::onIde() {
#if ax_OS_MacOSX
	String cmd("open \"", g_ws->genData_xcode.xcworkspace, "\"");
	Log::info("cmd> ", cmd);
	::system(cmd.c_str());
#else
	Log::error("doesn't support on this platform");
#endif
}

void Generator_xcode::gen_workspace() {
	g_ws->genData_xcode.xcworkspace.set(g_ws->buildDir, g_ws->workspace_name, ".xcworkspace");

	//cache file
	String cacheFilename;
	cacheFilename.append(g_ws->buildDir, "_ax_gen_cache.json");

	readCacheFile(cacheFilename);

	for (auto& proj : g_ws->projects) {
		gen_project_genUuid(proj);
	}
	
	for (auto& proj : g_ws->projects) {
		gen_project(proj);
	}

	XmlWriter wr;
	{
		auto tag = wr.tagScope("Workspace");
		wr.attr("version", "1.0");
		gen_workspace_group(wr, *g_ws->projectGroups.root);
	}
	
	writeCacheFile(cacheFilename);	
	
	String filename(g_ws->genData_xcode.xcworkspace, "/contents.xcworkspacedata");
	FileUtil::writeTextFile(filename, wr.buffer());
}

void Generator_xcode::gen_workspace_group(XmlWriter& wr, ProjectGroup& group) {
	for (auto& c : group.children) {
		auto tag = wr.tagScope("Group");
		wr.attr("location", "container:");
		
		auto basename = Path::basename(c->path, true);
		wr.attr("name", basename);
		
		gen_workspace_group(wr, *c);
	}

	String rel;
	for (auto& proj : group.projects) {
		auto tag = wr.tagScope("FileRef");
		wr.attr("location", String("container:", proj->genData_xcode.xcodeproj.path()));
	}
}

void Generator_xcode::gen_project_genUuid(Project& proj) {
	auto& gd = proj.genData_xcode;
	gd.xcodeproj.init(String(g_ws->buildDir, proj.name, ".xcodeproj"), false, true);
	gd.pbxproj.set(gd.xcodeproj.absPath(), "/", "project.pbxproj");
	genUuid(gd.uuid);
	genUuid(gd.targetUuid);
	genUuid(gd.targetProductUuid);
	genUuid(gd.configListUuid);
	genUuid(gd.targetConfigListUuid);
	genUuid(gd.dependencyProxyUuid);
	genUuid(gd.dependencyTargetUuid);
	genUuid(gd.dependencyTargetProxyUuid);	

	for (auto& f : proj.fileEntries) {
		genUuid(f.genData_xcode.uuid);
		genUuid(f.genData_xcode.buildUuid);
	}

	for (auto& f : proj.virtualFolders.dict) {
		genUuid(f.genData_xcode.uuid);
	}

	for (auto& config : proj.configs) {
		genUuid(config.genData_xcode.projectConfigUuid);
		genUuid(config.genData_xcode.targetUuid);
		genUuid(config.genData_xcode.targetConfigUuid);
	}
}

void Generator_xcode::gen_project(Project& proj) {
	auto& gd = proj.genData_xcode;
	
	assert(gd.xcodeproj);
	Log::info("gen_proejct ", gd.xcodeproj);

	if (proj.input.gui_app) {
		gen_info_plist(proj);
	}
	
	XCodePbxWriter wr;
	wr.buffer().append("// !$*UTF8*$!\n");
	{
		auto scope = wr.objectScope();
		wr.member("archiveVersion", "1");
		{
			auto scope = wr.objectScope("classes");
		}
		wr.member("objectVersion", "46");
		{
			auto scope = wr.objectScope("objects");
			gen_project_PBXBuildFile(wr, proj);
			gen_project_dependencies(wr, proj);
			gen_project_PBXGroup(wr, proj);
			gen_project_PBXProject(wr, proj);
			gen_project_PBXSourcesBuildPhase(wr, proj);
			gen_project_PBXNativeTarget(wr, proj);
			gen_project_XCBuildConfiguration(wr, proj);
			gen_project_XCConfigurationList(wr, proj);
		}
		
		wr.member("rootObject", proj.genData_xcode.uuid);
	}

	FileUtil::writeTextFile(gd.pbxproj, wr.buffer());
}

void Generator_xcode::gen_file_reference(XCodePbxWriter& wr, Project& proj, FileEntry& f) {
	wr.newline();
	wr.commentBlock(f.path());
	auto scope = wr.objectScope(f.genData_xcode.uuid);
	auto basename = Path::basename(f.path(), true);
	
	wr.member("isa", StrView("PBXFileReference"));
	wr.member("name", quoteString(basename));
	wr.member("path", quoteString(basename));
	wr.member("sourceTree", kSourceTreeGroup);
	
	StrView explicitFileType;
	switch (f.type()) {
		case FileType::cpp_source: {
			if (proj.input.cpp_as_objcpp) {
				explicitFileType = "sourcecode.cpp.objcpp";
			}else{
				explicitFileType = "sourcecode.cpp.cpp";
			}
		}break;
		case FileType::c_source: {
			if (proj.input.cpp_as_objcpp) {
				explicitFileType = "sourcecode.c.objc";
			}else{
				explicitFileType = "sourcecode.c.c";
			}
		}break;
		default: break;
	}
	
	if (explicitFileType.size()) {
		wr.member("explicitFileType", explicitFileType);
	}
	
}

void Generator_xcode::gen_project_dependencies(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.newline(); wr.commentBlock("----- project dependencies -----------------");
	for (auto& dp : proj._dependencies_inherit) {
		if (!dp->hasOutputTarget) continue;
	
		auto targetBasename = Path::basename(dp->genData_xcode.xcodeproj.path(), true);
	
		wr.newline();
		wr.newline(); wr.commentBlock(dp->name);
		{
			wr.newline(); wr.commentBlock("PBXContainerItemProxy for xcodeproject");
			auto scope = wr.objectScope(dp->genData_xcode.dependencyProxyUuid);
			wr.member("isa", "PBXContainerItemProxy");
			wr.member("containerPortal", dp->genData_xcode.uuid);
			wr.member("proxyType", "2");
			wr.member("remoteInfo", quoteString(dp->name));
		}
		
		{
			wr.newline(); wr.commentBlock("PBXContainerItemProxy for PBXTargetDependency");
			auto scope = wr.objectScope(dp->genData_xcode.dependencyTargetProxyUuid);
			wr.member("isa", "PBXContainerItemProxy");
			wr.member("containerPortal", dp->genData_xcode.uuid);
			wr.member("proxyType", "1");
			wr.member("remoteInfo", quoteString(dp->name));
		}

		{
			wr.newline(); wr.commentBlock("PBXTargetDependency");
			auto scope = wr.objectScope(dp->genData_xcode.dependencyTargetUuid);
			wr.member("isa", "PBXTargetDependency");
			wr.member("name", quoteString(dp->name));
			wr.member("targetProxy", dp->genData_xcode.dependencyTargetProxyUuid);
		}
		
		{
			wr.newline(); wr.commentBlock("PBXFileReference");
			auto scope = wr.objectScope(dp->genData_xcode.uuid);
			wr.member("isa", "PBXFileReference");
			wr.member("name", quoteString(targetBasename));
			wr.member("path", quoteString(dp->genData_xcode.xcodeproj.path()));
			wr.member("sourceTree", kSourceTreeAbsolute);
		}
	}
	
	{
		wr.newline(); wr.commentBlock("------ Folder dependencies");
		auto scope = wr.objectScope(dependencies_group_uuid);
		wr.member("isa", "PBXGroup");
		{
			auto scope = wr.arrayScope("children");
			for (auto& dp : proj._dependencies_inherit) {
				wr.write(dp->genData_xcode.uuid);
			}
		}
		wr.member("sourceTree", kSourceTreeGroup);
		wr.member("name", "_dependencies_");
	}
}

void Generator_xcode::gen_project_PBXBuildFile(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.newline(); wr.commentBlock("------ Begin PBXBuildFile section");
	for (auto& f : proj.fileEntries) {
		if (f.excludedFromBuild) continue;
	
		wr.newline();
		wr.commentBlock(f.path());
		auto scope = wr.objectScope(f.genData_xcode.buildUuid);
		wr.member("isa", "PBXBuildFile");
		wr.member("fileRef", f.genData_xcode.uuid);
	}
	wr.newline(); wr.commentBlock("------ End PBXBuildFile section");
	//-------
	wr.newline();
	wr.newline(); wr.commentBlock("------ Begin PBXFileReference section");
	for (auto& f : proj.fileEntries) {
		gen_file_reference(wr, proj, f);
	}
	wr.newline(); wr.commentBlock("------ End PBXFileReference section");

	//-------
	wr.newline();
	wr.newline(); wr.commentBlock("----- PBXFrameworksBuildPhase -----------------");
	wr.newline(); wr.commentBlock(" Product Group ");
	{
		auto scope = wr.objectScope( build_phase_frameworks_uuid);
		wr.member("isa", "PBXFrameworksBuildPhase");
		{
			auto scope = wr.objectScope("file");
		}
		wr.member("runOnlyForDeploymentPostprocessing", "0");
	}
}

void Generator_xcode::gen_project_PBXGroup(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.newline(); wr.commentBlock("------ Begin PBXGroup section");
	
	{
		wr.newline(); wr.commentBlock("------ Folder Product");
		auto scope = wr.objectScope(product_group_uuid);
		wr.member("isa", "PBXGroup");
		{
			auto scope = wr.arrayScope("children");
			if (proj.hasOutputTarget) {
				wr.write(proj.genData_xcode.targetProductUuid);
			}
		}
		wr.member("sourceTree", kSourceTreeGroup);
		wr.member("name", "_products_");
	}

	auto* root = proj.virtualFolders.root;

	for (auto& v : proj.virtualFolders.dict) {
		if (&v == proj.virtualFolders.root) continue;
		wr.newline();
		wr.commentBlock(v.path);
		{
			auto scope = wr.objectScope(v.genData_xcode.uuid);
			wr.member("isa", "PBXGroup");
			{
				auto scope = wr.arrayScope("children");
				for (auto& c : v.children) {
					wr.write(c->genData_xcode.uuid);
				}
				for (auto& f : v.files) {
					wr.write(f->genData_xcode.uuid);
				}
			}

			auto basename = Path::basename(v.path, true);
			wr.member("name", quoteString(basename));
						
			if (v.parent == root) {
				wr.member("sourceTree", "SOURCE_ROOT");
				String rel;
				Path::getRel(rel, v.diskPath, g_ws->buildDir);
				wr.member("path", quoteString(rel));
			}else{
				wr.member("sourceTree", kSourceTreeGroup);
				wr.member("path", quoteString(basename));
			}
		}
	}

	{
		auto& v = *proj.virtualFolders.root;
		auto scope = wr.objectScope(main_group_uuid);
		wr.member("isa", "PBXGroup");
		{
			auto scope = wr.arrayScope("children");
			wr.write(product_group_uuid);
			wr.write(dependencies_group_uuid);

			for (auto& c : v.children) {
				wr.write(c->genData_xcode.uuid);
			}
			for (auto& f : v.files) {
				wr.write(f->genData_xcode.uuid);
			}
		}
		
		wr.member("sourceTree", "SOURCE_ROOT");
		String rel;
		Path::getRel(rel, proj.axprojDir, g_ws->buildDir);
		wr.member("path", quoteString(rel));
		wr.member("name", "MainGroup");
	}
	
	wr.newline();
	wr.newline(); wr.commentBlock("------ End PBXGroup section");
}


void Generator_xcode::gen_project_PBXProject(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.newline(); wr.commentBlock("------ Begin PBXProject section");

	auto scope = wr.objectScope(proj.genData_xcode.uuid);
	wr.member("isa", "PBXProject");
	{
		auto scope = wr.objectScope("attributes");
		{
			auto scope = wr.objectScope("TargetAttributes");
			{
				auto scope = wr.objectScope(proj.genData_xcode.targetUuid);
				wr.member("CreatedOnToolsVersion", "7.3.1");
				wr.member("ProvisioningStyle", "Automatic");
			}
		}
	}
	wr.member("compatibilityVersion", quoteString("Xcode 3.2"));
	wr.member("developmentRegion", "English");
	wr.member("hasScannedForEncodings", "0");
	wr.member("knownRegions", "(en,)");
	wr.member("buildConfigurationList", proj.genData_xcode.configListUuid);
	wr.member("mainGroup", main_group_uuid);
	wr.member("productRefGroup", product_group_uuid );
	wr.member("projectDirPath", quoteString(""));
	wr.member("projectRoot", quoteString(""));
	{
		auto scope = wr.arrayScope("targets");
		wr.write(proj.genData_xcode.targetUuid);
	}
	
	wr.newline(); wr.commentBlock("------ End PBXProject section");
}

void Generator_xcode::gen_project_PBXSourcesBuildPhase(XCodePbxWriter& wr, Project& proj) {
	wr.newline(); wr.commentBlock("------ PBXSourcesBuildPhase section");
	{
		auto scope = wr.objectScope(build_phase_sources_uuid);
		{
			wr.member("isa", "PBXSourcesBuildPhase");
			wr.member("buildActionMask", "2147483647");
			wr.member("runOnlyForDeploymentPostprocessing", "0");
		
			{
				auto scope = wr.arrayScope("files");
				for (auto& f : proj.fileEntries) {
					if (f.excludedFromBuild) continue;
					wr.write(f.genData_xcode.buildUuid);
				}
			}
		}
	}
}

void Generator_xcode::gen_project_PBXNativeTarget(XCodePbxWriter& wr, Project& proj) {		StrView productType;
	if (!proj.hasOutputTarget)
		return;

	if (proj.type_is_exe()) {
		if (proj.input.gui_app || g_ws->os == "ios") { // iOS doesn't have command line tools
			productType = "com.apple.product-type.application";
		}else{
			productType = "com.apple.product-type.tool";
		}
	}else if (proj.type_is_dll()) {
		productType = "com.apple.product-type.library.dynamic";
		
	}else if (proj.type_is_lib()){
		productType = "com.apple.product-type.library.static";
		
	}else{
		throw Error("Unsupported porject type ", proj.input.type);
	}


	wr.newline(); wr.commentBlock("--------- PBXNativeTarget section");
	{
		wr.newline(); wr.commentBlock("--------- File Reference");
		auto scope = wr.objectScope(proj.genData_xcode.targetProductUuid);
		
		StrView explicitFileType;
		if (proj.type_is_lib()) {
			explicitFileType = "archive.ar";
		}else{
			explicitFileType = "compiled.mach-o.executable";
		}
		
		wr.member("isa", "PBXFileReference");
		wr.member("explicitFileType", explicitFileType);
		wr.member("includeInIndex", "0");
		
		auto targetBasename = Path::basename(proj.defaultConfig().outputTarget.path(), true);
		wr.member("path", quoteString(targetBasename));
		wr.member("sourceTree", "BUILT_PRODUCTS_DIR");
	}
	
	{
		wr.newline(); wr.commentBlock("--------- Target");
		auto scope = wr.objectScope(proj.genData_xcode.targetUuid);
		wr.member("isa",			"PBXNativeTarget");
		wr.member("name",		quoteString(proj.name));
		wr.member("productName", quoteString(proj.name));
		wr.member("productReference", proj.genData_xcode.targetProductUuid);
		wr.member("productType", productType);
		wr.member("buildConfigurationList", proj.genData_xcode.targetConfigListUuid);
		{
			auto scope = wr.arrayScope("buildPhases");
			wr.write(build_phase_sources_uuid);
			wr.write(build_phase_frameworks_uuid);
			wr.write(build_phase_headers_uuid);
		}
		{
			auto scope = wr.arrayScope("buildRules");
		}
		{
			auto scope = wr.arrayScope("dependencies");
			if (proj.type_is_exe_or_dll()) {
				for (auto &dp : proj._dependencies_inherit) {
					wr.write(dp->genData_xcode.dependencyTargetUuid);
				}
			}
		}
	}
}

void Generator_xcode::gen_project_XCBuildConfiguration(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.commentBlock("----- XCBuildConfiguration ---------------");
	wr.newline();
	
	for (auto& config : proj.configs) {
		wr.newline();
		wr.commentBlock(String("Project Conifg [", config.name, "]"));
		{
			auto scope = wr.objectScope(config.genData_xcode.projectConfigUuid);
			wr.member("isa", "XCBuildConfiguration");
			{
				auto scope = wr.objectScope("buildSettings");
				//cc_flags
			}
			wr.member("name", config.name);
		}
	}
	
	for (auto& config : proj.configs) {
		wr.newline();
		wr.commentBlock(String("Target Conifg [", config.name, "]"));
		{
			auto scope = wr.objectScope(config.genData_xcode.targetConfigUuid);
			wr.member("isa", "XCBuildConfiguration");
			{
				auto scope = wr.objectScope("buildSettings");
				//link_flags
				String targetDir, targetBasename, targetExt;

				auto& outputTarget = config.outputTarget.path();
				targetDir		= Path::dirname(outputTarget);
				targetBasename	= Path::basename(outputTarget, false);
				targetExt		= Path::extension(outputTarget);
				
				
				wr.member("PRODUCT_NAME",					quoteString(targetBasename));
				wr.member("EXECUTABLE_PREFIX",				quoteString(""));
				wr.member("EXECUTABLE_EXTENSION",			quoteString(targetExt));
				wr.member("CONFIGURATION_BUILD_DIR",		quoteString(targetDir));
				wr.member("CONFIGURATION_TEMP_DIR",			quoteString(config._build_tmp_dir.path()));
				wr.member("STRIP_STYLE",					quoteString("all"));
				wr.member("DEAD_CODE_STRIPPING",			"YES");
				
				if (g_ws->os == "ios") {
					wr.member("SDKROOT",					quoteString("iphoneos"));
					wr.member("SUPPORTED_PLATFORMS",		quoteString("iphonesimulator iphoneos"));
					wr.member("VALID_ARCHS",				quoteString("arm64 armv7 armv7s"));
					wr.member("IPHONEOS_DEPLOYMENT_TARGET",	quoteString("10.1"));
				}else if (g_ws->os == "macosx"){
					wr.member("SDKROOT",					quoteString("macosx"));
					wr.member("SUPPORTED_PLATFORMS",		quoteString("macosx"));
					wr.member("MACOSX_DEPLOYMENT_TARGET",	quoteString("10.10"));
				}else{
					throw Error("Unsupported OS type ", g_ws->os);
				}
				
				wr.member("CLANG_CXX_LANGUAGE_STANDARD",	quoteString("c++11"));
				wr.member("GCC_SYMBOLS_PRIVATE_EXTERN",		"YES");
				wr.member("CLANG_ENABLE_OBJC_ARC",			"YES");
				
				if (config.isDebug) {
					wr.member("DEBUG_INFORMATION_FORMAT",		"dwarf");
					wr.member("GCC_GENERATE_DEBUGGING_SYMBOLS",	"YES");

					// 0: None[-O0], 1: Fast[-O1],  2: Faster[-O2], 3: Fastest[-O3], s: Fastest, Smallest[-Os], Fastest, Aggressive Optimizations [-Ofast]
					wr.member("GCC_OPTIMIZATION_LEVEL",			"0");
					wr.member("ONLY_ACTIVE_ARCH",				"YES");
					wr.member("ENABLE_TESTABILITY",				"YES");
					
				}else{
					wr.member("DEBUG_INFORMATION_FORMAT",		"dwarf-with-dsym");
					wr.member("GCC_GENERATE_DEBUGGING_SYMBOLS",	"NO");
					
					// 0: None[-O0], 1: Fast[-O1],  2: Faster[-O2], 3: Fastest[-O3], s: Fastest, Smallest[-Os], Fastest, Aggressive Optimizations [-Ofast]
					wr.member("GCC_OPTIMIZATION_LEVEL",			"s");

					wr.member("ONLY_ACTIVE_ARCH",				"YES");
					wr.member("ENABLE_TESTABILITY",				"YES");
				}
				
				
				{
					auto scope = wr.arrayScope("GCC_PREPROCESSOR_DEFINITIONS");
					for (auto& q : config.cpp_defines._final) {
						wr.newline();
						wr.write(quoteString(q.path()));
					}
				}
				{
					auto scope = wr.arrayScope("HEADER_SEARCH_PATHS");
					for (auto& q : config.include_dirs._final) {
						wr.newline();
						wr.write(quoteString2(q.path()));
					}
				}
				{
					auto scope = wr.arrayScope("OTHER_CPLUSPLUSFLAGS");
					for (auto& q : config.cpp_flags._final) {
						wr.newline();
						wr.write(quoteString(q.path()));
					}
				}

				{
					auto scope = wr.arrayScope("OTHER_LDFLAGS");
					for (auto& q : config.link_flags._final) {
						wr.newline();
						wr.write(quoteString(q.path()));
					}
					for (auto& q : config.link_files._final) {
						wr.newline();
						wr.write(quoteString2(q.path()));
					}
				}
				
				// warning flags
				
				wr.member("CLANG_WARN_BOOL_CONVERSION",								"YES");
				wr.member("CLANG_WARN_CONSTANT_CONVERSION", 						"YES");
				wr.member("CLANG_WARN_EMPTY_BODY",									"YES");
				wr.member("CLANG_WARN_ENUM_CONVERSION",								"YES");
				wr.member("CLANG_WARN_INFINITE_RECURSION",							"YES");
				wr.member("CLANG_WARN_INT_CONVERSION",								"YES");
				wr.member("CLANG_WARN_SUSPICIOUS_MOVE",								"YES");
				wr.member("CLANG_WARN_UNREACHABLE_CODE",							"YES");
				wr.member("CLANG_WARN__DUPLICATE_METHOD_MATCH", 					"YES");
				
				wr.member("GCC_WARN_FOUR_CHARACTER_CONSTANTS",						"YES");
				wr.member("CLANG_WARN_IMPLICIT_SIGN_CONVERSION",					"YES");
				wr.member("GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED",				"YES");
				wr.member("GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS",				"YES");
			//	wr.member("GCC_WARN_ABOUT_MISSING_PROTOTYPES",						"YES");
				wr.member("CLANG_WARN_ASSIGN_ENUM",									"YES");
				wr.member("GCC_WARN_SIGN_COMPARE",									"YES");
				wr.member("CLANG_WARN_SUSPICIOUS_IMPLICIT_CONVERSION",				"YES");
				wr.member("GCC_TREAT_INCOMPATIBLE_POINTER_TYPE_WARNINGS_AS_ERRORS",	"YES");
				wr.member("GCC_TREAT_IMPLICIT_FUNCTION_DECLARATIONS_AS_ERRORS",		"YES");
				wr.member("GCC_WARN_UNUSED_LABEL",									"YES");
				
				//-----------
				if (proj.type_is_exe_or_dll()) {
					if (proj.input.gui_app) {
						wr.member("PRODUCT_BUNDLE_IDENTIFIER",	quoteString(proj.input.xcode_bundle_identifier));
						wr.member("INFOPLIST_FILE",				quoteString(proj.genData_xcode.info_plist_file));
					}
				}
				
				if (proj.pch_header) {
					wr.member("GCC_PREFIX_HEADER", quoteString(proj.pch_header->path()));
					wr.member("GCC_PRECOMPILE_PREFIX_HEADER", "YES");
				}
				
				for (auto& q : config.xcode_settings.pairs()) {
					wr.member(q.key, quoteString(*q.value));
				}
			}
			wr.member("name", config.name);
		}
	}
}

void Generator_xcode::gen_project_XCConfigurationList(XCodePbxWriter& wr, Project& proj) {
	wr.newline();
	wr.commentBlock("----- XCConfigurationList -----------------");
	{
		wr.newline(); wr.commentBlock("Build configuration list for PBXProject");
		auto scope = wr.objectScope(proj.genData_xcode.configListUuid);
		wr.member("isa", "XCConfigurationList");
		{
			auto scope = wr.arrayScope("buildConfigurations");
			for (auto& config : proj.configs) {
				wr.newline(); wr.commentBlock(config.name);
				wr.newline();
				wr.write(config.genData_xcode.projectConfigUuid);
			}
		}
		
		wr.member("defaultConfigurationIsVisible", "0");
		wr.member("defaultConfigurationName", g_ws->defaultConfigName());
	}
	{
		wr.newline(); wr.commentBlock("Build configuration list for PBXNativeTarget");
		auto scope = wr.objectScope(proj.genData_xcode.targetConfigListUuid);
		wr.member("isa", "XCConfigurationList");
		{
			auto scope = wr.arrayScope("buildConfigurations");
			for (auto& config : proj.configs) {
				wr.newline(); wr.commentBlock(config.name);
				wr.newline();
				wr.write(config.genData_xcode.targetConfigUuid);
			}
		}
		
		wr.member("defaultConfigurationIsVisible", "0");
		wr.member("defaultConfigurationName", g_ws->defaultConfigName());
	}
}

void Generator_xcode::gen_info_plist(Project& proj) {
	auto& gd = proj.genData_xcode;

	gd.info_plist_file.set(proj.name, "_info.plist");


	XmlWriter wr;
	wr.writeHeader();
	wr.writeDocType("plist", 
		"-//Apple//DTD PLIST 1.0//EN", 
		"http://www.apple.com/DTDs/PropertyList-1.0.dtd");

	{
		auto tag = wr.tagScope("plist");
		wr.attr("version", "1.0");
		{
			auto tag = wr.tagScope("dict");
			wr.tagWithBody("key", "CFBundleDevelopmentRegion");
			wr.tagWithBody("string", "en");

			wr.tagWithBody("key", "CFBundleDevelopmentRegion");
			wr.tagWithBody("string", "en");

			wr.tagWithBody("key", "CFBundleExecutable");
			wr.tagWithBody("string", "$(EXECUTABLE_NAME)");

			wr.tagWithBody("key", "CFBundleIconFile");
			wr.tagWithBody("string", "");
		
			wr.tagWithBody("key", "CFBundleIdentifier");
			wr.tagWithBody("string", proj.input.xcode_bundle_identifier); //# $(PRODUCT_BUNDLE_IDENTIFIER)

			wr.tagWithBody("key", "CFBundleInfoDictionaryVersion");
			wr.tagWithBody("string", "6.0");
		
			wr.tagWithBody("key", "CFBundleName");
			wr.tagWithBody("string", "$(PRODUCT_NAME)");

			wr.tagWithBody("key", "CFBundlePackageType");
			wr.tagWithBody("string", "APPL");

			wr.tagWithBody("key", "CFBundleShortVersionString");
			wr.tagWithBody("string", "1.0");

			wr.tagWithBody("key", "CFBundleVersion");
			wr.tagWithBody("string", "1");

			wr.tagWithBody("key", "LSMinimumSystemVersion");
			wr.tagWithBody("string", "$(MACOSX_DEPLOYMENT_TARGET)");

			wr.tagWithBody("key", "NSHumanReadableCopyright");
			wr.tagWithBody("string", "=== Copyright ===");

			wr.tagWithBody("key", "NSMainNibFile");
			wr.tagWithBody("string", "MainMenu");

			wr.tagWithBody("key", "NSPrincipalClass");
			wr.tagWithBody("string", "NSApplication");
		}
	}
	FileUtil::writeTextFile(String(g_ws->buildDir, gd.info_plist_file), wr.buffer());
}

void Generator_xcode::genUuid(String& o) {
	if (o) return;

	o.set("AEEEEEEE");
	_lastGenId.v64++;
	
	char tmp[100 + 1];
	snprintf(tmp, 100, "%02X%02X%02X%02X%02X%02X%02X%02X",
						_lastGenId.c[7],
						_lastGenId.c[6],
						_lastGenId.c[5],
						_lastGenId.c[4],
						_lastGenId.c[3],
						_lastGenId.c[2],
						_lastGenId.c[1],
						_lastGenId.c[0]);
	tmp[100] = 0;

	o.append(StrView_c_str(tmp));
}

String Generator_xcode::quoteString2(const StrView& v) {
	return quoteString(quoteString(v));
}

String Generator_xcode::quoteString(const StrView& v) {
	String o;
	o.append('\"');
	for (auto ch : v) {
		switch (ch) {
			case '\"':	o.append("\\\"");	break;
			case '\\':	o.append("\\\\");	break;
			default:	o.append(ch);		break;
		}
	}
	o.append('\"');
	return o;
}

void Generator_xcode::readCacheFile(const StrView& filename) {
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
				};
				
				reader.beginObject();
				while (!reader.endObject()) {
					#define ENTRY(T) reader.member(#T, proj->genData_xcode.T);
						ENTRY(uuid);
						ENTRY(targetUuid);
						ENTRY(targetProductUuid);
						ENTRY(configListUuid);
						ENTRY(targetConfigListUuid);
						ENTRY(dependencyProxyUuid);
						ENTRY(dependencyTargetUuid);
						ENTRY(dependencyTargetProxyUuid);
					#undef ENTRY
					
					if (reader.member("configs")) {
						reader.beginObject();
						while(!reader.endObject()) {
							String configName;
							reader.getMemberName(configName);
							auto* config = proj->configs.find(configName);
							if (!config) {
								reader.skipValue();
								continue;
							}
						
							reader.beginObject();
							while(!reader.endObject()) {
								reader.member("projectConfigUuid",	config->genData_xcode.projectConfigUuid);
								reader.member("targetUuid",			config->genData_xcode.targetUuid);
								reader.member("targetConfigUuid",	config->genData_xcode.targetConfigUuid);
							}
						}
					}
					
					if (reader.member("fileEntries")) {
						reader.beginObject();
						while(!reader.endObject()) {
							String path;
							reader.getMemberName(path);
							auto* f = proj->fileEntries.find(path);
							if (!f) {
								reader.skipValue();
								continue;
							}
							
							reader.beginObject();
							while(!reader.endObject()) {
								reader.member("uuid",		f->genData_xcode.uuid);
								reader.member("buildUuid",	f->genData_xcode.buildUuid);
							}
						}
					}
					
					if (reader.member("virtualFolders")) {
						reader.beginObject();
						while(!reader.endObject()) {
							String path;
							reader.getMemberName(path);
							auto* v = proj->virtualFolders.dict.find(path);
							if (!v) {
								reader.skipValue();
								continue;
							}
							reader.beginObject();
							while(!reader.endObject()) {
								reader.member("uuid", v->genData_xcode.uuid);
							}
						}
					}
				}
			}
			continue;
		}
	}
}

void Generator_xcode::writeCacheFile(const StrView& filename) {
	JsonWriter wr;
	{
		auto scope = wr.objectScope();
		wr.write("lastGenId", (double)_lastGenId.v64);
		{
			auto scope = wr.objectScope("projects");
			for (auto& proj : g_ws->projects) {
				auto scope = wr.objectScope(proj.name);
				#define ENTRY(T) do{ wr.write(#T, proj.genData_xcode.T); }while(false)
					ENTRY(uuid);
					ENTRY(targetUuid);
					ENTRY(targetProductUuid);
					ENTRY(configListUuid);
					ENTRY(targetConfigListUuid);
					ENTRY(dependencyProxyUuid);
					ENTRY(dependencyTargetUuid);
					ENTRY(dependencyTargetProxyUuid);
				#undef ENTRY

				{
					auto scope = wr.objectScope("configs");
					for (auto& f : proj.configs) {
						auto scope = wr.objectScope(f.name);
						wr.write("projectConfigUuid",	f.genData_xcode.projectConfigUuid);
						wr.write("targetUuid",			f.genData_xcode.targetUuid);
						wr.write("targetConfigUuid",	f.genData_xcode.targetConfigUuid);
					}
				}
				
				{
					auto scope = wr.objectScope("fileEntries");
					for (auto& f : proj.fileEntries) {
						auto scope = wr.objectScope(f.path());
						wr.write("uuid",		f.genData_xcode.uuid);
						wr.write("buildUuid",	f.genData_xcode.buildUuid);
					}
				}
				{
					auto scope = wr.objectScope("virtualFolders");
					for (auto& f : proj.virtualFolders.dict) {
						auto scope = wr.objectScope(f.path);
						wr.write("uuid", f.genData_xcode.uuid);
					}
				}
			}
		}
	}
	FileUtil::writeTextFile(filename, wr.buffer());
}
	
} //namespace
