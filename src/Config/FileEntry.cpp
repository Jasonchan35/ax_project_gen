//
//  FileEntry.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "FileEntry.h"

namespace ax_gen {
	
void FileEntry::init(const StrView& name) {
	_name = name;
	auto ext = Path::extension(name);

	if (ext == "h" || ext == "hpp") {
		_type = FileType::cpp_header;
	}else if (ext == "cpp" || ext == "cc" || ext == "cxx") {
		_type = FileType::cpp_source;
		excludedFromBuild = false;
	}else if (ext == "c") {
		_type = FileType::c_source;
		excludedFromBuild = false;
	}
}
	
} //namespace
