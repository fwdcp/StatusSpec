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

AntiFreeze *g_AntiFreeze = NULL;
LoadoutIcons *g_LoadoutIcons = NULL;
MedigunInfo *g_MedigunInfo = NULL;
PlayerAliases *g_PlayerAliases = NULL;
PlayerOutlines *g_PlayerOutlines = NULL;
StatusIcons *g_StatusIcons = NULL;

static IGameResources* gameResources = NULL;
static int getPlayerNameHook;

SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_HOOK1_void(C_BaseCombatCharacter, OnDataChanged, SH_NOATTRIB, 0, DataUpdateType_t);
SH_DECL_MANUALHOOK3_void(C_TFPlayer_GetGlowEffectColor, OFFSET_GETGLOWEFFECTCOLOR, 0, 0, float *, float *, float *);
SH_DECL_MANUALHOOK0_void(C_TFPlayer_UpdateGlowEffect, OFFSET_UPDATEGLOWEFFECT, 0, 0);
SH_DECL_HOOK1_void(IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t);
SH_DECL_HOOK1(IGameResources, GetPlayerName, SH_NOATTRIB, 0, const char *, int);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

int AddHook_C_BaseCombatCharacter_OnDataChanged(C_BaseCombatCharacter *baseCombatCharacter) {
	return SH_ADD_HOOK(C_BaseCombatCharacter, OnDataChanged, baseCombatCharacter, Hook_C_BaseCombatCharacter_OnDataChanged, true);
}

int AddHook_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *tfPlayer) {
	return SH_ADD_MANUALHOOK(C_TFPlayer_GetGlowEffectColor, tfPlayer, Hook_C_TFPlayer_GetGlowEffectColor, false);
}

void Call_C_TFPlayer_UpdateGlowEffect(C_TFPlayer *tfPlayer) {
	SH_MCALL(tfPlayer, C_TFPlayer_UpdateGlowEffect)();
}

void Hook_C_BaseCombatCharacter_OnDataChanged(DataUpdateType_t type) {
	if (g_PlayerOutlines) {
		if (g_PlayerOutlines->IsEnabled()) {
			C_BaseCombatCharacter *baseCombatCharacter = META_IFACEPTR(C_BaseCombatCharacter);

			if (g_PlayerOutlines->DataChangeOverride(baseCombatCharacter)) {
				RETURN_META(MRES_HANDLED);
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Hook_C_TFPlayer_GetGlowEffectColor(float *r, float *g, float *b) {
	if (g_PlayerOutlines) {
		if (g_PlayerOutlines->IsEnabled()) {
			C_TFPlayer *tfPlayer = META_IFACEPTR(C_TFPlayer);

			if (g_PlayerOutlines->GetGlowEffectColorOverride(tfPlayer, r, g, b)) {
				RETURN_META(MRES_SUPERCEDE);
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage) {
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
			if (g_LoadoutIcons->IsEnabled()) {
				g_LoadoutIcons->PreEntityUpdate();
			}
		}

		if (g_MedigunInfo) {
			if (g_MedigunInfo->IsEnabled()) {
				g_MedigunInfo->PreEntityUpdate();
			}
		}

		if (g_StatusIcons) {
			if (g_StatusIcons->IsEnabled()) {
				g_StatusIcons->PreEntityUpdate();
			}
		}

		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

		for (int i = 0; i < maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);
		
			if (!entity) {
				continue;
			}

			if (g_LoadoutIcons) {
				if (g_LoadoutIcons->IsEnabled()) {
					g_LoadoutIcons->ProcessEntity(entity);
				}
			}

			if (g_MedigunInfo) {
				if (g_MedigunInfo->IsEnabled()) {
					g_MedigunInfo->ProcessEntity(entity);
				}
			}

			if (g_PlayerOutlines) {
				g_PlayerOutlines->ProcessEntity(entity);
			}

			if (g_StatusIcons) {
				if (g_StatusIcons->IsEnabled()) {
					g_StatusIcons->ProcessEntity(entity);
				}
			}
		}

		if (g_LoadoutIcons) {
			if (g_LoadoutIcons->IsEnabled()) {
				g_LoadoutIcons->PostEntityUpdate();
			}
		}

		if (g_MedigunInfo) {
			if (g_MedigunInfo->IsEnabled()) {
				g_MedigunInfo->PostEntityUpdate();
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

const char * Hook_IGameResources_GetPlayerName(int client) {
	if (g_PlayerAliases) {
		if (g_PlayerAliases->IsEnabled()) {
			RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAliases->GetPlayerNameOverride(client));
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, "");
}
	
void Hook_IPanel_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible()) {
		return;
	}

	if (g_AntiFreeze) {
		if (g_AntiFreeze->IsEnabled()) {
			g_AntiFreeze->Paint(vguiPanel);
		}
	}

	if (g_LoadoutIcons) {
		if (g_LoadoutIcons->IsEnabled()) {
			g_LoadoutIcons->Paint(vguiPanel);
		}
	}

	if (g_MedigunInfo) {
		if (g_MedigunInfo->IsEnabled()) {
			g_MedigunInfo->Paint(vguiPanel);
		}
		else {
			g_MedigunInfo->NoPaint(vguiPanel);
		}
	}

	if (g_StatusIcons) {
		if (g_StatusIcons->IsEnabled()) {
			g_StatusIcons->Paint(vguiPanel);
		}
		else {
			g_StatusIcons->NoPaint(vguiPanel);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Hook_IPanel_SendMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	if (g_LoadoutIcons) {
		if (g_LoadoutIcons->IsEnabled()) {
			g_LoadoutIcons->InterceptMessage(vguiPanel, params, ifromPanel);
		}
	}
	
	if (g_StatusIcons) {
		if (g_StatusIcons->IsEnabled()) {
			g_StatusIcons->InterceptMessage(vguiPanel, params, ifromPanel);
		}
	}

	RETURN_META(MRES_IGNORED);
}

bool Hook_IVEngineClient_GetPlayerInfo(int ent_num, player_info_t *pinfo) {
	if (g_PlayerAliases) {
		if (g_PlayerAliases->IsEnabled()) {
			RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAliases->GetPlayerInfoOverride(ent_num, pinfo));
		}
	}
	
	RETURN_META_VALUE(MRES_IGNORED, false);
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

	g_AntiFreeze = new AntiFreeze();
	g_LoadoutIcons = new LoadoutIcons();
	g_MedigunInfo = new MedigunInfo();
	g_PlayerAliases = new PlayerAliases();
	g_PlayerOutlines = new PlayerOutlines();
	g_StatusIcons = new StatusIcons();
	
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
	delete g_AntiFreeze;
	delete g_LoadoutIcons;
	delete g_MedigunInfo;
	delete g_PlayerAliases;
	delete g_PlayerOutlines;
	delete g_StatusIcons;

	SH_REMOVE_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_REMOVE_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_REMOVE_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_REMOVE_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);

	if (getPlayerNameHook) {
		SH_REMOVE_HOOK_ID(getPlayerNameHook);
	}

	ConVar_Unregister();
	Interfaces::Unload();
}

void StatusSpecPlugin::Pause(void) {
	SH_REMOVE_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_REMOVE_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_REMOVE_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_REMOVE_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);

	if (getPlayerNameHook) {
		SH_REMOVE_HOOK_ID(getPlayerNameHook);
	}
}

void StatusSpecPlugin::UnPause(void) {
	SH_ADD_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_ADD_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_ADD_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);
}

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