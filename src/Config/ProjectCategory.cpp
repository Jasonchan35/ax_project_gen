//
//  ProjectCategory.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "ProjectCategory.h"
#include "Project.h"

namespace ax_gen {

ProjectCategory* ProjectCategoryDict::getOrAddCategory(const StrView& path) {
	auto* v = dict.find(path);
	if (v) return v;
	
	v = dict.add(path);
	v->path = path;
	auto* p = getOrAddParent(path);
	v->parent = p;
	p->children.append(v);
	return v;
}

ProjectCategory* ProjectCategoryDict::getOrAddParent(const StrView& path) {
	auto s = path.splitEndByChar('/');
	if (!s.second) {
		return root;
	}
	return getOrAddCategory(s.first);
}

ProjectCategoryDict::ProjectCategoryDict() {
	root = dict.add("");
	root->path = "<category_root>";
}

void ProjectCategoryDict::add(Project& proj) {
	auto* p = getOrAddCategory(proj.input.category);
	proj.category = p;
	p->projects.append(&proj);
}
	
} //namespace
