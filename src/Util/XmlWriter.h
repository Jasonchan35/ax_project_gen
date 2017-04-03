#pragma once

#include "Vector.h"
#include "String.h"

namespace ax_gen {

class XmlWriter : public NonCopyable {
public:
	void writeHeader();
	void writeDocType(const StrView& name, const StrView& publicId, const StrView& systemId);

	class TagScope : public NonCopyable {
	public:
		TagScope(XmlWriter* wr) : _wr(wr) {}
		TagScope(TagScope && rhs);
		~TagScope();
	private:
		XmlWriter* _wr;
	};

	TagScope tagScope(const StrView& name);

	void beginTag(const StrView& name);
	void endTag();

	void attr(const StrView& name, const StrView& value);
	void body(const StrView& text);
	void newline(int offset = 0);

	void tagWithBody(const StrView& tagName, const StrView& bodyText);
	void tagWithBodyBool(const StrView& tagName, bool b);

	String&	buffer() { return _buf; }

private:

	void closeBeginTag();
	void quoteString(const StrView& v);

	String _buf;
	Vector<String>	_tags;

	bool _beginTag {false};
	bool _noNewline{false};
};

} //namespace
