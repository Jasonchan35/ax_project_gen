#pragma once

#include "String.h"

namespace ax_gen {

struct Path {
	static const int kPathMax = 512;

	static bool		isAbs	(const StrView& path);
	static void		getAbs	(String& out_dir,  const StrView& path);
	static void		getRel	(String& out_str,  const StrView& path, const StrView& relativeTo);

	static StrView	dirname	  (const StrView& path);
	static StrView	basename  (const StrView& path, bool withExtension);
	static StrView	extension (const StrView& path);

	struct SplitResult	{
		StrView driver;
		StrView dir;
		StrView name;
		StrView ext;
	};
		
	static SplitResult	split	(const StrView& path);

	static void		makeFullPath(String& out_str, const StrView& dir, const StrView& path);

	static void		windowsPath	(String& outPath, const StrView& path);
	static void		unixPath	(String& outPath, const StrView& path);

	static bool		fileExists	(const StrView& path);
	static bool		dirExists	(const StrView& path);

	static void		getCurrentDir(String& out_dir);
	static bool		makeDir		(const StrView& path);

	static void		glob		(Vector<String>& out_paths, const StrView& path, bool needDir = true, bool needFile = true, bool needHidden = false);

private:
	Path() = delete;

};

} //namespace
