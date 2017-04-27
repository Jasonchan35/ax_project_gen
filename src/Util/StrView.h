#pragma once

#include <string>
#include <cctype>
#include <assert.h>

namespace ax_gen {

class String;
class WString;

class StrView {
public:
	StrView() {}
	StrView(const char* data, int size) : _data(data), _size(size) {}

	StrView(const std::string& v) : _data(v.c_str()), _size((int)v.size()) {}

	template<int N>
	StrView(const char (&sz)[N]) : _data((const char*)sz), _size(N ? N-1 : 0) {}

	StrView	getFromPrefix(const StrView& prefix);

	StrView slice(int from) const;
	StrView slice(int from, int size) const;

	bool startsWith(const StrView& v) const;

	int indexOfChar			(char ch) const;
	int indexOfAnyCharIn	(const StrView& charList) const;

	int lastIndexOfChar		(char ch) const;
	int lastIndexOfAnyCharIn(const StrView& charList) const;

	using SplitResult = std::pair<StrView, StrView>;
	SplitResult splitByChar			(char ch) const;
	SplitResult splitByCharInList	(const StrView& charList) const;

	SplitResult splitEndByChar		(char ch) const;
	SplitResult splitEndByAnyCharIn	(const StrView& charList) const;

	std::ostream& onStreamOut(std::ostream& s) const;

	explicit operator bool() const { return _size != 0; }
	
	bool operator==(const StrView& rhs) const { 
		if (_size != rhs._size) return false;
		return strncmp(_data, rhs._data, (size_t)_size) == 0;
	}
	bool operator!=(const StrView& rhs) const { return !operator==(rhs); }

	bool equals(const StrView& rhs, bool ignoreCase) const;

	bool matchWildcard(const StrView& wildcard, bool ignoreCase) const;

	char	operator[](int i) const {
		if (i < 0 || i >= _size) {
			assert(false); // "out_of_range"
		}
		return _data[i];
	}
	
	const char*	 begin	() const { return _data; }
	const char*  end	() const { return _data + _size; }

	const char*  data	() const { return _data; }
		  int    size	() const { return _size; }

private:
	const char* _data {nullptr};
	      int   _size {0};
};

inline StrView StrView_c_str(const char* sz) { return StrView(sz, (int)strlen(sz)); }

class WStrView {
public:
	const wchar_t* _data {nullptr};
	      int      _size {0};
};


} //namespace
