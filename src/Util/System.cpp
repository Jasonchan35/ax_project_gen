//
//  System.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-05.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "System.h"

namespace ax_gen {

int System::cpuCount() {
	return (int)std::thread::hardware_concurrency();
}

void System::shellOpen(const StrView& path) {
	Log::info("shellOpen> ", path);

#if ax_OS_Windows
	WString pathW;
	pathW.setUtf(path);
	::ShellExecute(nullptr, L"open", pathW.c_str(), nullptr, nullptr, SW_SHOW);

#elif ax_OS_MacOSX

	String cmd("open \"", path, "\"");
	::system(cmd.c_str());

#else
	Log::error("shellOpen does't support on this platform");
#endif

}

int System::createProcess(const StrView& exe, const StrView& args) {
#if ax_OS_Windows
	String cmd("\"", exe, "\" ", args);
	Log::info("run> ", cmd);

	WString exeW;
	exeW.setUtf(exe);

	WString cmdW;
	cmdW.appendUtf(cmd);

	STARTUPINFOW si; 
	ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi; 
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if (!::CreateProcess(exeW.c_str(), cmdW.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)) {
		auto dw = GetLastError();
		Log::error("error createProcess", dw);
		return -1;
	}

    WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code = -1;
	GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else

	String cmd("\"", exe, "\" ", args, " 2>&1"); //redirect stderr to stdout
	Log::info("run> ", cmd);

	FILE* fs = ::popen(cmd.c_str(), "r");
	if (!fs) {
		Log::error("error createProcess");
		return -1;
	}

	Vector<char> buf;
	buf.resize(2 * 1024 * 1024);
	
	while (!feof(fs)) {
		if (fgets(buf.data(), buf.size(), fs) != nullptr) {
			Log::info(StrView_c_str(buf.data()));
		}
	}

	int exit_code = pclose(fs);

#endif //ax_OS

	Log::info("process exit return: ", exit_code);
	return (int)exit_code;
}

	
} //namespace
