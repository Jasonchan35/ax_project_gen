#pragma once

#include "String.h"

namespace ax_gen {

using UniChar = uint32_t;

struct UtfUtil {

	inline static void	convert(char*    dst, int dst_len, const char*    src, int src_len) { _convert(dst, dst_len, src, src_len); }
	inline static void	convert(char*    dst, int dst_len, const wchar_t* src, int src_len) { _convert(dst, dst_len, src, src_len); }
	inline static void	convert(wchar_t* dst, int dst_len, const char*    src, int src_len) { _convert(dst, dst_len, src, src_len); }
	inline static void	convert(wchar_t* dst, int dst_len, const wchar_t* src, int src_len) { _convert(dst, dst_len, src, src_len); }

	template< typename SRC, typename DST >
	static int	getConvertedCount ( const SRC* src, int src_len );

private:

	template< typename SRC, typename DST >
	static void	_convert	( DST* dst, int dst_len, const SRC* src, int src_len );

	template< typename SRC > static int			_utfCount 	( UniChar v );
	template< typename SRC > static UniChar 	_decodeUtf	( const SRC* & src, const SRC* e );
	template< typename DST > static void 		_encodeUtf	(       DST* & dst, const DST* e, UniChar v );

	UtfUtil() = delete;
}; // UtfUtil

template< typename SRC, typename DST > inline
int UtfUtil::getConvertedCount(const SRC* src, int src_len) {
	if (sizeof(DST) == sizeof(SRC)) {
		return src_len;
	}

	int out_len = 0;
	auto e = src + src_len;
	for (; src < e; src++) {
		out_len += _utfCount<DST>(_decodeUtf(src, e));
	}
	return out_len;
}

template< typename SRC, typename DST > inline
void UtfUtil::_convert(DST* dst, int dst_len, const SRC* src, int src_len) {
	if (sizeof(DST) == sizeof(SRC)) {
		memcpy(dst, reinterpret_cast<const DST*>(src), src_len);
	} else {
		auto e = src + src_len;
		auto de = dst + dst_len;
		for (; src < e; src++, dst++) {
			_encodeUtf(dst, de, _decodeUtf(src, e));
		}
	}
}

template<> inline
UniChar UtfUtil::_decodeUtf<char>( const char* & src, const char* end ) {
	auto a = static_cast<UniChar>(*src);

	if( ( a & 0x80 ) == 0 ) return a;

	if( ( a & 0xE0 ) == 0xC0 ) {
		if( src+2 > end ) throw Error("Convert UTF");
		auto b = static_cast<UniChar>(*src); src++;
		return ( ( a & 0x1F ) << 6 ) | ( b & 0x3F );
	}

	if( ( a & 0xF0 ) == 0xE0 ) {
		if( src+3 > end ) throw Error("Convert UTF");
		auto b = static_cast<UniChar>(*src); src++;
		auto c = static_cast<UniChar>(*src); src++;
		return ( (a & 0x0F) << 12 ) | ( (b & 0x3F) << 6 ) | ( c & 0x3F );
	}

	if( ( a & 0xF8 ) == 0xF0 ) {
		if( src+4 > end ) throw Error("Convert UTF");
		auto b = static_cast<UniChar>(*src); src++;
		auto c = static_cast<UniChar>(*src); src++;
		auto d = static_cast<UniChar>(*src); src++;
		return ( (a & 0x07) << 18 ) | ( (b & 0x3F) << 12 ) | ( (c & 0x3F) << 6 ) | ( d & 0x3F );
	}


	if( ( a & 0xFC ) == 0xF8 ) {
		if( src+5 > end ) throw Error("Convert UTF");
		auto b = static_cast<UniChar>(*src); src++;
		auto c = static_cast<UniChar>(*src); src++;
		auto d = static_cast<UniChar>(*src); src++;
		auto e = static_cast<UniChar>(*src); src++;
		return ( (a & 0x03) << 24 ) | ( (b & 0x3F) << 18 ) | ( (c & 0x3F) << 12 ) | ( (d & 0x3F) << 6 ) | ( e & 0x3F );
	}

	if( ( a & 0xFE ) == 0xFC ) {
		if( src+6 > end ) throw Error("Convert UTF");
		auto b = static_cast<UniChar>(*src); src++;
		auto c = static_cast<UniChar>(*src); src++;
		auto d = static_cast<UniChar>(*src); src++;
		auto e = static_cast<UniChar>(*src); src++;
		auto f = static_cast<UniChar>(*src); src++;
		return ( (a & 0x01) << 30 ) | ( (b & 0x3F) << 24 ) | ( (c & 0x3F) << 18 ) | ( (d & 0x3F) << 12 ) | ( (e & 0x3F) << 6 ) | ( f & 0x3F );
	}

	throw Error("Convert UTF");
}

template<> inline
UniChar UtfUtil::_decodeUtf<wchar_t>( const wchar_t* & src, const wchar_t* end ) {
	UniChar a = *src;
	if( a >= 0xD800 && a <= 0xDBFF ) {
		src++;
		if( src >= end ) throw Error("Convert UTF");

		UniChar b = *src;
		if( b >= 0xDC00 && b <= 0xDFFF ) {
			return ( a << 10 ) + b - 0x35FDC00;
		}else{
			src--; // push back
		}
	}
	return a;
}

//----------------------------------------------------

template<> inline
int UtfUtil::_utfCount< char >( UniChar v ) {
	if( v <       0x80 ) return 1;
	if( v <    0x00800 ) return 2;
	if( v <    0x10000 ) return 3;
	if( v <   0x200000 ) return 4;
// The patterns below are not part of UTF-8, but were part of the first specification.
	if( v <  0x4000000 ) return 5;
	if( v < 0x80000000 ) return 6;
	throw Error("Convert UTF");
}

template<> inline
int UtfUtil::_utfCount< wchar_t >( UniChar v ) {
	if( v <  0x10000 ) return 1;
	if( v < 0x110000 ) return 2;
	throw Error("Convert UTF");
}

//----------------------------------------------------

template<> inline
void UtfUtil::_encodeUtf< char >( char* & dst, const char* end, UniChar v ) {
	if( v <       0x80 ) {
		*dst = static_cast<char>(v);
		return;
	}

	if( v <    0x00800 ) {
		if( dst + 2 > end ) throw Error("Convert UTF");
		*dst = static_cast<char>(( v >> 6   ) | 0xC0); dst++;
		*dst = static_cast<char>(( v & 0x3F ) | 0x80); dst++;
		return;
	}

	if( v <    0x10000 ) {
		if( dst + 3 > end ) throw Error("Convert UTF");
		*dst = static_cast<char>(( (v >> 12)        ) | 0xC0); dst++;
		*dst = static_cast<char>(( (v >> 6 ) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>((  v        & 0x3F ) | 0x80); dst++;
		return;
	}

	if( v <   0x200000 ) {
		if( dst + 4 > end ) throw Error("Convert UTF");
		*dst = static_cast<char>(( (v >> 18)        ) | 0xC0); dst++;
		*dst = static_cast<char>(( (v >> 12) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(( (v >> 6 ) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>((  v        & 0x3F ) | 0x80); dst++;
		return;
	}

// The patterns below are not part of UTF-8, but were part of the first specification.
	if( v <  0x4000000 ) {
		if( dst + 5 > end ) throw Error("Convert UTF");
		*dst = static_cast<char>(((v >> 24)        ) | 0xC0); dst++;
		*dst = static_cast<char>(((v >> 18) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(((v >> 12) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(((v >> 6 ) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(( v        & 0x3F ) | 0x80); dst++;
		return;
	}

	if( v < 0x80000000 ) {
		if( dst + 6 > end ) throw Error("Convert UTF");
		*dst = static_cast<char>(((v >> 30)        ) | 0xC0); dst++;
		*dst = static_cast<char>(((v >> 24) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(((v >> 18) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(((v >> 12) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(((v >> 6 ) & 0x3F ) | 0x80); dst++;
		*dst = static_cast<char>(( v        & 0x3F ) | 0x80); dst++;
		return;
	}

	throw Error("Convert UTF");
}

template<> inline
void UtfUtil::_encodeUtf< wchar_t >( wchar_t* & dst, const wchar_t* end, UniChar v ) {
	if( v <  0x10000 ) {
		if( dst + 1 > end ) throw Error("Convert UTF");
		*dst = static_cast<wchar_t>(v);
		return;
	}

	if( v < 0x110000 ) {
		if( dst + 2 > end ) throw Error("Convert UTF");
		*dst = static_cast<wchar_t>(( v >> 10   ) + 0xD7C0); dst++;
		*dst = static_cast<wchar_t>(( v & 0x3FF ) + 0xDC00); dst++;
		return;
	}

	throw Error("Convert UTF");
};

} //namespace ax_gen
