#pragma once

#include <string>

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

	template<typename S> inline
	S& onStreamOut(S& s) const {
		return s.write(_data, _size);
	}

	explicit operator bool() const { return _size != 0; }
	
	bool operator==(const StrView& rhs) const { 
		if (_size != rhs._size) return false;
		return strncmp(_data, rhs._data, (size_t)_size) == 0;
	}

	bool operator!=(const StrView& rhs) const { return !operator==(rhs); }

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


class String {
public:
	String() {}
	String(const StrView& v) { operator=(v); }

	template<typename... ARGS>
 	explicit String(ARGS&&... args) {
		append(std::forward<ARGS>(args)...);
	}

	explicit String(const std::string& v) { operator=(StrView(v.c_str(), (int)v.size())); }

	void clear() { _p.clear(); }
	void set(const StrView& v)				{ clear(); append(v); }

	template<typename... ARGS>
	void set(ARGS&&... args) {
		clear();
		append(std::forward<ARGS>(args)...);
	}

	void setUtf(const wchar_t* w, int size)	{ clear(); appendUtf(w, size); }
	void setUtf(const WString& w)			{ clear(); appendUtf(w); }
	void appendUtf(const WString& w);
	void appendUtf(const wchar_t* w, int size);

	explicit operator bool() const		{ return size() > 0; }

	operator StrView() const			{ return StrView(_p.c_str(), (int)_p.size()); }

	void append() {}

	template<typename First, typename... ARGS> inline
	void append(First&& first, ARGS&&... args) {
		_append(std::forward<First>(first));
		append(std::forward<ARGS>(args)...);
	}

	void operator+=(char ch)			{ _append(ch); }
	void operator+=(const StrView& v)	{ _append(v);  }

	bool operator==(const StrView& v) const { 
		if (size() != v.size()) return false;
		return strncmp(c_str(), v.data(), (size_t)v.size()) == 0;
	}

	bool operator!=(const StrView& v) const { return !operator==(v); }

	const char* 	c_str	() const	{ return _p.c_str(); }
			int		size	() const	{ return (int)_p.size(); }

	bool operator<(const StrView& v) const {
		int c = strncmp(c_str(), v.data(), (size_t)v.size());
		if (c == 0) return size() < v.size();
		return c < 0;
	}

	void operator=(const StrView& v) { _p.assign(v.data(), (size_t)v.size()); }

	template<int N>
	void operator=(const char (&sz)[N]) { operator=( StrView(sz) ); }

	char operator[](int i) { return _p[(size_t)i]; }

	void resize(int n) { _p.resize((size_t)n); }
	void reserve(int n) { _p.reserve((size_t)n); }

	void replaceChars(char from, char to);

	StrView view() const { return StrView(_p.data(), (int)_p.size()); }

private:
	void _append(char    ch)						{ _p.push_back(ch); }
	void _append(int     v);
	void _append(double  v);
	void _append(const StrView& v)					{ _p.append(v.data(), (size_t)v.size()); }

	template<int N>
	void _append(const char (&v)[N])				{ _append(StrView(v)); }

	std::string	_p;
};

class WString {
public:
	void clear() { _p.clear(); }

	void setUtf(const StrView& v)		{ clear(); appendUtf(v); }
	void appendUtf(const StrView& v);

	void push_back(char ch) { return _p.push_back(ch); }

	const wchar_t*	c_str	() const	{ return _p.c_str(); }
			int		size	() const	{ return (int)_p.size(); }

	void reserve(int n) { _p.reserve((size_t)n); }

		wchar_t*	data	() { return &_p[0]; }

private:
	std::wstring _p;
};

inline std::ostream& operator<<(std::ostream& s, const StrView& v) { return v.onStreamOut(s); }
inline std::ostream& operator<<(std::ostream& s, const WString& v) { 
	String tmp;
	tmp.setUtf(v);
	return s << tmp;
}

template<typename S, typename Value> inline
void ax_dump_(S& s, const StrView& name, Value& value) {
	int padding = 30 - name.size();
	for (int i = 0; i<padding; i++) {
		s.put(' ');
	}
	s << name << " = " << value << '\n';
}

template<typename S> inline
void ax_dump_(S& s, const StrView& name, const char* sz) {
	ax_dump_<S, const char*>(s, name, sz);
}

template<typename S> inline
void ax_dump_(S& s, const StrView& name, bool b) {
	ax_dump_(s, name, b ? "true" : "false");
}

#define ax_dump(stream, value)					ax_dump_(stream, #value, value);

} //namespace
