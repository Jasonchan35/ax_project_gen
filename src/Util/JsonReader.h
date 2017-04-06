#pragma once

#include "Log.h"

namespace ax_gen {

class JsonReader {
public:
	enum class ValueType {
		Invalid,
		BeginObject,
		EndObject,
		BeginArray,
		EndArray,
		Number,
		Bool,
		Null,
		String,
		Member,
	};

	JsonReader();
	JsonReader(const StrView& json, const StrView& filename);
	JsonReader(JsonReader && rhs) { operator=(rhs); }

	void readJson(const StrView& json, const StrView& filename);

	ValueType valeType() { return _valueType; }

	bool beginObject(const StrView& name);
	void beginObject();
	bool endObject();

	bool beginArray(const StrView& name);
	void beginArray();
	bool endArray();

	bool member(const StrView& name);

	template<typename T>
	bool member(const StrView& name, T& out_value);

	StrView memberWithPrefix(const StrView& prefix);


	void getMemberName(String& out_value);

	void peekMemberName(String& out_value);


	void getValue(String&	out_value);
	void getValue(bool&		out_value);
	void getValue(double&	out_value);
	void getValue(int&		out_value) { double tmp; getValue(tmp); out_value = static_cast<int>(tmp); }
	void getValue(uint32_t& out_value) { double tmp; getValue(tmp); out_value = static_cast<uint32_t>(tmp); }

	template<typename T>
	void getValue(Vector<T>& out_value);

	void skipValue();

	void reset();
	bool next();

	template<typename... ARGS> void info	(ARGS&&... args) { log(Log::Level::Info,	std::forward<ARGS>(args)...); }
	template<typename... ARGS> void warning	(ARGS&&... args) { log(Log::Level::Warning, std::forward<ARGS>(args)...); }
	template<typename... ARGS> void error	(ARGS&&... args) { log(Log::Level::Error,	std::forward<ARGS>(args)...); }

	template<typename... ARGS>
	void log(Log::Level lv, ARGS&&... args);

	void dumpToken();
	void dumpValue();

	void getLocationString(String& out_str, int maxNumLines);

	StrView		filename() const { return _filename; }

	JsonReader	clone() const { return JsonReader(*this); }

	bool unhandledMember();
	bool warningAndSkipUnhandledMember();
	bool errorOnUnhandledMemeber();

private:

	JsonReader(const JsonReader& rhs) { operator=(rhs); }
	void operator=(const JsonReader& rhs);

	enum class TokenType {
		Invalid,
		BeginObject,
		EndObject,
		BeginArray,
		EndArray,
		Number,
		Bool,
		Null,
		String,
		Comma,
		Colon,
	};

	bool _nextToken();
	void _nextChar();

	void _skipCommentBlock();
	void _parseStringToken();

	int _findPrevNewlineLocation(const char* &out_pos, int maxNumLines);
	int _findNextNewlineLocation(const char* &out_pos, int maxNumLines);

	TokenType _currentLevel() {
		return _levels ? _levels.back().type : TokenType::Invalid;
	}

	char _ch;
	const char* _r;
	const char* _start;
	const char* _end;

	struct Token {
		TokenType	type;
		String		str;
		bool		boolValue;
	};

	String		_filename;
	int			_lineNumber;
	Token		_token;
	ValueType	_valueType;
	
	struct Level {
		Level(TokenType type_, const char* pos_) : type(type_), pos(pos_) {}
	
		TokenType	type;
		const char* pos;
	};
	
	Vector<Level>	_levels;
};

template<typename T>
bool JsonReader::member(const StrView& name, T& out_value)
{
	if (!member(name)) return false;
	getValue(out_value);
	return true;
}

template<typename T>
void JsonReader::getValue(Vector<T>& out_value)
{
	beginArray();
	while (!endArray()) {
		T tmp;
		getValue(tmp);
		out_value.append(std::move(tmp));
	}
}

template<typename... ARGS> inline
void JsonReader::log(Log::Level lv, ARGS&&... args) {
	String s;
	getLocationString(s, 2);
	if (lv == Log::Level::Error) {
		throw Error(std::forward<ARGS>(args)..., s);
	}
	else {
		Log::log(lv, std::forward<ARGS>(args)..., s);
	}
}

inline
void JsonReader::_nextChar() {
	if (_r >= _end) {
		_ch = 0;
		return;
	}
	_ch = *_r;
	_r++;

	if (_ch == '\n') {
		_lineNumber++;
	}
}

} //namespace
