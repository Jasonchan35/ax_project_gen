#pragma once

#include "String.h"

namespace ax_gen {

//TODO change to Log::info(), so class Log -> LogImpl

class Log : public NonCopyable {
public:
	enum class Level {
		Info,
		Warning,
		Error,
	};

	static void onOutput(Level lv, const StrView& s);

	template<typename... ARGS> static void info		(ARGS&&... args) { log(Level::Info,		std::forward<ARGS>(args)...); }
	template<typename... ARGS> static void warning	(ARGS&&... args) { log(Level::Warning,	std::forward<ARGS>(args)...); }
	template<typename... ARGS> static void error	(ARGS&&... args) { log(Level::Error,	std::forward<ARGS>(args)...); }
	template<typename... ARGS> static void log		(Level lv, ARGS&&... args) {
		StringStream s;
		switch (lv) {
			case Level::Info:	 break;
			case Level::Warning: s.append("warning: "); break;
			case Level::Error:	 s.append("  error: "); break;
		}
		s.append(std::forward<ARGS>(args)...);
		s.append("\n");

		onOutput(lv, StrView(s.str()));
	}

	static void createLogFile(const StrView& filename);
	static void flushLogFile();
	static void closeLogFile();
	static void setLogLevel(Level lv);
};

} //namespace



