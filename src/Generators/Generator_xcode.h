#pragma once

#include "Generator.h"
#include "XCodePbxWriter.h"

namespace ax_gen {

class Generator_xcode : public Generator {
public:
	void onInit		() override;
	void onGenerate	() override;
	void onBuild	() override;
	void onIde		() override;

private:
	void gen_workspace();
	void gen_workspace_group(XmlWriter& wr, ProjectGroup& group);

	void gen_project(Project& proj);
	void gen_project_genUuid(Project& proj);
	void gen_project_dependencies			(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXBuildFile			(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXProject				(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXGroup				(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXSourcesBuildPhase	(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXNativeTarget		(XCodePbxWriter& wr, Project& proj);
	void gen_project_XCBuildConfiguration	(XCodePbxWriter& wr, Project& proj);
	void gen_project_XCConfigurationList	(XCodePbxWriter& wr, Project& proj);

	void gen_file_reference					(XCodePbxWriter& wr, Project& proj, FileEntry& f);

	void gen_info_plist_MacOSX(Project& proj);
	void gen_info_plist_iOS(Project& proj);

	void genUuid(String& o);

	String quoteString (const StrView& v);
	String quoteString2(const StrView& v);
	
	void readCacheFile(const StrView& filename);
	void writeCacheFile(const StrView& filename);	

	class GenId {
	public:
		GenId() { v64 = 0; }
		union {
			uint8_t  c[8];
			uint64_t v64;
		};
	};
	GenId _lastGenId;

	static const StrView build_phase_sources_uuid;
	static const StrView build_phase_frameworks_uuid;
	static const StrView build_phase_headers_uuid;

	static const StrView main_group_uuid;
	static const StrView product_group_uuid;
	static const StrView dependencies_group_uuid;
	
	static const StrView kSourceTreeGroup;
	static const StrView kSourceTreeAbsolute;
};

} //namespace
