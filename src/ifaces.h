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

#define CLIENT_DLL
#define VERSION_SAFE_STEAM_API_INTERFACES

#include "cbase.h"
#include "cdll_int.h"
#include "c_playerresource.h"
#include "engine/ivmodelinfo.h"
#include "entitylist_base.h"
#include "filesystem.h"
#include "game/server/iplayerinfo.h"
#include "hltvcamera.h"
#include "icliententitylist.h"
#include "iclientmode.h"
#include "igameevents.h"
#include "igameresources.h"
#include "ivrenderview.h"
#include "steam/steam_api.h"
#include "tier3/tier3.h"
#include "filesystem_init.h"
#include "vgui_controls/Controls.h"

#include "common.h"
#include "exceptions.h"
#include "gamedata.h"

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
		static IPlayerInfoManager *pPlayerInfoManager;
		static IVRenderView *pRenderView;
		static CSteamAPIContext *pSteamAPIContext;
		static IClientMode *GetClientMode();
		static IGameResources *GetGameResources();
		static C_HLTVCamera *GetHLTVCamera();
	private:
		static CDllDemandLoader *pClientModule;
};