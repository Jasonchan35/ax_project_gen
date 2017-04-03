#include "../common.h"
#include "XmlWriter.h"

namespace ax_gen {

void ax_gen::XmlWriter::writeHeader() {
	_buf.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
}

void XmlWriter::writeDocType(const StrView& name, const StrView& publicId, const StrView& systemId) {
	newline();
	_buf.append("<!DOCTYPE ", name, " PUBLIC ");
	quoteString(publicId);
	_buf.append(" ");
	quoteString(systemId);
	_buf.append(">");
}

ax_gen::XmlWriter::TagScope XmlWriter::tagScope(const StrView& name) {
	beginTag(name);
	return TagScope(this);
}

void XmlWriter::beginTag(const StrView& name) {
	closeBeginTag();
	newline();
	_buf.append("<");
	_buf.append(name);
	_beginTag = true;

	_tags.append(name);
}

void XmlWriter::endTag() {
	if (_tags.size() == 0) {
		throw Error("");
	}

	if (_beginTag) {
		_buf.append("/>");
		_beginTag = false;
	}else{
		closeBeginTag();
		newline(-1);
		_buf.append("</", _tags.back(), ">");
	}

	_tags.popBack();
}

void XmlWriter::attr(const StrView& name, const StrView& value) {
	_buf.append(" ");
	_buf.append(name);
	_buf.append("=");
	quoteString(value);
}

void XmlWriter::body(const StrView& text) {
	closeBeginTag();
	newline();
	_buf.append(text);
}

void XmlWriter::newline(int offset) {
	if (_noNewline) return;

	_buf.append('\n');
	int n = _tags.size() + offset;
	for (int i=0; i<n; i++) {
		_buf.append("  ");
	}
}

void XmlWriter::tagWithBody(const StrView& tagName, const StrView& bodyText) {
	beginTag(tagName);
	_noNewline = true;
	body(bodyText);
	endTag();
	_noNewline = false;
}

void XmlWriter::tagWithBodyBool(const StrView& tagName, bool b) {
	tagWithBody(tagName, b ? StrView("true") : StrView("false"));
}

void XmlWriter::closeBeginTag() {
	if (!_beginTag) return;
	_buf.append(">");
	_beginTag = false;
}

void XmlWriter::quoteString(const StrView& v) {
	_buf.append('\"');

	const char* p = v.data();
	const char* end = p + v.size();
	for (; *p && p < end; p++) {
		char ch = *p;
		switch (ch) {
			case '\"':	_buf.append("&quot;");	break;
			case '\'':	_buf.append("&apos;");	break;
			case '<':	_buf.append("&lt;");	break;
			case '>':	_buf.append("&gt;");	break;
			case '&':	_buf.append("&amp;");	break;
			default:	_buf.append(ch);		break;
		}
	}
	_buf.append('\"');
}

XmlWriter::TagScope::TagScope(TagScope && rhs) {
	_wr = rhs._wr;
	rhs._wr = nullptr;
}

XmlWriter::TagScope::~TagScope() {
	if (_wr) {
		_wr->endTag();
		_wr = nullptr;
	}
}

} //namespace
