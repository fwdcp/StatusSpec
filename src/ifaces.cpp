/**
 *  ifaces.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "ifaces.h"

#include "cbase.h"
#include "cdll_int.h"
#include "engine/ivmodelinfo.h"
#include "entitylist_base.h"
#include "icliententitylist.h"
#include "igameevents.h"
#include "iprediction.h"
#include "ivrenderview.h"
#include "steam/steam_api.h"
#include "teamplayroundbased_gamerules.h"
#include "toolframework/iclientenginetools.h"
#include "toolframework/ienginetool.h"
#include "tier3/tier3.h"
#include "vgui_controls/Controls.h"

#include "exceptions.h"
#include "gamedata.h"

IBaseClientDLL *Interfaces::pClientDLL = nullptr;
IClientEngineTools *Interfaces::pClientEngineTools = nullptr;
IClientEntityList *Interfaces::pClientEntityList = nullptr;
IVEngineClient *Interfaces::pEngineClient = nullptr;
IEngineTool *Interfaces::pEngineTool = nullptr;
IGameEventManager2 *Interfaces::pGameEventManager = nullptr;
IPrediction *Interfaces::pPrediction = nullptr;
IVModelInfoClient *Interfaces::pModelInfoClient = nullptr;
IVRenderView *Interfaces::pRenderView = nullptr;
CSteamAPIContext *Interfaces::pSteamAPIContext = nullptr;

bool Interfaces::steamLibrariesAvailable = false;
bool Interfaces::vguiLibrariesAvailable = false;

CBaseEntityList *g_pEntityList;

IClientMode *Interfaces::GetClientMode() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!pointer) {
		pointer = SignatureScan("client", CLIENTMODE_SIG, CLIENTMODE_MASK) + CLIENTMODE_OFFSET;

		if (!pointer) {
			throw bad_pointer("IClientMode");
		}
	}

	if (!**(IClientMode***)pointer) {
		throw bad_pointer("IClientMode");
	}

	return **(IClientMode***)(pointer);
#else
	throw bad_pointer("IClientMode");

	return nullptr;
#endif
}

C_HLTVCamera *Interfaces::GetHLTVCamera() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!pointer) {
		pointer = SignatureScan("client", HLTVCAMERA_SIG, HLTVCAMERA_MASK) + HLTVCAMERA_OFFSET;

		if (!pointer) {
			throw bad_pointer("C_HLTVCamera");
		}
	}

	if (!*(C_HLTVCamera**)pointer) {
		throw bad_pointer("C_HLTVCamera");
	}

	return *(C_HLTVCamera**)(pointer);
#else
	throw bad_pointer("C_HLTVCamera");

	return nullptr;
#endif
}

void Interfaces::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	ConnectTier1Libraries(&interfaceFactory, 1);
	ConnectTier2Libraries(&interfaceFactory, 1);
	ConnectTier3Libraries(&interfaceFactory, 1);
	
	vguiLibrariesAvailable = vgui::VGui_InitInterfacesList("statusspec", &interfaceFactory, 1);
	
	pClientEngineTools = (IClientEngineTools *)interfaceFactory(VCLIENTENGINETOOLS_INTERFACE_VERSION, nullptr);
	pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr);
	pEngineTool = (IEngineTool *)interfaceFactory(VENGINETOOL_INTERFACE_VERSION, nullptr);
	pGameEventManager = (IGameEventManager2 *)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr);
	pModelInfoClient = (IVModelInfoClient *)interfaceFactory(VMODELINFO_CLIENT_INTERFACE_VERSION, nullptr);
	pRenderView = (IVRenderView *)interfaceFactory(VENGINE_RENDERVIEW_INTERFACE_VERSION, nullptr);
	
	CreateInterfaceFn gameClientFactory;
	pEngineTool->GetClientFactory(gameClientFactory);
	
	pClientDLL = (IBaseClientDLL*)gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, nullptr);
	pClientEntityList = (IClientEntityList*)gameClientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, nullptr);
	pPrediction = (IPrediction *)gameClientFactory(VCLIENT_PREDICTION_INTERFACE_VERSION, nullptr);

	pSteamAPIContext = new CSteamAPIContext();
	steamLibrariesAvailable = SteamAPI_InitSafe() && pSteamAPIContext->Init();

	g_pEntityList = dynamic_cast<CBaseEntityList *>(Interfaces::pClientEntityList);
}

void Interfaces::Unload() {
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
	
	pSteamAPIContext->Clear();
	
	pClientDLL = nullptr;
	pClientEntityList = nullptr;
	pEngineClient = nullptr;
	pGameEventManager = nullptr;
	pRenderView = nullptr;
}