#pragma once

#if !ax_COMPILER_VC	
	#error
#endif

#define	ax_FUNC_NAME			__FUNCTION__
#define ax_PRETTY_FUNC_NAME		__FUNCSIG__

#define ax_DEPRECATED			__declspec(deprecated)

#define ax_OPTIMIZE_OFF			__pragma optimize("", off)

#if _MSC_VER < 1600
	#define	nullptr	NULL
#endif

//cpu
#if _M_X64 || _M_AMD64 
	#define ax_CPU_x86_64	1

#elif _M_PPC
	#define ax_CPU_PowerPC     1
#else
	#define ax_CPU_x86         1
	
#endif

//os

#if _WIN64
	#define ax_OS_Win64     1
	#define ax_OS_Windows	1
#elif _WIN32
	#define ax_OS_Win32     1
	#define ax_OS_Windows	1
#elif _WINCE
	#define ax_OS_WinCE     1
	#define ax_OS_Windows	1
#endif
