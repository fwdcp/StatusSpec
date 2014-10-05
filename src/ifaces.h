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

#include "stdafx.h"

#include <sstream>
#include <string>

#define CLIENT_DLL
#define VERSION_SAFE_STEAM_API_INTERFACES

#include "cbase.h"
#include "cdll_int.h"
#include "c_playerresource.h"
#include "engine/ivmodelinfo.h"
#include "entitylist_base.h"
#include "filesystem.h"
#include "icliententitylist.h"
#include "iclientmode.h"
#include "igameevents.h"
#include "igameresources.h"
#include "ivrenderview.h"
#include "steam/steam_api.h"
#include "tier3/tier3.h"
#include "filesystem_init.h"
#include "vgui_controls/Controls.h"

#include "entities.h"

#if defined __linux__
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#elif defined _WIN32
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#endif

#if defined _WIN32
#define CLIENT_MODULE_FILE "tf/bin/client.dll"
#define CLIENT_MODULE_SIZE 0xC74EC0
#define GAMERESOURCES_SIG "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05"
#define GAMERESOURCES_MASK "x????xxxxx"
#define CLIENTMODE_SIG "\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8B\xC8"
#define CLIENTMODE_MASK "xx????????x????x????xx"
#define CLIENTMODE_OFFSET 2
#define noexcept _NOEXCEPT
#elif defined __APPLE__
#define CLIENT_MODULE_FILE "tf/bin/client.dylib"
#elif defined __linux__
#define CLIENT_MODULE_FILE "tf/bin/client.so"
#endif

class bad_pointer : public std::exception {
public:
	bad_pointer(const char *type) noexcept;
	virtual const char* what() const noexcept;
private:
	const char *pointerType;
};

inline bad_pointer::bad_pointer(const char *type) noexcept {
	pointerType = type;
}

inline const char *bad_pointer::what() const noexcept {
	std::string s;
	std::stringstream ss;

	ss << "Invalid pointer to " << pointerType << "!\n";
	ss >> s;

	return s.c_str();
}

class Interfaces {
	public:
		static bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
		static void Unload();
		static IBaseClientDLL *pClientDLL;
		static IClientEntityList *pClientEntityList;
		static IVEngineClient *pEngineClient;
		static IFileSystem *pFileSystem;
		static IGameEventManager2 *pGameEventManager;
		static IVModelInfoClient *pModelInfoClient;
		static IVRenderView *pRenderView;
		static CSteamAPIContext *pSteamAPIContext;
		static IClientMode *GetClientMode();
		static IGameResources *GetGameResources();
	private:
		static CDllDemandLoader *pClientModule;
};