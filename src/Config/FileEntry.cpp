//
//  FileEntry.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "FileEntry.h"
#include "Workspace.h"

namespace ax_gen {
	
void FileEntry::init(const StrView& absPath, bool isAbs, bool isGenerated) {
	_absPath = absPath;
	generated = isGenerated;

	if (isAbs) {
		_path = absPath;
	}else{
		Path::getRel(_path, absPath, g_ws->buildDir);
	}

	auto ext = Path::extension(_path);

	if (ext == "h" || ext == "hpp") {
		_type = FileType::cpp_header;
	}else if (ext == "cpp" || ext == "cc" || ext == "cxx") {
		_type = FileType::cpp_source;
		excludedFromBuild = false;
	}else if (ext == "c") {
		_type = FileType::c_source;
		excludedFromBuild = false;
	}else if (ext == "cuh") {
		_type = FileType::cu_header;
		excludedFromBuild = false;
	}else if (ext == "cu") {
		_type = FileType::cu_source;
		excludedFromBuild = false;
	}
}

FileEntry* FileEntryDict::add(const StrView& path, const StrView& fromDir, bool isGenerated) {
	String key;
	bool isAbs = true;

	if (!fromDir) {
		key = path;
	}else{
		isAbs = Path::isAbs(path);
		if (isAbs) {
			key = path;
		}else{
			Path::makeFullPath(key, fromDir, path);
		}
	}

	auto* e = _dict.find(key);
	if (!e) {
		e = _dict.add(key);
	}

	e->init(key, isAbs, isGenerated);
	return e;
}


} //namespace
