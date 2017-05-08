#pragma once
#include "../common.h"
#include "String.h"

namespace ax_gen {

class JsonWriter {
public:
	~JsonWriter();

	class ObjectScope : public NonCopyable {
	public:
		ObjectScope(JsonWriter* p);
		ObjectScope(ObjectScope && rhs);
		~ObjectScope();
	private:
		JsonWriter* _p;
	};

	class ArrayScope : public NonCopyable {
	public:
		ArrayScope(JsonWriter* p);
		ArrayScope(ArrayScope && rhs);
		~ArrayScope();
	private:
		JsonWriter* _p;
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

	void memberName(const StrView& name);

	void write(int      value);
	void write(double   value);
	void write(bool     value);
	void write(const StrView& value);

	template<typename VALUE>
	void member(const StrView& name, const VALUE& value) {
		memberName(name);
		write(value);
	}

	String& buffer() { return _buf; }

	void writeComma();
	void newline(int offset = 0);
private:
	void quoteString(const StrView& v);

	enum class LevelType {
		Object,
		Array,
	};

	String	_buf;
	Vector<LevelType> _level;

	bool _commaNeeded	{false};
	bool _newlineNeeded {true};
};

} //namespace
