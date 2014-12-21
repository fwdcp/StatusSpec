/**
 *  gamedata.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include <string>

#if defined _WIN32
#include <Windows.h>
#include <Psapi.h>
#endif

// DLL loading info
#if defined _WIN32
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandle((std::string(szModuleName) + ".dll").c_str())
#elif defined __linux__
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen((std::string(szModuleName) + ".so").c_str(), RTLD_NOLOAD)
#endif

// client DLL info
#if defined _WIN32
#define CLIENT_MODULE_FILE "tf/bin/client.dll"
#elif defined __APPLE__
#define CLIENT_MODULE_FILE "tf/bin/client.dylib"
#elif defined __linux__
#define CLIENT_MODULE_FILE "tf/bin/client.so"
#endif

// signatures
#if defined _WIN32
#define GAMERESOURCES_SIG "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05"
#define GAMERESOURCES_MASK "x????xxxxx"
#define CLIENTMODE_SIG "\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8B\xC8"
#define CLIENTMODE_MASK "xx????????x????x????xx"
#define CLIENTMODE_OFFSET 2
#define HLTVCAMERA_SIG "\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x00"
#define HLTVCAMERA_MASK "x????x????x????xx????xxxxxx????x"
#define HLTVCAMERA_OFFSET 1
#define GPGLOBALS_SIG "\xA3\x00\x00\x00\x00\x8D\x45\x08\x6A\x01"
#define GPGLOBALS_MASK "x????xxxxx"
#define GPGLOBALS_OFFSET 1
#define GETLOCALPLAYERINDEX_SIG "\xE8\x00\x00\x00\x00\x85\xC0\x74\x08\x8D\x48\x08\x8B\x01\xFF\x60\x24\x33\xC0\xC3"
#define GETLOCALPLAYERINDEX_MASK "x????xxxxxxxxxxxxxxx"
#define SETMODEL_SIG "\x55\x8B\xEC\x8B\x55\x08\x56\x57\x8B\xF9\x85\xD2"
#define SETMODEL_MASK "xxxxxxxxxxxx"
#define SETMODELINDEX_SIG "\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x57\x66\x89\x86\x00\x00\x00\x00"
#define SETMODELINDEX_MASK "xxxxxxxxxxxxx????"
#define SETMODELPOINTER_SIG "\x55\x8B\xEC\x56\x8B\xF1\x57\x8B\x7D\x08\x3B\x7E\x00\x74\x00"
#define SETMODELPOINTER_MASK "xxxxxxxxxxxx?x?"
#define SETPRIMARYTARGET_SIG "\x55\x8B\xEC\x8B\x45\x08\x83\xEC\x00\x53\x56\x8B\xF1"
#define SETPRIMARYTARGET_MASK "xxxxxxxx?xxxx"
#endif

class C_BaseEntity;
class C_HLTVCamera;
class C_TFPlayer;
class IGameResources;
struct model_t;

// C_TFPlayer offsets
#if defined _WIN32
#define OFFSET_GETHEALTH 106
#define OFFSET_GETMAXHEALTH 107
#define OFFSET_ISALIVE 137
#define OFFSET_GETGLOWEFFECTCOLOR 224
#define OFFSET_UPDATEGLOWEFFECT 225
#define OFFSET_DESTROYGLOWEFFECT 226
#define OFFSET_CALCVIEW 230
#define OFFSET_GETOBSERVERMODE 241
#define OFFSET_GETOBSERVERTARGET 242
#define OFFSET_GETFOV 269
#endif

// non-member function types
typedef int(*GLPI_t)(void);
typedef IGameResources *(*GGR_t)(void);

// member function types
#if defined _WIN32
typedef void(__thiscall *SMI_t)(C_BaseEntity *, int);
typedef void(__thiscall *SMP_t)(C_BaseEntity *, const model_t *);
typedef void(__thiscall *SPT_t)(C_HLTVCamera *, int);
typedef void(__fastcall *SMIH_t)(C_BaseEntity *, void *, int);
typedef void(__fastcall *SMPH_t)(C_BaseEntity *, void *, const model_t *);
#endif