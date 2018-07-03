#include "../common.h"
#include "Log.h"
#include "Path.h"

namespace ax_gen {

class LogImpl {
public:
	~LogImpl() {
		Log::closeLogFile();
	}

	std::ofstream file;
	String fileBuffer;
	Log::Level level = Log::Level::Info;
};

LogImpl	g_log;

void Log::onOutput(Level lv, const StrView& s) {
	if (lv < g_log.level)
		return;

	std::cout.write(s.data(), s.size());

	g_log.fileBuffer.append(s);

	const int kMaxBufferSize = 512 * 1024;
	if (lv == Level::Error || g_log.fileBuffer.size() >= kMaxBufferSize) {
		flushLogFile();
	}
}

void Log::createLogFile(const StrView& filename_) {
	String filename = filename_;

	Path::makeDir(Path::dirname(filename_));

	g_log.file.open(filename.c_str(), std::ios::binary);
	g_log.fileBuffer.reserve(4096);
}

void Log::flushLogFile() {
	if (!g_log.file.is_open()) return;
	g_log.file.write(g_log.fileBuffer.c_str(), g_log.fileBuffer.size());
	g_log.file.flush();
	g_log.fileBuffer.clear();
}

void Log::closeLogFile() {
	if (!g_log.file.is_open()) return;
	flushLogFile();
	g_log.file.close();
}

void Log::setLogLevel(Level lv) {
	g_log.level = lv;
}

} //namespace
