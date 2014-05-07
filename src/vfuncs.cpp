/*
 *  vfuncs.cpp
 *  WebSpec project
 *  Modified for AdvSpec, used in StatusSpec
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "vfuncs.h"

DWORD *HookVFunc(DWORD *vtable, int index, DWORD *newFunction)
{
	#ifndef _POSIX
	DWORD dwOldProt;
	#endif
	DWORD *oldFunc;
	
	#ifdef _POSIX
	mprotect(&vtable[index], 4, PROT_READ|PROT_WRITE|PROT_EXEC);
	#else
	VirtualProtect(&vtable[index], 4, PAGE_EXECUTE_READWRITE, &dwOldProt);
	#endif
	oldFunc=(DWORD*)vtable[index];
	vtable[index]=(DWORD)newFunction;
	#ifndef _POSIX //TODO: Restore mem protection on posix
	VirtualProtect(&vtable[index], 4, dwOldProt, &dwOldProt);
	#endif
	
	return oldFunc;
}