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

#include "interface.h"

class C_HLTVCamera;
class CGlobalVarsBase;
class CSteamAPIContext;
class IBaseClientDLL;
class IClientEntityList;
class IClientMode;
class IFileSystem;
class IGameEventManager2;
class IGameResources;
class IVEngineClient;
class IVModelInfoClient;
class IVRenderView;

class Interfaces {
	public:
		static void Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
		static void Unload();

		static IBaseClientDLL *pClientDLL;
		static IClientEntityList *pClientEntityList;
		static IVEngineClient *pEngineClient;
		static IFileSystem *pFileSystem;
		static IGameEventManager2 *pGameEventManager;
		static IVModelInfoClient *pModelInfoClient;
		static IVRenderView *pRenderView;
		static CSteamAPIContext *pSteamAPIContext;

		static bool steamLibrariesAvailable;
		static bool vguiLibrariesAvailable;

		static IClientMode *GetClientMode();
		static IGameResources *GetGameResources();
		static CGlobalVarsBase *GetGlobalVars();
		static C_HLTVCamera *GetHLTVCamera();
	private:
		static CDllDemandLoader *pClientModule;
};