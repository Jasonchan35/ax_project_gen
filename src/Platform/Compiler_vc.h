#pragma once

#if !AX_COMPILER_VC	
	#error
#endif

#define	AX_FUNC_NAME			__FUNCTION__
#define AX_PRETTY_FUNC_NAME		__FUNCSIG__

#define AX_DEPRECATED			__declspec(deprecated)

#define AX_OPTIMIZE_OFF			__pragma optimize("", off)

#if _MSC_VER < 1600
	#define	nullptr	NULL
#endif

//cpu
#if _M_X64 || _M_AMD64 
	#define AX_CPU_X86_64	1

#elif _M_PPC
	#define AX_CPU_POWER_PC     1
#else
	#define AX_CPU_X86         1
	
#endif

//os

#if _WIN64
	#define AX_OS_WIN64     1
	#define AX_OS_WINDOWS	1
#elif _WIN32
	#define AX_OS_WIN32     1
	#define AX_OS_WINDOWS	1
#elif _WINCE
	#define AX_OS_WIN_CE     1
	#define AX_OS_WINDOWS	1
#endif
