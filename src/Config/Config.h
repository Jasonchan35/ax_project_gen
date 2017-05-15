#pragma once

#include "../common.h"
#include "FileEntry.h"

namespace ax_gen {

class Workspace;
class Project;

class Config {
public:
	Config();

	void readJson(JsonReader& r);
	bool _readSettings(JsonReader& r);

	void init(Project* proj, Config* source, StrView name_);
	void _init_xcode_settings();
	void _init_vs2015_settings();

	String	name;

	bool			isDebug {false};

	bool			warning_as_error {false};
	String			warning_level;

	Project*		_project {nullptr};
	FileEntry		outputTarget;
	FileEntry		_build_tmp_dir;

	class Entry {
	public:
		void init(const StrView& path, bool isAbs);

		const String&	absPath		() const { return _absPath; }
		const String&	path		() const { return _path; }

	private:
		String		_absPath;
		String		_path; // absPath or relative path depends on user input, and relative path always relative to _build_dir
		bool		_isAbs {false};
	};

	class EntryDict {
		using Dict = StringDict<Entry>;
		using ValueIterator = Dict::ValueIterator;
	public:
		Entry*	add(const StrView& path, const StrView& fromDir = StrView());
		void	add(Entry& e);

		void extend(EntryDict& rhs);

		ValueIterator begin()	{ return _dict.begin(); }
		ValueIterator end()		{ return _dict.end(); }

		int size() const { return _dict.size(); }

	private:
		StringDict<Entry>	_dict;
	};
	
	struct Setting {
		EntryDict	add;
		EntryDict	remove;
		EntryDict	localAdd;
		EntryDict	localRemove;
		EntryDict	_inherit;
		EntryDict	_final;

		bool readEntry(JsonReader& r);
		void inherit(Setting& rhs);
		void computeFinal();

		void dump(StringStream& s);

		template<typename S>
		void dumpEntries(S& s, EntryDict& dict, const StrView& suffix) {
			if (!dict.size()) return;
			s << std::setw(ax_dump_padding) << String(name, suffix) << " = ";
			int i = 0;
			for (auto& it : dict) {
				if (i > 0) s << "\n" << std::setw(ax_dump_padding + 3) << " ";
				s << it;
				i++;
			}
			s << "\n";
		}

		String name;
		bool isPath {false};

	private:
		void _readValue(JsonReader& r, EntryDict& v);

	};

	String		exe_target_prefix;
	String		exe_target_suffix;

	String		dll_target_prefix;
	String		dll_target_suffix;

	String		lib_target_prefix;
	String		lib_target_suffix;

	Setting		cpp_defines;
	Setting		cpp_flags;
	
	Setting		include_dirs;
	Setting		include_files;
	Setting		link_dirs;
	Setting		link_files;
	Setting		link_flags;
	Setting		disable_warning;

	Vector<Setting*>	_settings;

	StringDict<String>	xcode_settings;
	StringDict<String>	vs2015_ClCompile;
	StringDict<String>	vs2015_Link;
	
	void inherit(const Config& rhs);
	void computeFinal();

	void resolve();
	void dump(StringStream& s);

	bool _resolved {false};
		
	struct GenData_makefile {
		String		cpp_obj_dir;
	};
	GenData_makefile genData_makefile;
		
	struct GenData_xcode {
		String		projectConfigUuid;
		String		targetUuid;
		String		targetConfigUuid;
	};
	GenData_xcode genData_xcode;
};


inline
std::ostream& operator<<(std::ostream& s, const Config::Entry& v) {
	s << v.path();
	return s;
}

inline
std::ostream& operator<<(std::ostream& s, Config::EntryDict& v) {
	//s << "[";
	int i = 0;
	for (auto& it : v) {
		if (i > 0) s << "\n" << std::setw(33) << " ";
		s << it;
		i++;
	}
	//s << "]";
	return s;
}

} //namespace
