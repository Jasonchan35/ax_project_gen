#include "../common.h"
#include "FileUtil.h"
#include "Path.h"

namespace ax_gen {

void FileUtil::readTextFile(const StrView& filename, String& out_str) {
	String strFilename(filename);

	std::ifstream file(strFilename.c_str(), std::ios::binary);

	if (!file.is_open()) {
		throw Error("cannot read file ", filename);
	}

    file.seekg(0, std::ios::end);

    int fileSize = (int)file.tellg();
    file.seekg(0, std::ios::beg);
	
	Vector<char> buf;
	buf.resize(fileSize);

	file.read(buf.data(), buf.size());
	file.close();

	out_str = StrView(buf.data(), buf.size());
}

void FileUtil::writeTextFile(const StrView& filename, const StrView& text, bool verbose) {
	String strFilename(filename);

	char type = '+';
	if (Path::fileExists(filename)) {
		String tmp;
		readTextFile(filename, tmp);
		type = (tmp == text) ? '=' : 'U';
	}

	if (type == '=') return;

	if (verbose) {
		Log::info("[", type, "] ", filename);
	}
	
	auto dir = Path::dirname(filename);
	if (dir) {
		Path::makeDir(dir);
	}

	std::ofstream file(strFilename.c_str(), std::ios::binary);
	if (!file.is_open()) {
		throw Error("cannot write file ", filename);
	}
	file.write(text.data(), text.size());
	file.close();
}

} //namespace
