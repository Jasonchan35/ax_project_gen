#include "XCodePbxWriter.h"

namespace ax_gen {

XCodePbxWriter::~XCodePbxWriter() {
	if (_level.size() != 0) {
		assert(false); //"XCodePbxWriter end within scope"
	}
}

void XCodePbxWriter::beginObject(const StrView& name) {
	memberName(name);
	beginObject();
}

void XCodePbxWriter::beginObject() {
	_buf.append('{');
	_level.append(LevelType::Object);
}

void XCodePbxWriter::endObject() {
	newline(-1);
	_buf.append("}");
	
	if (_level.size() == 0) {
		throw Error("XCodePbxWriter error endObject level");
	}
	if (_level.back() != LevelType::Object) {
		throw Error("XCodePbxWriter error endObject");
	}
	_level.popBack();
	writeTail();
}

void XCodePbxWriter::beginArray(const StrView& name) {
	memberName(name);
	beginArray();
}

void XCodePbxWriter::beginArray() {
	_buf.append("(");
	_level.append(LevelType::Array);
}

void XCodePbxWriter::endArray() {
	_buf.append(")");

	if (_level.size() == 0) {
		throw Error("XCodePbxWriter error endArray level");
	}
	if (_level.back() != LevelType::Array) {
		throw Error("XCodePbxWriter error endArray");
	}
	_level.popBack();
	writeTail();
}

void XCodePbxWriter::commentBlock(const StrView& s) {
	_buf.append(" /* ");
	_buf.append(s);
	_buf.append(" */ ");
}

void XCodePbxWriter::memberName(const StrView& name) {
	if (_level.size() == 0 || _level.back() != LevelType::Object) {
		throw Error("XCodePbxWriter member must inside object scope");
	}

	//writeComma();
	newline();
	_buf.append(name);
	_buf.append(" = ");
}

void XCodePbxWriter::write(const StrView& value) {
	_buf.append(value);
	writeTail();
}

void XCodePbxWriter::writeTail()
{
	if (!_level) return;
	if (_level.back() == LevelType::Array)
		_buf.append(",");
	
	if (_level.back() == LevelType::Object)
		_buf.append(";");
}

void XCodePbxWriter::quoteString(const StrView& v) {
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

void XCodePbxWriter::newline(int offset) {
	if (_newlineNeeded) {
		_buf.append('\n');
		int n = _level.size() + offset;
		for(int i = 0; i < n; i++) {
			_buf.append("  ");
		}
	}
}

XCodePbxWriter::ObjectScope::ObjectScope(XCodePbxWriter* p)  {
	_p = p;
}

XCodePbxWriter::ObjectScope::ObjectScope(ObjectScope && rhs)  {
	_p = rhs._p;
	rhs._p = nullptr;
}

XCodePbxWriter::ObjectScope::~ObjectScope() {
	if (_p) { _p->endObject(); _p = nullptr; }
}

XCodePbxWriter::ArrayScope::ArrayScope(XCodePbxWriter* p)  {
	_p = p;
}

XCodePbxWriter::ArrayScope::ArrayScope(ArrayScope && rhs)  {
	_p = rhs._p;
	rhs._p = nullptr;
}

XCodePbxWriter::ArrayScope::~ArrayScope() {
	if (_p) { _p->endArray(); _p = nullptr; }
}


} //namespace
