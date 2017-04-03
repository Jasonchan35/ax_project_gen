#include "JsonWriter.h"

namespace ax_pjgen {

JsonWriter::~JsonWriter() {
	if (_level.size() != 0) {
		assert(false); //"JsonWriter end within scope"
	}
}

void JsonWriter::beginObject(const StrView& name) {
	memberName(name);
	beginObject();
}

void JsonWriter::beginObject() {
	writeComma();
	_buf.append('{');
	_commaNeeded = false;
	_level.append(LevelType::Object);
}

void JsonWriter::endObject() {
	newline(-1);
	_buf.append('}');
	_commaNeeded = true;

	if (_level.size() == 0) {
		throw Error("JsonWriter error endObject level");
	}
	if (_level.back() != LevelType::Object) {
		throw Error("JsonWriter error endObject");
	}
	_level.popBack();

}

void JsonWriter::beginArray(const StrView& name) {
	memberName(name);
	beginArray();
}

void JsonWriter::beginArray() {
	writeComma();
	_buf.append('[');
	_commaNeeded = false;
	_level.append(LevelType::Array);
}

void JsonWriter::endArray() {
	_buf.append(']');
	_commaNeeded = true;

	if (_level.size() == 0) {
		throw Error("JsonWriter error endArray level");
	}
	if (_level.back() != LevelType::Array) {
		throw Error("JsonWriter error endArray");
	}
	_level.popBack();
}

void JsonWriter::memberName(const StrView& name) {
	if (_level.size() == 0 || _level.back() != LevelType::Object) {
		throw Error("JsonWriter member must inside object scope");
	}

	writeComma();
	newline();
	quoteString(name);
	_buf.append(':');
	_commaNeeded = false;
}

void JsonWriter::write(const StrView& value) {
	writeComma();
	quoteString(value);
}

void JsonWriter::write(bool value) {
	writeComma();
	_buf.append(value ? StrView("true") : StrView("false"));
}

void JsonWriter::write(int value) {
	writeComma();

	const int kTempBufSize = 100;
	char tmp[kTempBufSize+1];
	snprintf(tmp, kTempBufSize, "%d", value);
	tmp[kTempBufSize] = 0;

	_buf.append(StrView_c_str(tmp));
}

void JsonWriter::write(double value) {
	writeComma();

	const int kTempBufSize = 100;
	char tmp[kTempBufSize+1];
	snprintf(tmp, kTempBufSize, "%.18g", value);
	tmp[kTempBufSize] = 0;

	_buf.append(StrView_c_str(tmp));
}

void JsonWriter::writeComma()
{
	if (_commaNeeded)
		_buf.append(", ");
	_commaNeeded = true;
}

void JsonWriter::quoteString(const StrView& v) {
	_buf.append('\"');

	const char* p = v.data();
	const char* end = p + v.size();
	for (; *p && p < end; p++) {
		char ch = *p;

		if (ch >= 0 && ch <= 0x1F) {
			const int kTempBufSize = 100;
			char tmp[kTempBufSize+1];
			snprintf(tmp, kTempBufSize, "\\u%04x", ch);
			tmp[kTempBufSize] = 0;

			_buf.append(StrView_c_str(tmp));
			continue;
		}

		switch (ch) {
			case '/':	ax_fallthrough
			case '\\':	ax_fallthrough
			case '\"':	_buf.append('\\', ch);	break;
			case '\b':	_buf.append("\\b");		break;
			case '\f':	_buf.append("\\f");		break;
			case '\n':	_buf.append("\\n");		break;
			case '\r':	_buf.append("\\r");		break;
			case '\t':	_buf.append("\\t");		break;
			default:	_buf.append(ch);		break;
		}
	}
	_buf.append('\"');
}

void JsonWriter::newline(int offset) {
	if (_newlineNeeded) {
		_buf.append('\n');
		int n = _level.size() + offset;
		for(int i = 0; i < n; i++) {
			_buf.append("  ");
		}
	}
}

JsonWriter::ObjectScope::ObjectScope(JsonWriter* p)  {
	_p = p;
}

JsonWriter::ObjectScope::ObjectScope(ObjectScope && rhs)  {
	_p = rhs._p;
	rhs._p = nullptr;
}

JsonWriter::ObjectScope::~ObjectScope() {
	if (_p) { _p->endObject(); _p = nullptr; }
}

JsonWriter::ArrayScope::ArrayScope(JsonWriter* p)  {
	_p = p;
}

JsonWriter::ArrayScope::ArrayScope(ArrayScope && rhs)  {
	_p = rhs._p;
	rhs._p = nullptr;
}

JsonWriter::ArrayScope::~ArrayScope() {
	if (_p) { _p->endArray(); _p = nullptr; }
}


} //namespace
