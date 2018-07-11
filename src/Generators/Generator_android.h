#pragma once

#include "Generator.h"

namespace ax_gen {
class Generator_android : public Generator {
public:
	void onInit		() override;
	void onGenerate	() override;
	void onBuild	() override;

private:
	void gen_workspace();
	void gen_project(Project& proj);
	void gen_AndroidManifest();
	void gen_Application_mk();
	void gen_dummy_main_cpp();

	String escapeString	(const StrView& v);
	String quotePath	(const StrView& v);

	String	pch_suffix;
};

} //namespace
