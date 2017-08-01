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
	cu_header,	// cuda header
	cu_source,	// cuda source
};

class VirtualFolder;

class FileEntry {
public:
	void init(const StrView& absPath, bool isAbs, bool isGenerated);
	bool excludedFromBuild {true};

	const String&	absPath		() const { return _absPath; }
	const String&	path		() const { return _path; }

	FileType		type		() const { return _type; }
	bool			type_is_c	() const { return _type == FileType::c_source; }
	bool			type_is_cpp	() const { return _type == FileType::cpp_source; }
	bool			type_is_c_or_cpp() const { return type_is_c() || type_is_cpp(); }

	struct GenData_xcode {
		String		uuid;
		String		buildUuid;
	};
	GenData_xcode genData_xcode;
	
	VirtualFolder* parent {nullptr};
	bool	generated = {false};

	explicit operator bool() const { return _absPath.operator bool(); }

private:
	String		_absPath;
	String		_path; // absPath or relative path depends on user input, and relative path always relative to _build_dir
	FileType	_type {FileType::None};
};

class FileEntryDict {
	using Dict = StringDict<FileEntry>;
	using ValueIterator = Dict::ValueIterator;
public:

	FileEntry*	add(const StrView& path, const StrView& fromDir, bool isGenerated);

	int size() const { return _dict.size(); }

	ValueIterator begin	() { return _dict.begin(); }
	ValueIterator end	() { return _dict.end(); }

	void remove(const StrView& key) { _dict.remove(key); }

	FileEntry* find(const StrView& key) { return _dict.find(key); }

private:
	StringDict<FileEntry> _dict;
};

inline
std::ostream& operator<<(std::ostream& s, const FileEntry& v) {
	s << v.path();
	return s;
}

inline
std::ostream& operator<<(std::ostream& s, FileEntryDict& v) {
	//s << "[";
	int i = 0;
	for (auto& it : v) {
		if (i > 0) s << "\n" << std::setw(ax_dump_padding + 3) << " ";
		s << it;
		i++;
	}
	//s << "]";
	return s;
}
	
} //namespace
