#pragma once

#if ! (AX_COMPILER_CLANG | AX_COMPILER_GCC)
	#error
#endif

#if AX_COMPILER_CLANG
	#define AX_OPTIMIZE_OFF			_Pragma("clang optimize off")
#else
	#define AX_OPTIMIZE_OFF			_Pragma("GCC optimize(\"O0\")")
#endif

#define	AX_FUNC_NAME			__FUNCTION__
#define AX_PRETTY_FUNC_NAME		__PRETTY_FUNCTION__

#define AX_DEPRECATED( f )		f __attribute__( (deprecated) )


#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(__amd64__)
	#define AX_CPU_X86_64      1

#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
	#define AX_CPU_X86         1

#elif defined(__POWERPC__)
	#define AX_CPU_POWER_PC     1

#elif defined(__arm__)
	#define AX_CPU_ARM 1

	#if __ARMEL__
		#define AX_CPU_ENDIAN_LITTLE	1
	#elif defined( __ARMEB__ )
		#define AX_CPU_ENDIAN_BIG		1
	#else
		#error unknown ARM CPU endian
	#endif

	#if __aarch64__
		#define AX_CPU_LP64		1
	#else
		#define AX_CPU_LP32		1
	#endif

#endif

//======== OS ===========

#if __linux
	#define AX_OS_LINUX        1

	#if __ANDROID__
		#define AX_OS_ANDROID		1
	#endif

#elif __FreeBSD__
	#define AX_OS_FREEBSD		1

#elif __APPLE__ && __MACH__
	#include <TargetConditionals.h>
	#if (TARGET_OF_IPHONE_SIMULATOR) || (TARGET_OS_IPHONE) || (TARGET_IPHONE)
		#define AX_OS_IOS		1
	#else
		#define AX_OS_MACOSX	1
	#endif

#elif __sun
	#define AX_OS_SOLARIS		1

#elif __CYGWIN__
    #define AX_OS_CYGWIN        1

#elif __MINGW32__
	#define AX_OS_MINGW			1
#endif

