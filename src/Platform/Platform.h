#pragma once

/*
libax_platform:
	- headers only
	- detect compile / OS
	- for external lib if needed to ensure using the same platform detection logic
*/

#ifdef __OBJC__
	#define ax_ObjC		1
#else
	#define ax_ObjC		0
#endif

//=========== Detect COMPILER ===============
#if defined(__clang__) 
	#define ax_COMPILER_CLANG	1
	#include "Compiler_gcc.h"

#elif defined(__GNUC__)
	#define ax_COMPILER_GCC		1
	#include "Compiler_gcc.h"

#elif defined(_MSC_VER)
	#define ax_COMPILER_VC		1
	#include "Compiler_vc.h"

#endif

#if ax_COMPILER_VC + ax_COMPILER_GCC + ax_COMPILER_CLANG != 1
    #error "Compiler should be specified"
#endif

//======== Detect CPU =============

// check CPU define
#if ax_CPU_x86_64 + ax_CPU_x86 + ax_CPU_PowerPC + ax_CPU_ARM != 1
	#error CPU should be specified
#endif

#if ax_CPU_x86_64
	#define ax_CPU_LP64				1
	#define ax_CPU_ENDIAN_LITTLE	1
#endif

#if ax_CPU_x86
	#define ax_CPU_LP32				1
	#define ax_CPU_ENDIAN_LITTLE	1
#endif

#if ax_CPU_PowerPC
	#define ax_CPU_LP32				1
	#define ax_CPU_ENDIAN_BIG		1
#endif

#if ax_CPU_ARM
	#define ax_CPU_LP32				1
#endif

#if ax_CPU_LP32 + ax_CPU_LP64 != 1
	#error CPU bits should be specified
#endif


#if ax_CPU_ENDIAN_BIG + ax_CPU_ENDIAN_LITTLE != 1
	#error CPU endian should be specified
#endif

//======== Detect OS ===============

#if ax_OS_Win32 + ax_OS_Win64 + ax_OS_WinCE \
	+ ax_OS_FreeBSD + ax_OS_Linux \
	+ ax_OS_MacOSX  + ax_OS_iOS \
	+ ax_OS_Cygwin  + ax_OS_MinGW != 1
	#error OS should be specified
#endif

//====================================

#ifndef UNICODE
	#define UNICODE
#endif

#ifndef _UNICODE
	#define _UNICODE
#endif