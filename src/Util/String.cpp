#include "../common.h"
#include "String.h"

namespace ax_pjgen {

StrView StrView::getFromPrefix(const StrView& prefix) {
	if (prefix._size <= _size) {
		if (strncmp(_data, prefix._data, (size_t)prefix._size) == 0) {
			return slice(prefix._size);
		}
	}
	return StrView();
}

StrView StrView::slice(int from) const {
	if (from < 0) from = 0;
	return slice(from, _size - from);
}

StrView StrView::slice(int from, int size) const {
	if (from < 0) from = 0;
	if (from >= _size) return StrView();
	auto end = from + size;
	if (end > _size) end = _size;
	return StrView(_data + from, end - from);
}

bool StrView::startsWith(const StrView& v) const {
	auto n = v.size();
	if (n < v.size()) return false;
	auto t = slice(0, n);
	return t == v;
}

int StrView::indexOfChar(char ch) const {
	auto* end = _data + _size;
	for (auto* p = _data; p < end; p++) {
		if (*p == ch) {
			return (int)(p - _data);
		}
	}
	return -1;
}

int StrView::lastIndexOfChar(char ch) const {
	if (_size <= 0) return false;
	auto* end = _data + _size;
	for (auto* p = end - 1; p >= _data; p--) {
		if (*p == ch) {
			return (int)(p - _data);
		}
	}
	return -1;
}

int StrView::indexOfAnyCharIn(const StrView& charList) const {
	auto* end = _data + _size;
	for (auto* p = _data; p < end; p++) {
		if (charList.indexOfChar(*p) >= 0) {
			return (int)(p - _data);
		}
	}
	return -1;
}

int StrView::lastIndexOfAnyCharIn(const StrView& charList) const {
	if (_size <= 0) return false;
	auto* end = _data + _size;
	for (auto* p = end - 1; p >= _data; p--) {
		if (charList.lastIndexOfChar(*p) >= 0) {
			return (int)(p - _data);
		}
	}
	return -1;
}

StrView::SplitResult StrView::splitByChar(char ch) const {
	auto index = indexOfChar(ch);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), slice(index + 1));
}

StrView::SplitResult StrView::splitByCharInList(const StrView& charList) const {
	auto index = indexOfAnyCharIn(charList);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), slice(index + 1));
}

StrView::SplitResult StrView::splitEndByChar(char ch) const {
	auto index = lastIndexOfChar(ch);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), slice(index + 1));
}

StrView::SplitResult StrView::splitEndByAnyCharIn(const StrView& charList) const {
	auto index = lastIndexOfAnyCharIn(charList);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), slice(index + 1));
}

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

void String::appendUtf(const wchar_t* w, int size) {
	try{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
		auto a = c.to_bytes(w, w + size);
		_p.append(a.data(), a.size());
	}catch(std::exception e){
		throw Error("error convert wchar to utf8");
	}
}

void String::appendUtf(const WString& w) {
	appendUtf(w.c_str(), w.size());
}

void WString::appendUtf(const StrView& v) {
	try{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
		auto w = c.from_bytes(v.begin(), v.end());
		_p.append(w.data(), w.size()); 
	}catch(std::exception e){
		throw Error("error convert utf8 to wchar");
	}
}

} //namespace
