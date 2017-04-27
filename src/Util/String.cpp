#include "../common.h"
#include "String.h"
#include "UtfUtil.h"

namespace ax_gen {

void String::replaceChars(char from, char to) {
	int n = size();
	for (int i = 0; i < n; i++) {
		auto& ch = _p[(size_t)i];
		if ( ch == from) ch = to;
	}
}

void String::_append(int v) {
	char buf[63+1];
	snprintf(buf, 63, "%d", v);
	buf[63] = 0;
	_append(StrView_c_str(buf));
}

void String::_append(double v) {
	char buf[63+1];
	snprintf(buf, 63, "%f", v);
	buf[63] = 0;
	_append(StrView_c_str(buf));
}

void String::appendUtf(const wchar_t* wc, int wc_len) {
	int oldSize = size();
	int n = UtfUtil::getConvertedCount<wchar_t, char>(wc, wc_len);

	int newSize = oldSize + n;

	Vector<char> buf;
	buf.resize(newSize);

	UtfUtil::convert(buf.data(), buf.size(), wc, wc_len);
	_append( StrView(buf.data(), buf.size()) );
}

void String::appendUtf(const WString& w) {
	appendUtf(w.c_str(), w.size());
}

void String::setQuoted(StrView v) {
	clear();
	append('\"');
	for (auto ch : v) {
		switch (ch) {
			case '\"':	append("\\\"");	break;
			case '\\':	append("\\\\");	break;
			default:	append(ch);		break;
		}
	}
	append('\"');
}

void WString::appendUtf(const StrView& v) {
	int oldSize = size();
	int n = UtfUtil::getConvertedCount<char, wchar_t>(v.data(), v.size());

	int newSize = oldSize + n;
	
	Vector<wchar_t> buf;
	buf.resize(newSize);

	UtfUtil::convert(buf.data(), buf.size(), v.data(), v.size());	
	_p.append(buf.data(), (size_t)buf.size());
}

} //namespace
