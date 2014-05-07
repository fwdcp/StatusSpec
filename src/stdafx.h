// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _POSIX
//#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>

typedef unsigned char BYTE;
typedef BYTE *PBYTE;

#define __fastcall
#define __thiscall
#endif

#ifndef _POSIX
// Only available on windows...
#include <intrin.h>
#else
#define __debugbreak()
#endif

#include <assert.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef _POSIX
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#else
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#endif
