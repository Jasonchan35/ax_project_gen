//
//  VirtualFolder.hpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

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
};

class VirtualFolderDict {
public:
	VirtualFolderDict();

	void add(const StrView& baseDir, FileEntry& file);
	
	StringDict<VirtualFolder> dict;
	VirtualFolder* root {nullptr};

private:
	VirtualFolder* getOrAddParent(const StrView& baseDir, const StrView& path);
};

} //namespace
