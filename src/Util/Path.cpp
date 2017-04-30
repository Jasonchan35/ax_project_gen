#include "../common.h"
#include "Path.h"

namespace ax_gen {

bool Path::isAbs(const StrView& path) {
	if (path.size() < 1) return false;
	if (path[0] == '/') return true;

	if (path.size() < 2) return false;
	if (isalpha(path[0]) && path[1] == ':') return true;

	return false;
}

StrView Path::dirname(const StrView& path) {
	auto s = path.splitEndByAnyCharIn("\\/");
	return s.second ? s.first : StrView();
}

StrView Path::basename(const StrView& path, bool withExtension) {
	auto s = path.splitEndByAnyCharIn("\\/");
	auto f = s.second ? s.second : s.first;
	if (withExtension) return f;
	s = f.splitEndByChar('.');
	return s.first;
}

StrView Path::extension(const StrView& path) {
	//	remove dir first to avoid corner case like: "/aaa/bbb/ccc.here/eee"
	//	while should return "" instead or "here/eee"
	auto s = path.splitEndByAnyCharIn("\\/");
	auto f = s.second ? s.second : s.first;

	s = f.splitEndByChar('.');
	return s.second;
}

Path::SpliteResult Path::splite(const StrView& path) {
	SpliteResult o;

	auto s = path.splitEndByAnyCharIn("\\/");
	o.dir  = s.second ? s.first : StrView();
	
	auto basename = s.second ? s.second : s.first;
	s = basename.splitEndByChar('.');
	o.name = s.first;
	if (s.second) o.ext = s.second;

	return o;
}

void Path::makeFullPath(String& out_str, const StrView& dir, const StrView& path) {
	if (isAbs(path)) {
		getAbs(out_str, path); // normalize '.' or '..'
	}else{
		String tmp;
		tmp.append(dir, '/', path);
		Path::getAbs(out_str, tmp);
	}
}

void Path::getAbs(String& out_str, const StrView& path) {
	out_str.clear();

	if (!path) return;
	bool needSlash = false;
	if (Path::isAbs(path)) {
		needSlash = (path[0] == '/'); //unix path need '/' at beginning

	}else{
		getCurrentDir(out_str);
		needSlash = true;
	}

	StrView p = path;
	while (p) {
		auto s = p.splitByCharInList("\\/");
		if (s.first == ".") {
			//skip '.'
		}else if (!s.first) {
			//skip '/'
		}else if (s.first == "..") {
			auto idx = out_str.view().lastIndexOfAnyCharIn("\\/");
			if (idx < 0) {
				out_str.clear(); //no more parent folder
				return;
			}

			out_str.resize(idx);
		}else{
			if (needSlash) {
				out_str += '/';
			}
			out_str += s.first;
			needSlash = true;
		}
		p = s.second;
	}
}

void Path::getRel(String& out_str, const StrView& path, const StrView& relativeTo) {
	out_str.clear();

	String src;
	Path::getAbs(src, path);

	String to;
	Path::getAbs(to, relativeTo);

	auto sv = src.view();
	auto tv = to.view();
	
	//unix path starts with /
	if (sv && sv[0] == '/') sv = sv.sliceFrom(1);
	if (tv && tv[0] == '/') tv = tv.sliceFrom(1);
	
	for(;;) {
		auto sp = sv.splitByChar('/');
		auto tp = tv.splitByChar('/');
		if(!sp.first || !tp.first) break;
		if (sp.first != tp.first) break;
		
		sv = sp.second;
		tv = tp.second;
	}

	for(;;) {
		auto tp = tv.splitByChar('/');
		if (!tp.first) break;
		out_str.append("../");
		tv = tp.second;
	}

	out_str.append(sv);
}

void Path::getCurrentDir(String& out_dir) {
#if ax_OS_Windows
	wchar_t  tmp[ kPathMax + 1 ];
	auto n = ::GetCurrentDirectory((DWORD)kPathMax, tmp);
	tmp[n] = 0;
	out_dir.setUtf(tmp, n);
	out_dir.replaceChars('\\', '/');

#else
    out_dir.clear();
	char  tmp[ kPathMax + 1 ];
	if( ! ::getcwd( tmp, kPathMax ) ) {
		throw Error("cannot get current directory");
	}
	tmp[ kPathMax ] = 0;
	out_dir = StrView_c_str(tmp);
#endif
}

void Path::windowsPath(String& outPath, const StrView& path) {
	outPath = path;
	outPath.replaceChars('/', '\\');	
}

void Path::unixPath(String& outPath, const StrView& path) {
	outPath = path;
	outPath.replaceChars('\\', '/');
}

bool Path::makeDir(const StrView& path) {
	if (Path::dirExists(path)) return false;

	auto parent = Path::dirname(path);
	if (parent) {
		makeDir(parent);
	}

#if ax_OS_Windows
	WString pathW;
	pathW.setUtf(path);
	if (0 == ::CreateDirectoryW(pathW.c_str(), nullptr)) {
		throw Error("cannot create directory ", path);
	}
#else
	String pathA(path);
	if (0 != mkdir(pathA.c_str(), S_IRWXG | S_IRWXU | S_IRWXO)) {
		throw Error("cannot create directory ", path);
	}
#endif
	return true;
}

bool Path::fileExists(const StrView& path) {
#if ax_OS_Windows
	WString pathW;
	pathW.setUtf(path);
    DWORD dwAttrib = ::GetFileAttributes(pathW.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	String pathA;
	pathA = path;
	struct stat s;
	if( 0 != ::stat( pathA.c_str(), &s ) ) return false;
	return ( s.st_mode & S_IFDIR ) == 0;
#endif	
}

bool Path::dirExists(const StrView& path) {
#if ax_OS_Windows
	WString pathW;
	pathW.setUtf(path);
    DWORD dwAttrib = ::GetFileAttributes(pathW.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
	String pathA;
	pathA = path;
	struct stat s;
	if( 0 != ::stat( pathA.c_str(), &s ) ) return false;
	return ( s.st_mode & S_IFDIR ) != 0;
#endif
}

} //namespace
