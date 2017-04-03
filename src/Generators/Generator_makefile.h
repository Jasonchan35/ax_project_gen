#pragma once

#include "Generator.h"

namespace ax_pjgen {
class Generator_makefile : public Generator {
public:
	Generator_makefile();

	void generate() override;
	void build() override;

private:
	void gen_workspace();
	void gen_project(Project& proj);
	void gen_project_config(String& o, Config& config);

	void gen_helper_batch_file();
	void gen_common_var(String& o);

	String get_obj_file(Config& config, FileEntry& f);

	String escapeString(const StrView& v);
	String quoteString (const StrView& v);

	String	pch_suffix;
};

} //namespace
