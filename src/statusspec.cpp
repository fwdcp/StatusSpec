/*
 *  statusspec.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusspec.h"

SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_HOOK1_void(IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t);
SH_DECL_HOOK1(IGameResources, GetPlayerName, SH_NOATTRIB, 0, const char *, int);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage) {
	static IGameResources* gameResources = NULL;
	static int getPlayerNameHook;

	if (gameResources != Interfaces::GetGameResources()) {
		if (getPlayerNameHook) {
			SH_REMOVE_HOOK_ID(getPlayerNameHook);
			getPlayerNameHook = 0;
		}

		gameResources = Interfaces::GetGameResources();
		
		if (gameResources) {
			getPlayerNameHook = SH_ADD_HOOK(IGameResources, GetPlayerName, gameResources, Hook_IGameResources_GetPlayerName, true);
		}
	}

	if (curStage == FRAME_RENDER_START) {
		if (g_LoadoutIcons) {
			g_LoadoutIcons->Update();
		}

		if (g_MedigunInfo) {
			g_MedigunInfo->Update();
		}

		if (g_StatusIcons) {
			g_StatusIcons->Update();
		}
	}

	RETURN_META(MRES_IGNORED);
}

const char * Hook_IGameResources_GetPlayerName(int client) {
	if (g_PlayerAlias) {
		RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAlias->GetPlayerNameOverride(client));
	}
	else {
		RETURN_META_VALUE(MRES_IGNORED, "");
	}
}
	
void Hook_IPanel_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible()) {
		return;
	}

	if (g_AntiFreeze) {
		g_AntiFreeze->Paint(vguiPanel);
	}

	if (g_LoadoutIcons) {
		g_LoadoutIcons->Paint(vguiPanel);
	}

	if (g_MedigunInfo) {
		g_MedigunInfo->Paint(vguiPanel);
	}

	if (g_StatusIcons) {
		g_StatusIcons->Paint(vguiPanel);
	}
}

void Hook_IPanel_SendMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	if (g_LoadoutIcons) {
		g_LoadoutIcons->InterceptMessage(vguiPanel, params, ifromPanel);
	}
		
	if (g_StatusIcons) {
		g_StatusIcons->InterceptMessage(vguiPanel, params, ifromPanel);
	}
}

bool Hook_IVEngineClient_GetPlayerInfo(int ent_num, player_info_t *pinfo) {
	if (g_PlayerAlias) {
		RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAlias->GetPlayerInfoOverride(ent_num, pinfo));
	}
	else {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}
}

// The plugin is a static singleton that is exported as an interface
StatusSpecPlugin g_StatusSpecPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(StatusSpecPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_StatusSpecPlugin);

StatusSpecPlugin::StatusSpecPlugin()
{
}

StatusSpecPlugin::~StatusSpecPlugin()
{
}

bool StatusSpecPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	if (!Interfaces::Load(interfaceFactory, gameServerFactory)) {
		Warning("[%s] Unable to load required libraries!\n", PLUGIN_DESC);
		return false;
	}

	if (!Entities::PrepareOffsets()) {
		Warning("[%s] Unable to determine proper offsets!\n", PLUGIN_DESC);
		return false;
	}
	
	SH_ADD_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_ADD_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_ADD_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);
	
	ConVar_Register();
	
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
	SH_REMOVE_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_REMOVE_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_REMOVE_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_REMOVE_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);

	ConVar_Unregister();
	Interfaces::Unload();
}

void StatusSpecPlugin::Pause(void) {}
void StatusSpecPlugin::UnPause(void) {}
const char *StatusSpecPlugin::GetPluginDescription(void) { return PLUGIN_DESC; }
void StatusSpecPlugin::LevelInit(char const *pMapName) {}
void StatusSpecPlugin::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
void StatusSpecPlugin::GameFrame(bool simulating) {}
void StatusSpecPlugin::LevelShutdown(void) {}
void StatusSpecPlugin::ClientActive(edict_t *pEntity) {}
void StatusSpecPlugin::ClientDisconnect(edict_t *pEntity) {}
void StatusSpecPlugin::ClientPutInServer(edict_t *pEntity, char const *playername) {}
void StatusSpecPlugin::SetCommandClient(int index) {}
void StatusSpecPlugin::ClientSettingsChanged(edict_t *pEdict) {}
PLUGIN_RESULT StatusSpecPlugin::ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::ClientCommand(edict_t *pEntity, const CCommand &args) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return PLUGIN_CONTINUE; }
void StatusSpecPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}
void StatusSpecPlugin::OnEdictAllocated(edict_t *edict) {}
void StatusSpecPlugin::OnEdictFreed(const edict_t *edict) {}