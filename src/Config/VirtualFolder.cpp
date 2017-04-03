//
//  VirtualFolder.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "VirtualFolder.h"
#include "FileEntry.h"

namespace ax_gen {

VirtualFolder* VirtualFolderDict::getOrAddParent(const StrView& baseDir, const StrView& path) {
	auto s = path.splitEndByChar('/');
	if (!s.second) {
		return root;
	}
	
	auto d = s.first;
	auto* v = dict.find(d);
	if (!v) {
		v = dict.add(d);
		v->path = d;
		
		Path::makeFullPath(v->diskPath, baseDir, d);
		
		auto* p = getOrAddParent(baseDir, d);
		v->parent = p;
		p->children.append(v);
	}
	return v;
}

VirtualFolderDict::VirtualFolderDict() {
	root = dict.add("");
	root->path = "<virtual_root>";
}

void VirtualFolderDict::add(const StrView& baseDir, FileEntry& file) {
	String rel;
	Path::getRel(rel, file.name(), baseDir);

	auto* p = getOrAddParent(baseDir, rel);
	file.parent = p;
	p->files.append(&file);
}
	
} //namespace
