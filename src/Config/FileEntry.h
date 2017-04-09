//
//  FileEntry.hpp
//  ax_gen
//
//  Created by Jason on 2017-04-02.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

#include "../common.h"

namespace ax_gen {

enum class FileType {
	None,
	cpp_header, // c or cpp header
	cpp_source,
	c_source,
};

class VirtualFolder;

class FileEntry {
public:
	void init(const StrView& name_);
	bool excludedFromBuild {true};
	const String&	name() { return _name; }
	FileType		type() { return _type; }

	struct GenData_xcode {
		String		uuid;
		String		buildUuid;
	};
	GenData_xcode genData_xcode;
	
	VirtualFolder* parent {nullptr};
	bool	generated = {false};
private:
	String		_name;
	FileType	_type {FileType::None};
};
	
} //namespace
