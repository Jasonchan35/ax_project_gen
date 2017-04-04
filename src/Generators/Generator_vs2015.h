#pragma once

#include "Generator.h"

namespace ax_gen {

class Generator_vs2015 : public Generator {
public:
	Generator_vs2015();

	void generate() override;

private:

	void gen_workspace();
	void gen_vcxproj_filters(Project& proj);

	void gen_project(Project& proj);
	void gen_project_files( XmlWriter& wr, Project& proj);
	void gen_project_pch( XmlWriter& wr, Project& proj);

	void gen_project_config(XmlWriter& wr, Project& proj, Config& config);
	void gen_config_option(XmlWriter& wr, const StrView& name, Vector<String>& value);

	void readCacheFile(const StrView& filename);
	void writeCacheFile(const StrView& filename);

	void genUuid(String& outStr);

	String vcxproj_cpu;

	class GenId {
	public:
		GenId() { v64 = 0; }
		union {
			struct {
				uint32_t	a;
				uint32_t	b;
			};
			uint64_t v64;
		};
	};
	GenId _lastGenId;
};

} //namespace
