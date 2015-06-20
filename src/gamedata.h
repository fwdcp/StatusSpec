/**
 *  gamedata.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#undef GetClassName
#undef PostMessage
#undef SendMessage
#endif

// DLL loading info
#if defined _WIN32
#define GetHandleOfModule(szModuleName) GetModuleHandle((std::string(szModuleName) + ".dll").c_str())
#elif defined __linux__
#define GetHandleOfModule(szModuleName) dlopen((std::string(szModuleName) + ".so").c_str(), RTLD_NOLOAD)
#endif

// signatures
#if defined _WIN32
#define CLIENTMODE_SIG "\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8B\xC8"
#define CLIENTMODE_MASK "xx????????x????x????xx"
#define CLIENTMODE_OFFSET 2
#define HLTVCAMERA_SIG "\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x00"
#define HLTVCAMERA_MASK "x????x????x????xx????xxxxxx????x"
#define HLTVCAMERA_OFFSET 1
#define GETLOCALPLAYERINDEX_SIG "\xE8\x00\x00\x00\x00\x85\xC0\x74\x08\x8D\x48\x08\x8B\x01\xFF\x60\x24\x33\xC0\xC3"
#define GETLOCALPLAYERINDEX_MASK "x????xxxxxxxxxxxxxxx"
#define SETCAMERAANGLE_SIG "\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x8D\x56\x00\xD9\x00\xD9\x1A\xD9\x40\x00\xD9\x5A\x00\xD9\x40\x00\x52"
#define SETCAMERAANGLE_MASK "xxxxxxxxxxx?xxxxxx?xx?xx?x"
#define SETMODE_SIG "\x55\x8B\xEC\x8B\x45\x08\x53\x56\x8B\xF1\x8B\x5E\x00"
#define SETMODE_MASK "xxxxxxxxxxxx?"
#define SETMODELINDEX_SIG "\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x57\x66\x89\x86\x00\x00\x00\x00"
#define SETMODELINDEX_MASK "xxxxxxxxxxxxx????"
#define SETMODELPOINTER_SIG "\x55\x8B\xEC\x56\x8B\xF1\x57\x8B\x7D\x08\x3B\x7E\x00\x74\x00"
#define SETMODELPOINTER_MASK "xxxxxxxxxxxx?x?"
#define SETPRIMARYTARGET_SIG "\x55\x8B\xEC\x8B\x45\x08\x83\xEC\x00\x53\x56\x8B\xF1"
#define SETPRIMARYTARGET_MASK "xxxxxxxx?xxxx"
#endif

class C_BaseEntity;
class C_HLTVCamera;
struct model_t;

// C_TFPlayer offsets
#if defined _WIN32
#define OFFSET_GETGLOWEFFECTCOLOR 224
#define OFFSET_GETFOV 269
#endif

// non-member function types
typedef int(*GLPI_t)(void);

// member function types
#if defined _WIN32
typedef void(__thiscall *SCA_t)(C_HLTVCamera *, QAngle &);
typedef void(__thiscall *SM_t)(C_HLTVCamera *, int);
typedef void(__thiscall *SMI_t)(C_BaseEntity *, int);
typedef void(__thiscall *SMP_t)(C_BaseEntity *, const model_t *);
typedef void(__thiscall *SPT_t)(C_HLTVCamera *, int);
typedef void(__fastcall *SMH_t)(C_HLTVCamera *, void *, int);
typedef void(__fastcall *SMIH_t)(C_BaseEntity *, void *, int);
typedef void(__fastcall *SMPH_t)(C_BaseEntity *, void *, const model_t *);
typedef void(__fastcall *SPTH_t)(C_HLTVCamera *, void *, int);
#endif

inline bool DataCompare(const BYTE* pData, const BYTE* bSig, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}

	return (*szMask) == NULL;
}

inline DWORD FindPattern(DWORD dwAddress, DWORD dwSize, BYTE* pbSig, const char* szMask) {
	for (DWORD i = NULL; i < dwSize; i++)
	{
		if (DataCompare((BYTE*)(dwAddress + i), pbSig, szMask))
			return (DWORD)(dwAddress + i);
	}

	return 0;
}

inline DWORD SignatureScan(const char *moduleName, const char *signature, const char *mask) {
#if defined _WIN32
	MODULEINFO clientModInfo;
	const HMODULE clientModule = GetHandleOfModule(moduleName);
	GetModuleInformation(GetCurrentProcess(), clientModule, &clientModInfo, sizeof(MODULEINFO));

	return FindPattern((DWORD)clientModule, clientModInfo.SizeOfImage, (PBYTE)signature, mask);
#endif
}