/**
 *  ifaces.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "interface.h"

class C_HLTVCamera;
class CSteamAPIContext;
class IBaseClientDLL;
class IClientEngineTools;
class IClientEntityList;
class IClientMode;
class IEngineTool;
class IGameEventManager2;
class IPrediction;
class IVEngineClient;
class IVModelInfoClient;
class IVRenderView;

class Interfaces {
	public:
		static void Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
		static void Unload();

		static IBaseClientDLL *pClientDLL;
		static IClientEngineTools *pClientEngineTools;
		static IClientEntityList *pClientEntityList;
		static IVEngineClient *pEngineClient;
		static IEngineTool *pEngineTool;
		static IGameEventManager2 *pGameEventManager;
		static IVModelInfoClient *pModelInfoClient;
		static IPrediction *pPrediction;
		static IVRenderView *pRenderView;
		static CSteamAPIContext *pSteamAPIContext;

		static bool steamLibrariesAvailable;
		static bool vguiLibrariesAvailable;

		static IClientMode *GetClientMode();
		static C_HLTVCamera *GetHLTVCamera();
};