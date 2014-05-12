/*
 *  vfuncs.h
 *  WebSpec project
 *  Modified for AdvSpec, used in StatusSpec
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#define CLIENT_DLL

class WSEmptyClass {};

#if defined (_POSIX) // client.dylib
enum vfuncIndices {
	Index_SendMessage = 39,
	Index_PaintTraverse = 42
};
#else // client.dll
enum vfuncIndices {
	Index_SendMessage = 38,
	Index_PaintTraverse = 41
};
#endif

#ifndef _POSIX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
//typedef unsigned long DWORD;
#endif

extern DWORD *HookVFunc(DWORD *vtable, int index, DWORD *newFunction);
