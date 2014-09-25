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

AntiFreeze *g_AntiFreeze = nullptr;
Killstreaks *g_Killstreaks = nullptr;
LoadoutIcons *g_LoadoutIcons = nullptr;
LocalPlayer *g_LocalPlayer = nullptr;
MedigunInfo *g_MedigunInfo = nullptr;
MultiPanel *g_MultiPanel = nullptr;
PlayerAliases *g_PlayerAliases = nullptr;
PlayerOutlines *g_PlayerOutlines = nullptr;
ProjectileOutlines *g_ProjectileOutlines = nullptr;
StatusIcons *g_StatusIcons = nullptr;
TeamOverrides *g_TeamOverrides = nullptr;

static int doPostScreenSpaceEffectsHook;

int Detour_GetLocalPlayerIndex() {
	if (g_LocalPlayer) {
		if (g_LocalPlayer->IsEnabled()) {
			return g_LocalPlayer->GetLocalPlayerIndexOverride();
		}
	}

	return Funcs::CallFunc_GetLocalPlayerIndex();
}

void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage) {
	if (!doPostScreenSpaceEffectsHook && Interfaces::GetClientMode()) {
		doPostScreenSpaceEffectsHook = Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(Interfaces::GetClientMode(), Hook_IClientMode_DoPostScreenSpaceEffects);
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

		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

		for (int i = 0; i < maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);
		
			if (!entity) {
				continue;
			}

			if (g_AntiFreeze) {
				g_AntiFreeze->ProcessEntity(entity);
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

			if (g_ProjectileOutlines) {
				g_ProjectileOutlines->ProcessEntity(entity);
			}
		}

		if (g_AntiFreeze) {
			g_AntiFreeze->PostEntityUpdate();
		}

		if (g_Killstreaks) {
			g_Killstreaks->PostEntityUpdate();
		}

		if (g_LoadoutIcons) {
			if (g_LoadoutIcons->IsEnabled()) {
				g_LoadoutIcons->PostEntityUpdate();
			}
		}

		if (g_LocalPlayer) {
			if (g_LocalPlayer->IsEnabled()) {
				g_LocalPlayer->PostEntityUpdate();
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

bool Hook_IClientMode_DoPostScreenSpaceEffects(const CViewSetup *pSetup) {
	if (g_PlayerOutlines) {
		if (g_PlayerOutlines->IsEnabled()) {
			g_PlayerOutlines->PreGlowRender(pSetup);
		}
	}

	if (g_ProjectileOutlines) {
		if (g_ProjectileOutlines->IsEnabled()) {
			g_ProjectileOutlines->PreGlowRender(pSetup);
		}
	}

	g_GlowObjectManager.RenderGlowEffects(pSetup);

	RETURN_META_VALUE(MRES_OVERRIDE, true);
}

bool Hook_IGameEventManager2_FireEvent(IGameEvent *event, bool bDontBroadcast) {
	IGameEvent *newEvent = Interfaces::pGameEventManager->DuplicateEvent(event);
	Interfaces::pGameEventManager->FreeEvent(event);

	if (g_Killstreaks) {
		g_Killstreaks->FireEvent(newEvent);
	}

	RETURN_META_VALUE_NEWPARAMS(MRES_HANDLED, false, &IGameEventManager2::FireEvent, (newEvent, bDontBroadcast));
}

bool Hook_IGameEventManager2_FireEventClientSide(IGameEvent *event) {
	IGameEvent *newEvent = Interfaces::pGameEventManager->DuplicateEvent(event);
	Interfaces::pGameEventManager->FreeEvent(event);

	if (g_Killstreaks) {
		g_Killstreaks->FireEvent(newEvent);
	}

	RETURN_META_VALUE_NEWPARAMS(MRES_HANDLED, false, &IGameEventManager2::FireEventClientSide, (newEvent));
}

void Hook_IPanel_PaintTraverse_Pre(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible()) {
		RETURN_META(MRES_IGNORED);
	}

	if (g_AntiFreeze) {
		if (g_AntiFreeze->IsEnabled()) {
			g_AntiFreeze->Paint(vguiPanel);
		}
	}

	if (g_MedigunInfo) {
		if (g_MedigunInfo->IsEnabled()) {
			g_MedigunInfo->Paint(vguiPanel);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Hook_IPanel_PaintTraverse_Post(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (g_LoadoutIcons) {
		if (g_LoadoutIcons->IsEnabled()) {
			g_LoadoutIcons->Paint(vguiPanel);
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

	if (g_TeamOverrides) {
		if (g_TeamOverrides->IsEnabled()) {
			g_TeamOverrides->InterceptMessage(vguiPanel, params, ifromPanel);
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

	if (!Funcs::Load()) {
		Warning("[%s] Unable to initialize hooking!", PLUGIN_DESC);
		return false;
	}

	Funcs::AddDetour_GetLocalPlayerIndex(Detour_GetLocalPlayerIndex);
	
	Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify);
	Funcs::AddHook_IGameEventManager2_FireEvent(Interfaces::pGameEventManager, Hook_IGameEventManager2_FireEvent);
	Funcs::AddHook_IGameEventManager2_FireEventClientSide(Interfaces::pGameEventManager, Hook_IGameEventManager2_FireEventClientSide);
	Funcs::AddHook_IPanel_PaintTraverse_Pre(g_pVGuiPanel, Hook_IPanel_PaintTraverse_Pre);
	Funcs::AddHook_IPanel_PaintTraverse_Post(g_pVGuiPanel, Hook_IPanel_PaintTraverse_Post);
	Funcs::AddHook_IPanel_SendMessage(g_pVGuiPanel, Hook_IPanel_SendMessage);
	Funcs::AddHook_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo);
	
	ConVar_Register();

	g_AntiFreeze = new AntiFreeze();
	g_Killstreaks = new Killstreaks();
	g_LoadoutIcons = new LoadoutIcons();
	g_LocalPlayer = new LocalPlayer();
	g_MedigunInfo = new MedigunInfo();
	g_MultiPanel = new MultiPanel();
	g_PlayerAliases = new PlayerAliases();
	g_PlayerOutlines = new PlayerOutlines();
	g_ProjectileOutlines = new ProjectileOutlines();
	g_StatusIcons = new StatusIcons();
	g_TeamOverrides = new TeamOverrides();
	
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
	delete g_AntiFreeze;
	delete g_Killstreaks;
	delete g_LoadoutIcons;
	delete g_LocalPlayer;
	delete g_MedigunInfo;
	delete g_MultiPanel;
	delete g_PlayerAliases;
	delete g_PlayerOutlines;
	delete g_ProjectileOutlines;
	delete g_StatusIcons;
	delete g_TeamOverrides;

	Funcs::Unload();

	ConVar_Unregister();
	Interfaces::Unload();
}

void StatusSpecPlugin::Pause(void) {
	Funcs::Pause();
}

void StatusSpecPlugin::UnPause(void) {
	Funcs::Unpause();
}

const char *StatusSpecPlugin::GetPluginDescription(void) {
	return PLUGIN_DESC;
}

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