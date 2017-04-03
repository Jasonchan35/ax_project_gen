//
//  ProjectGroup.hpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

namespace ax_gen {

class Project;

class ProjectGroup {
public:
	String path;
	Vector<ProjectGroup*> children;
	Vector<Project*> projects;
	ProjectGroup* parent {nullptr};
	
	struct GenData_vs2015 {
		String		uuid;
	};
	GenData_vs2015 genData_vs2015;
};

class ProjectGroupDict {
public:
	ProjectGroupDict();
	void add(Project& proj);
	
	StringDict<ProjectGroup> dict;
	ProjectGroup* root {nullptr};

	ProjectGroup* getOrAddGroup(const StrView& path);	
private:
	ProjectGroup* getOrAddParent(const StrView& path);
};

} //namespace ax_gen
