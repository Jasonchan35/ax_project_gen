#pragma once

#include "Generator.h"

namespace ax_gen {

class Generator_vs2015 : public Generator {
public:
	void onInit		() override;
	void onGenerate	() override;
	void onBuild	() override;
	void onIde		() override;

	virtual bool vsForLinux() { return false; }

	virtual StrView slnFileHeader();
	virtual StrView vcxprojToolsVersion() { return StrView("14.0"); }
	virtual StrView visualc_PlatformToolset() { return StrView("v140"); } // v140_xp doesn't work with CUDA' compiler
private:

	void gen_workspace(ExtraWorkspace& extraWorkspace);
	void gen_vcxproj_filters(Project& proj);

	void gen_project(Project& proj);
	void gen_project_files( XmlWriter& wr, Project& proj);
	void gen_project_pch( XmlWriter& wr, Project& proj);

	void gen_project_config(XmlWriter& wr, Project& proj, Config& config);
	void gen_config_option(XmlWriter& wr, const StrView& name, Config::EntryDict& value, const StrView& relativeTo = StrView());

	void readCacheFile(const StrView& filename);
	void writeCacheFile(const StrView& filename);

	StrView _visualc_PlatformToolset(Project& proj);

	void genUuid(String& outStr);

	String vcxproj_cpu;

	class GenId {
	public:
		GenId() { v64 = 0; }
		union {
			uint8_t  c[8];
			uint64_t v64;
		};
	};
	GenId _lastGenId;
};

class Generator_vs2015_linux : public Generator_vs2015 {
public:
	virtual bool vsForLinux() override { return true; }
};

class Generator_vs2017 : public Generator_vs2015 {
public:
	virtual StrView slnFileHeader();
	virtual StrView vcxprojToolsVersion() { return StrView("15.0"); }
	virtual StrView visualc_PlatformToolset() { return StrView("v141_xp"); }
};

class Generator_vs2017_linux : public Generator_vs2017 {
public:
	virtual bool vsForLinux() override { return true; }
};

} //namespace
