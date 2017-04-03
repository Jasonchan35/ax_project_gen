#pragma once

#include "../common.h"

namespace ax_gen {

class Workspace;
class Project;

class Config {
public:
	Config();

	void readJson(JsonReader& r);
	bool _readSettings(JsonReader& r);

	String	name;

	bool			isDebug {false};

	bool			warning_as_error {false};
	String			warning_level;

	Project*		_project {nullptr};
	String			outputTarget;
	String			_build_tmp_dir;
	
	struct Setting {
		Vector<String>	add;
		Vector<String>	remove;
		Vector<String>	localAdd;
		Vector<String>	localRemove;
		Vector<String>	_inherit;
		Vector<String>	_final;

		bool readItem(JsonReader& r);
		void inherit(const Setting& rhs);
		void computeFinal();

		void dump(StringStream& s);

		template<typename S, typename ITEM>
		void dumpItem(S& s, ITEM& item, const StrView& suffix) {
			if (item.size()) {
				ax_dump_(s, String(name, suffix), item);
			}
		}

		String name;
		bool isPath {false};

	private:
		void _readValue(JsonReader& r, Vector<String>& v);

	};

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

} //namespace
