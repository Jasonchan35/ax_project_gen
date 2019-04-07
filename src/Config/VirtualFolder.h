//
//  VirtualFolder.hpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

#include "../common.h"

namespace ax_gen {

class FileEntry;

class VirtualFolder {
public:
	String path;
	String diskPath;
	
	Vector<VirtualFolder*> children;
	Vector<FileEntry*> files;
	VirtualFolder* parent {nullptr};
	
	struct GenData_xcode {
		String		uuid;
	};
	GenData_xcode genData_xcode;
	
	void sortTree() {
		files.sort(
			[](auto* a, auto* b){ return a->path() < b->path(); }
		);
		children.sort(
			[](auto* a, auto* b) { return a->path < b->path; }
		);
		
		for (auto* c : children) {
			c->sortTree();
		}
	}
};

class VirtualFolderDict {
public:
	VirtualFolderDict();

	void add(const StrView& baseDir, FileEntry& file);
	
	StringDict<VirtualFolder> dict;
	VirtualFolder* root {nullptr};

	void sort() {
		dict.sort();
		if (root) root->sortTree();
	}

private:
	VirtualFolder* getOrAddParent(const StrView& baseDir, const StrView& path);
};

} //namespace
