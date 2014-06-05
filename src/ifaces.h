/**
 *  ifaces.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#if defined _WIN32
#define strtoull _strtoui64
#endif

#include "stdafx.h"

#define CLIENT_DLL
#define VERSION_SAFE_STEAM_API_INTERFACES

#include "tier3/tier3.h"
#include "steam/steam_api.h"
#include "cdll_int.h"
#include "icliententitylist.h"
#include "igameresources.h"
#include "iclientmode.h"
#include "vgui_controls/Controls.h"

#if defined __linux__
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#elif defined _WIN32
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#endif

#if defined _WIN32
#define CLIENT_MODULE_SIZE 0xC74EC0
#define GAMERESOURCES_SIG "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05"
#define GAMERESOURCES_MASK "x????xxxxx"
#define CLIENTMODE_SIG "\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8B\xC8"
#define CLIENTMODE_MASK "xx????????x????x????xx"
#define CLIENTMODE_OFFSET 2
#endif

class Interfaces {
	public:
		static bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
		static void Unload();
		static IBaseClientDLL* pClientDLL;
		static IClientEntityList* pClientEntityList;
		static IVEngineClient* pEngineClient;
		static CSteamAPIContext* pSteamAPIContext; 
		static IGameResources* GetGameResources();
		static IClientMode* GetClientMode();
	private:
		static CDllDemandLoader *pClientModule;
};