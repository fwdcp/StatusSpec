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

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define CLIENT_DLL

#include "tier3/tier3.h"
#include "cdll_int.h"
#include "icliententitylist.h"
#include "vgui/IVGui.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "igameresources.h"

#ifdef _POSIX
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#else
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#endif

bool LoadInterfaces(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
void UnloadInterfaces();

extern CDllDemandLoader clientModule;

extern IBaseClientDLL* g_pClientDll;
extern IClientEntityList* g_pClientEntityList;
extern IVEngineClient* g_pEngineClient;
extern IGameResources* g_pGameResources;