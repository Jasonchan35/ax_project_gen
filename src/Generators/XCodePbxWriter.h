#pragma once

#include "../common.h"

namespace ax_gen {

class XCodePbxWriter {
public:
	~XCodePbxWriter();

	class ObjectScope : public NonCopyable {
	public:
		ObjectScope(XCodePbxWriter* p);
		ObjectScope(ObjectScope && rhs);
		~ObjectScope();
	private:
		XCodePbxWriter* _p;
	};

	class ArrayScope : public NonCopyable {
	public:
		ArrayScope(XCodePbxWriter* p);
		ArrayScope(ArrayScope && rhs);
		~ArrayScope();
	private:
		XCodePbxWriter* _p;
	};

	ObjectScope	objectScope(const StrView& name)	{ beginObject(name); return ObjectScope(this); }
	ObjectScope	objectScope()						{ beginObject();	 return ObjectScope(this); }

	ArrayScope	arrayScope(const StrView& name)		{ beginArray(name); return ArrayScope(this); }
	ArrayScope	arrayScope()						{ beginArray();		return ArrayScope(this); }

	void beginObject(const StrView& name);
	void beginObject();
	void endObject();

	void beginArray(const StrView& name);
	void beginArray();
	void endArray();

	void commentBlock(const StrView& s);

	void memberName(const StrView& name);

	void write(const StrView& value);

	template<typename VALUE>
	void _member(const StrView& name, VALUE&& value) {
		memberName(name);
		write(std::forward<VALUE>(value));
	}

	void member(const StrView& name, StrView value) { _member(name, value); }

	template<int N>
	void member(const StrView& name, const char (&sz)[N]) { _member(name, StrView(sz)); }	
	
	String& buffer() { return _buf; }

	void newline(int offset = 0);
	
private:
	void quoteString(const StrView& v);
	void writeTail();
	
	enum class LevelType {
		Object,
		Array,
	};

	String	_buf;
	Vector<LevelType> _level;

	bool _newlineNeeded {true};
};

} //namespace
