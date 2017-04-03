//
//  ProjectCategory.hpp
//  ax_pjgen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

namespace ax_pjgen {

class Project;

class ProjectCategory {
public:
	String path;
	Vector<ProjectCategory*> children;
	Vector<Project*> projects;
	ProjectCategory* parent {nullptr};
	
	struct GenData_vs2015 {
		String		uuid;
	};
	GenData_vs2015 genData_vs2015;
};

class ProjectCategoryDict {
public:
	ProjectCategoryDict();
	void add(Project& proj);
	
	StringDict<ProjectCategory> dict;
	ProjectCategory* root {nullptr};

	ProjectCategory* getOrAddCategory(const StrView& path);	
private:
	ProjectCategory* getOrAddParent(const StrView& path);
};

} //namespace ax_pjgen
