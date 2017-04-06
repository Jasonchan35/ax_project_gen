//
//  System.cpp
//  ax_gen
//
//  Created by Jason on 2017-04-05.
//  Copyright © 2017 Jason. All rights reserved.
//

#include "System.h"

namespace ax_gen {

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

void System::createProcess(const StrView& exe, const Vector<StrView>& argv) {
	String cmd("\"", exe, "\"");
	for (auto& v : argv) {
		cmd.append(" \"", v,"\"");
	}

	Log::info("createProcess> ", cmd);

#if ax_OS_Windows
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
		return;
	}

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
	//::system(cmd.c_str());
	//return

	#if 1
		cmd.append(" 2>&1"); //redirect stderr to stdout

		FILE* fs = ::popen(cmd.c_str(), "r");
		if (!fs) {
			Log::error("error createProcess");
			return;
		}

		Vector<char> buf;
		buf.resize(2 * 1024 * 1024);
		
		while (!feof(fs)) {
			if (fgets(buf.data(), buf.size(), fs) != nullptr) {
				Log::info(StrView_c_str(buf.data()));
			}
		}

		pclose(fs);
	#else

		Vector<String> argv_str; // holding null-terminated string for char* in argv_p
		Vector<char*>  argv_p;
		
		//first arg must be the exe itself
		argv_str.append(exe);
		
		for (auto& v : argv) {
			argv_str.append(v);
		}
		
		for (auto& v : argv_str) {
			argv_p.append((char*)v.c_str());
		}
		//end will nullptr
		argv_p.append(nullptr);
		
		
		int out_pipe[2];
		int err_pipe[2];
		
		if(pipe(out_pipe) || pipe(err_pipe)) {
			throw Error("error create pipe");
		}

		posix_spawn_file_actions_t action;
		posix_spawn_file_actions_init(&action);
		posix_spawn_file_actions_addclose(&action, out_pipe[0]);
		posix_spawn_file_actions_addclose(&action, err_pipe[0]);
		posix_spawn_file_actions_adddup2( &action, out_pipe[1], 1);
		posix_spawn_file_actions_adddup2( &action, err_pipe[1], 2);
		posix_spawn_file_actions_addclose(&action, out_pipe[1]);
		posix_spawn_file_actions_addclose(&action, err_pipe[1]);
		
		pid_t pid;
		
		//posix_spawn();
		int ret = posix_spawnp(&pid, argv_p[0], &action, nullptr, argv_p.data(), nullptr);
		if (ret != 0) {
			Log::error("error createProcess ", ret);
			return;
		}
		
		pollfd p[2];
		p[0].fd = out_pipe[0];
		p[0].events = POLLIN;
		p[0].revents = 0;

		p[1].fd = err_pipe[0];
		p[1].events = POLLIN;
		p[1].revents = 0;
		
		for (;;) {
			int timeout = -1; //ms
			int rval = poll(p, 2, timeout);
			if (rval <= 0) break;
			
			if ( p[0].revents & POLLIN) {
				ssize_t bytes_read = read(out_pipe[0], buf.data(), (size_t)buf.size());
				Log::info(StrView(buf.data(), (int)bytes_read));
			}else if ( p[1].revents & POLLIN ) {
				ssize_t bytes_read = read(err_pipe[0], buf.data(), (size_t)buf.size());
				Log::error(StrView(buf.data(), (int)bytes_read));
			}else{
				break; // no more to read
			}
		}
		
		int exit_code = -1;
		waitpid(pid,&exit_code,0);
		Log::info("process exit return: ", exit_code);

		posix_spawn_file_actions_destroy(&action);
	#endif

#endif //ax_OS
}

	
} //namespace
