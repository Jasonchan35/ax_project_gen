#pragma once

#include <vector>

namespace ax_gen {

template<typename T>
class Vector {
public:
			void popBack	()				{ _p.pop_back(); }
			void append		(const T& v)	{ _p.push_back(v); }
			void extend		(const Vector<T>& rhs)	{ for (auto& p : rhs) append(p); }

			template<typename... ARGS>
			T& emplaceBack(ARGS&&... args) { _p.emplace_back(std::forward<ARGS>(args)...); return back(); }

			void resize		(int n)			{ _p.resize((size_t)n); }
			void reserve	(int n)			{ _p.reserve((size_t)n); }
			int  size		() const		{ return (int)_p.size(); }
			T&	 back		()				{ return _p.back(); }

			T*	data		()				{ return _p.data(); }

			T&	operator[]	(int i)			{ return _p[(size_t)i]; }
	const	T&	operator[]	(int i) const	{ return _p[(size_t)i]; }

		int		indexOf		(const T& v);
		void	removeAt	(int i)			{ _p.erase(_p.begin() + i); }

			T*	begin		()				{ return _p.data(); }
	const	T*	begin		() const		{ return _p.data(); }

			T*	end			()				{ return _p.data() + _p.size(); }
	const	T*	end			() const		{ return _p.data() + _p.size(); }

		void	clear		()				{ _p.clear(); }


	void uniqueAppend(const T& v)			{ if (indexOf(v) >= 0) return; append(v); }
	void uniqueExtend(const Vector<T>& rhs)	{ for (auto& p : rhs) uniqueAppend(p); }

	explicit operator bool() const { return size() != 0; }

private:
	std::vector<T>	_p;
};

template<typename T> inline
int ax_gen::Vector<T>::indexOf(const T& v) {
	int i = 0;
	for (auto& a : _p) {
		if (a == v) {
			return i;
		}
	}
	return -1;
}

template<typename T> inline
std::ostream& operator<<(std::ostream& s, Vector<T>& v) {
	//s << "[";
	int i = 0;
	for (auto& it : v) {
		if (i > 0) s << "\n" << std::setw(ax_dump_padding + 3) << " ";
		s << it;
		i++;
	}
	//s << "]";
	return s;
}

} //namespace
