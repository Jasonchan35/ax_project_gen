#include "StrView.h"

namespace ax_gen {


std::ostream& StrView::onStreamOut(std::ostream& s) const {
	const int n = 512;
	char tmp[n+1];
	if (_size < n) {
		memcpy(tmp, _data, (size_t)_size);
		tmp[_size] = 0;
		s << tmp;
	}else{
		String tmp = *this;
		s << tmp.c_str();
	}
	return s;
}

bool StrView::equals(const StrView& rhs, bool ignoreCase) const {
	if (!ignoreCase) return operator==(rhs);

	if (_size != rhs._size) return false;
	auto* a = data();
	auto* b = rhs.data();
	for (int i = 0; i < _size; i++) {
		if (std::tolower(*a) != std::tolower(*b)) return false;
	}
	return true;
}

StrView StrView::getFromPrefix(const StrView& prefix) {
	if (prefix._size <= _size) {
		if (strncmp(_data, prefix._data, (size_t)prefix._size) == 0) {
			return sliceFrom(prefix._size);
		}
	}
	return StrView();
}

StrView StrView::sliceFrom(int from) const {
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
	return SplitResult(slice(0, index), sliceFrom(index + 1));
}

StrView::SplitResult StrView::splitByCharInList(const StrView& charList) const {
	auto index = indexOfAnyCharIn(charList);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), sliceFrom(index + 1));
}

StrView::SplitResult StrView::splitEndByChar(char ch) const {
	auto index = lastIndexOfChar(ch);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), sliceFrom(index + 1));
}

StrView::SplitResult StrView::splitEndByAnyCharIn(const StrView& charList) const {
	auto index = lastIndexOfAnyCharIn(charList);
	if (index < 0) {
		return SplitResult(*this, StrView());
	}
	return SplitResult(slice(0, index), sliceFrom(index + 1));
}

bool StrView::matchWildcard(const StrView& wildcard, bool ignoreCase) const {
	const auto* p = data();
	const auto* e = end();

	const auto* w = wildcard.begin();
	const auto* wEnd = wildcard.end();

	while (p < e && w < wEnd) {
		if (*w == '?') {
			p++;
			w++;
			continue;
		}

		if (ignoreCase) {
			if (std::tolower(*w) == std::tolower(*p)) {
				p++;
				w++;
				continue;
			}
		}else{
			if (*w == *p) {
				p++;
				w++;
				continue;
			}
		}
		
		if (*w == '*') {
			auto w1 = w + 1;
			if (w1 >= wEnd) return true; // * is the last

			auto p1 = p + 1;
			if (p1 >= e) return false;

			p = p1;
			if (*p1 == *w1) {
				w = w1; //next wildcard
			}

			continue;
		}
		
		return false;
	}

	if (p == e && w == wEnd) return true;

	return false;
}


} //namespace
