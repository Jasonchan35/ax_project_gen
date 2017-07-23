#pragma once

#include "../common.h"
#include "Config.h"
#include "FileEntry.h"
#include "ProjectGroup.h"
#include "VirtualFolder.h"

namespace ax_gen {

enum class ProjectType {
	None,
	c_headers, // headers only library
	cpp_headers,
	c_lib,
	c_exe,
	c_dll,
	cpp_lib,
	cpp_dll,
	cpp_exe,
};

class Project : public NonCopyable {
public:
	Project();

	void readFile(const StrView& filename);
	void readJson(JsonReader& r);

	void init(const StrView& name_);

	class Input {
	public:
		String				group;
		String				type;
		bool				gui_app {false};
		String				enable_cuda;

		String				pch_header;
		Vector<String>		dependencies;

		bool				multithread_build {true};
		bool				unite_build {false};
		int					unite_filesize {0};

		bool				cpp_as_objcpp {true};

		//---
		String				xcode_bundle_identifier;

		//---
		String				visualc_PlatformToolset;

		void dump(StringStream& s);
	};
	Input	input;

	bool	multithread_build() { return input.multithread_build; }

	ProjectGroup*			group {nullptr};

	FileEntryDict			fileEntries;
	
	int						_uniteFileCount {0};
	bool					hasOutputTarget {false};
	
	VirtualFolderDict		virtualFolders;


	FileEntry				pch_cpp; //only for msvc

	String				axprojFilename;
	String				axprojDir;

	String				name;

	ProjectType			type;
	bool				type_is_cpp();
	bool				type_is_c();
	bool				type_is_exe();
	bool				type_is_dll();
	bool				type_is_lib();
	bool				type_is_headers();
	bool				type_is_exe_or_dll() { return type_is_exe() || type_is_dll(); }

	StringDict<Config>	configs;
	Config&				defaultConfig();
	Config&				configToBuild();

	String				_generatedFileDir;
	Vector<Project*>	_dependencies;
	Vector<Project*>	_dependencies_inherit;

	FileEntry*			pch_header {nullptr};

	void dump(StringStream& s);

	void resolve();
	void resolve_internal();
	void resolve_files();
	void resolve_genUniteFiles(FileType targetType, const StrView& ext);
	void write_uniteFile(const StrView& code, const StrView& ext);

	bool _resolved  : 1;
	bool _resolving : 1;

	struct GenData_xcode {
		FileEntry	xcodeproj; //folder
		String		pbxproj;
		String		info_plist_file;

		String		uuid;
		String		targetUuid;
		String		targetProductUuid;
		
		String		configListUuid;
		String		targetConfigListUuid;
		
		String		dependencyProxyUuid;
		String		dependencyTargetUuid;
		String		dependencyTargetProxyUuid;
	};
	GenData_xcode genData_xcode;

	struct GenData_vs2015 {
		String		vcxproj;
		String		uuid;
	};
	GenData_vs2015 genData_vs2015;

	struct GenData_makefile {
		String		makefile;
	};
	GenData_makefile genData_makefile;
};

} //namespace
