#include "../common.h"
#include "JsonReader.h"

namespace ax_gen {

JsonReader::JsonReader() {
	reset();
}

JsonReader::JsonReader(const StrView& json, const StrView& filename) {
	reset();
	readJson(json, filename);
}

void JsonReader::operator=(const JsonReader& rhs) {
	_ch 		= rhs._ch;
	_r 			= rhs._r;
	_start 		= rhs._start;
	_end 		= rhs._end;
	_filename 	= rhs._filename;
	_lineNumber = rhs._lineNumber;
	_token 		= rhs._token;
	_valueType 	= rhs._valueType;
	_level 		= rhs._level;
}

void JsonReader::reset() {
	_token.str.reserve(512);
	_level.reserve(64);
	_lineNumber = 0;
	_valueType = ValueType::Invalid;

	_start = nullptr;
	_end   = nullptr;
	_r     = nullptr;
}

void JsonReader::readJson(const StrView& json, const StrView& filename) {
	_filename = filename;
	_start = json.data();
	_end = _start + json.size();
	_r = _start;

	_nextChar();
	next();
}

void JsonReader::beginObject() {
	if (_valueType != ValueType::BeginObject) error("ValueType is not {");
	next();
}

bool JsonReader::beginObject(const StrView& name) {
	if (!member(name)) return false;
	beginObject();
	return true;
}

bool JsonReader::endObject() {
	if (_valueType != ValueType::EndObject) return false;
	next();
	return true;
}

void JsonReader::beginArray() {
	if (_valueType != ValueType::BeginArray) error("ValueType is not [");
	next();
}

bool JsonReader::beginArray(const StrView& name) {
	if (!member(name)) return false;
	beginArray();
	return true;
}

bool JsonReader::endArray() {
	if (_valueType != ValueType::EndArray) return false;
	next();
	return true;
}

bool JsonReader::member(const StrView& name) {
	if (_valueType != ValueType::Member) error("value is not member");
	if (_token.str != name) return false;
	next();
	return true;
}

StrView JsonReader::memberWithPrefix(const StrView& prefix) {
	if (_valueType != ValueType::Member) error("value is not member");
	auto name = StrView(_token.str);	
	auto suffix = name.getFromPrefix(prefix);
	if (suffix) next();
	return suffix;
}

void JsonReader::getMemberName(String& out_value) {
	peekMemberName(out_value);
	next();
}

void JsonReader::peekMemberName(String& out_value) {
	if (_valueType != ValueType::Member) error("is not member name");
	out_value = _token.str;
}

void JsonReader::getValue(String& out_value) {
	if (_valueType != ValueType::String) error("value is not String");
	out_value = _token.str;
	next();
}

void JsonReader::getValue(double& out_value) {
	if (_valueType != ValueType::Number) error("value is not number");
	if (1 != sscanf(_token.str.c_str(), "%lf", &out_value)) {
		throw Error("error parsing json number ", _token.str);
	}
	next();
}

void JsonReader::getValue(bool& out_value) {
	if (_valueType != ValueType::Bool) error("value is not bool");
	out_value = _token.boolValue;
	next();
}

void JsonReader::skipValue() {
	switch (_valueType) {
		case ValueType::BeginArray: {
			beginArray();
			while (!endArray()) {
				skipValue();
			}
		}break;

		case ValueType::BeginObject: {
			beginObject();
			while (!endObject()) {
				skipValue();
			}
		}break;

		case ValueType::Member: {
			next();
			skipValue();
		}break;

		default: {
			next();
		}break;
	}
}

bool JsonReader::next() {
	auto lastValueType = _valueType;
	auto lastTokenType = _token.type;

	bool b = _nextToken();
	if (!b) return false;

	if (_currentLevel() == TokenType::BeginObject) {
		if (lastValueType != ValueType::Member) {
			if (_token.type == TokenType::EndObject) {
				_level.popBack();
				_valueType = ValueType::EndObject;
				return true;
			}

			if (lastTokenType != TokenType::BeginObject) {
				if (_token.type == TokenType::Comma) {
					_nextToken();
				} else {
					error("comma expected between members");
				}
			}

			if (_token.type != TokenType::String) {
				error("Member Name must be String");
			}
			_valueType = ValueType::Member;
			return true;
		}

		if (_token.type != TokenType::Colon) {
			error("colon expected after member name");
		}
		_nextToken();

	} else if (_currentLevel() == TokenType::BeginArray) {
		if (_token.type == TokenType::EndArray) {
			_level.popBack();
			_valueType = ValueType::EndArray;
			return true;
		}

		if (lastTokenType != TokenType::BeginArray) {
			if (_token.type == TokenType::Comma) {
				_nextToken();
			} else {
				error("comma expected between elements");
			}
		}
	}

	switch (_token.type) {
		case TokenType::BeginObject:
		{
			_level.append(_token.type);
			_valueType = ValueType::BeginObject;
			return true;
		}break;

		case TokenType::BeginArray:
		{
			_level.append(_token.type);
			_valueType = ValueType::BeginArray;
			return true;
		}break;

		case TokenType::Null:	 _valueType = ValueType::Null;		return true;
		case TokenType::Bool:	 _valueType = ValueType::Bool;		return true;
		case TokenType::Number:	 _valueType = ValueType::Number;	return true;
		case TokenType::String:	 _valueType = ValueType::String;	return true;
		default:
			error("Error invalid type in next()");
			break;
	}

	return false;
}

void JsonReader::dumpToken() {
	switch (_token.type) {
		case TokenType::Invalid:		Log::info("Token[Invalid]"); break;
		case TokenType::BeginObject:	Log::info("Token({)"); break;
		case TokenType::EndObject:		Log::info("Token(})"); break;
		case TokenType::BeginArray:		Log::info("Token([)"); break;
		case TokenType::EndArray:		Log::info("Token(])"); break;
		case TokenType::Null:			Log::info("Token(null)"); break;
		case TokenType::Number:			Log::info("Token(", _token.str, ")"); break;
		case TokenType::Bool:
		{
			auto tmp = _token.boolValue ? StrView("true") : StrView("false");
			Log::info("Token(", tmp, ")");
		}break;
		case TokenType::String:			Log::info("Token(str=", _token.str, ")"); break;
		case TokenType::Comma:			Log::info("Token(,)"); break;
		case TokenType::Colon:			Log::info("Token(:)"); break;
		default:
			error("");
	}
}

void JsonReader::dumpValue() {
	switch (_valueType) {
		case ValueType::Invalid:		Log::info("JsValue[Invalid]"); break;
		case ValueType::BeginObject:	Log::info("JsValue({)"); break;
		case ValueType::EndObject:		Log::info("JsValue(})"); break;
		case ValueType::BeginArray:		Log::info("JsValue([)"); break;
		case ValueType::EndArray:		Log::info("JsValue(])"); break;
		case ValueType::Null:			Log::info("JsValue(null)"); break;
		case ValueType::Number:			Log::info("JsValue(", _token.str, ")"); break;
		case ValueType::Bool:
		{
			auto tmp = _token.boolValue ? StrView("true") : StrView("false");
			Log::info("JsValue(", tmp, ")");
		}break;
		case ValueType::String:			Log::info("JsValue(str=", _token.str, ")"); break;
		case ValueType::Member:			Log::info("JsValue(member=", _token.str, ")"); break;
		default:
			throw Error("");
	}
}

bool JsonReader::_nextToken() {
	_valueType = ValueType::Invalid;
	_token.type = TokenType::Invalid;
	_token.str.clear();

	for (;;) {
		//trim space
		while (_ch == ' ' || _ch == '\t' || _ch == '\n' || _ch == '\r') {
			_nextChar();
		}

		if (_ch == '/') {
			_nextChar();
			switch (_ch) {
				case '/': { 
					//trim inline comment
					_nextChar();
					while (_ch != '\n' && _ch != 0) {
						_nextChar();
					}
				}continue; //back to trim space again

				case '*': {
					//trim block comment
					_nextChar();
					for (;;) {
						if (_ch == 0) {
							error("Unexpected end of file in comment block");
							return false;
						}

						if (_ch == '*') {
							_nextChar();
							if (_ch == '/') {
								_nextChar();
								break;
							}
						}else{
							_nextChar();
						}
					}
				}continue;

				default: error("Invalid token after '/'");
			}
		}

		break;
	}

	switch (_ch) {
		case 0: return false;
		case '{': _token.type = TokenType::BeginObject;	_nextChar(); return true;
		case '}': _token.type = TokenType::EndObject;	_nextChar(); return true;
		case '[': _token.type = TokenType::BeginArray;	_nextChar(); return true;
		case ']': _token.type = TokenType::EndArray;	_nextChar(); return true;
		case ',': _token.type = TokenType::Comma;		_nextChar(); return true;
		case ':': _token.type = TokenType::Colon;		_nextChar(); return true;
		case '"':
		{
			_token.type = TokenType::String;
			_parseStringToken();
			return true;
		};
	}

	for (;;) {
		if (isdigit(_ch) || isalpha(_ch)) {
			_token.str += _ch;
			_nextChar();
		} else {
			break;
		}
	}

	if (_token.str == "null") {
		_token.type = TokenType::Null;
		return true;
	}

	if (_token.str == "true") {
		_token.type = TokenType::Bool;
		_token.boolValue = true;
		return true;
	}

	if (_token.str == "false") {
		_token.type = TokenType::Bool;
		_token.boolValue = false;
		return true;
	}

	if (_token.str.size() > 0) {
		if (isdigit(_token.str[0])) {
			_token.type = TokenType::Number;
			return true;
		}
	}

	error("token error token.str=", _token.str);
	return false;
}

void JsonReader::_parseStringToken() {
	for (;;) {
		_nextChar();
		if (_ch == 0) {
			throw Error("Unexpected end of Json String");
		}
		switch (_ch) {
			case '"': _nextChar(); return;
				//escape char
			case '\\':
			{
				_nextChar();
				switch (_ch) {
					case '\\':
					case '/':
					case '"':
						_token.str += _ch;
						break;
					case 'b': _token.str += '\b'; break;
					case 'f': _token.str += '\f'; break;
					case 'n': _token.str += '\n'; break;
					case 'r': _token.str += '\r'; break;
					case 't': _token.str += '\t'; break;
					default:
						throw Error("Unexpected String escape char ", _ch);
				}
			}break;

			default:
			{
				_token.str += _ch;
			}break;
		}
	}
}

void JsonReader::getLocationString(String& out_str, int maxNumLines) {
	if (!_start) return;

	std::stringstream s;

	const char* lineStart  = nullptr;
	const char* lineEnd    = nullptr;
	const char* blockStart = nullptr;

	if (!_findPrevNewlineLocation(lineStart, 1)) {
		lineStart = _start;
	}

	if (!_findPrevNewlineLocation(blockStart, maxNumLines)) {
		throw Error("cannot find line end");
	}

	if (!_findNextNewlineLocation(lineEnd, 1)) {
		//throw Error("cannot find line end");
		lineEnd = _end;
	}

	auto col = _r - lineStart;
	s << '\n' << _filename << ':' << _lineNumber << ',' << col << '\n';
	s.write(blockStart, lineEnd - blockStart);

	s << '\n';
	for (auto* p = lineStart; p < _r; p++) {
		switch (*p) {
		case '\t': s << '\t'; break;
		default:   s << ' '; break;
		}
	}
	s << "^----";

	out_str = StrView(s.str());
}

int JsonReader::_findPrevNewlineLocation(const char* &out_pos, int maxNumLines) {
	if (!_start || !_r) return 0;
	int found = 0;
	auto* p = _r - 1;
	for (; p >= _start; p--) {
		if (*p != '\n') continue;
		found++;
		out_pos = p;
		if (found >= maxNumLines) break;
	}
	return found;
}

int JsonReader::_findNextNewlineLocation(const char* &out_pos, int maxNumLines) {
	if (!_start || !_r) return 0;
	int found = 0;
	auto* p = _r;
	for (; p < _end; p++) {
		if (*p != '\n') continue;
		found++;
		out_pos = p;
		if (found >= maxNumLines) break;
	}
	return found;
}

} //namespace
