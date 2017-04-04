//
//  ProjectGroup.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "ProjectGroup.h"
#include "Project.h"

namespace ax_gen {

ProjectGroup* ProjectGroupDict::getOrAddGroup(const StrView& path) {
	auto* v = dict.find(path);
	if (v) return v;
	
	v = dict.add(path);
	v->path = path;
	auto* p = getOrAddParent(path);
	v->parent = p;
	p->children.append(v);
	return v;
}

ProjectGroup* ProjectGroupDict::getOrAddParent(const StrView& path) {
	auto s = path.splitEndByChar('/');
	if (!s.second) {
		return root;
	}
	return getOrAddGroup(s.first);
}

ProjectGroupDict::ProjectGroupDict() {
	root = dict.add("");
	root->path = "<group_root>";
}

void ProjectGroupDict::add(Project& proj) {
	auto* p = getOrAddGroup(proj.input.group);
	proj.group = p;
	p->projects.append(&proj);
}
	
} //namespace
