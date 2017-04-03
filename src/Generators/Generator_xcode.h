#pragma once

#include "Generator.h"
#include "XCodePbxWriter.h"

namespace ax_gen {

class Generator_xcode : public Generator {
public:
	Generator_xcode();

	void generate() override;

private:
	void gen_workspace();
	void gen_workspace_category(XmlWriter& wr, ProjectCategory& cat);

	void gen_project(Project& proj);
	void gen_project_dependencies			(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXBuildFile			(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXProject				(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXGroup				(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXSourcesBuildPhase	(XCodePbxWriter& wr, Project& proj);
	void gen_project_PBXNativeTarget		(XCodePbxWriter& wr, Project& proj);
	void gen_project_XCBuildConfiguration	(XCodePbxWriter& wr, Project& proj);
	void gen_project_XCConfigurationList	(XCodePbxWriter& wr, Project& proj);

	void gen_file_reference					(XCodePbxWriter& wr, Project& proj, FileEntry& f);


	void gen_info_plist(Project& proj);

	void genUuid(String& o);

	String quoteString (const StrView& v);
	String quoteString2(const StrView& v);

	int64_t _last_gen_uuid {0};

	static const StrView build_config_uuid;
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
