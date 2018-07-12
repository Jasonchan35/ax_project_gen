#pragma once

#if ! (ax_COMPILER_CLANG | ax_COMPILER_GCC)
	#error
#endif

#if ax_COMPILER_CLANG
	#define ax_OPTIMIZE_OFF			_Pragma("clang optimize off")
#else
	#define ax_OPTIMIZE_OFF			_Pragma("GCC optimize(\"O0\")")
#endif

#define	ax_FUNC_NAME			__FUNCTION__
#define ax_PRETTY_FUNC_NAME		__PRETTY_FUNCTION__

#define ax_DEPRECATED( f )		f __attribute__( (deprecated) )


#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(__amd64__)
	#define ax_CPU_x86_64      1

#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
	#define ax_CPU_x86         1

#elif defined(__POWERPC__)
	#define ax_CPU_PowerPC     1

#elif defined(__arm__)
	#define ax_CPU_ARM 1

	#if __ARMEL__
		#define ax_CPU_ENDIAN_LITTLE	1
	#elif defined( __ARMEB__ )
		#define ax_CPU_ENDIAN_BIG		1
	#else
		#error unknown ARM CPU endian
	#endif

	#if __aarch64__
		#define ax_CPU_LP64		1
	#else
		#define ax_CPU_LP32		1
	#endif

#endif

//======== OS ===========

#if __linux
	#define ax_OS_Linux        1

	#if __ANDROID__
		#define ax_OS_Android		1
	#endif

#elif __FreeBSD__
	#define ax_OS_FreeBSD		1

#elif __APPLE__ && __MACH__
	#include <TargetConditionals.h>
	#if (TARGET_OF_IPHONE_SIMULATOR) || (TARGET_OS_IPHONE) || (TARGET_IPHONE)
		#define ax_OS_iOS		1
	#else
		#define ax_OS_MacOSX	1
	#endif

#elif __sun
	#define ax_OS_Solaris		1

#elif __CYGWIN__
    #define ax_OS_Cygwin        1

#elif __MINGW32__
	#define ax_OS_MinGW			1
#endif

