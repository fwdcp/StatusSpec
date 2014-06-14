/*
 *  statusspec.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include "entities.h"
#include "enums.h"
#include "hooks.h"
#include "ifaces.h"
#include "itemschema.h"
#include "paint.h"

#include "modules/antifreeze.h"
#include "modules/loadouticons.h"
#include "modules/mediguninfo.h"
#include "modules/playeraliases.h"
#include "modules/playeroutlines.h"
#include "modules/statusicons.h"

#define PLUGIN_DESC "StatusSpec v0.11.2"

int AddHook_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *tfPlayer);

void Call_C_TFPlayer_UpdateGlowEffect(C_TFPlayer *tfPlayer);

void Hook_C_BaseCombatCharacter_OnDataChanged(DataUpdateType_t type);
void Hook_C_TFPlayer_GetGlowEffectColor(float *r, float *g, float *b);
void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage);
const char *Hook_IGameResources_GetPlayerName(int client);
void Hook_IPanel_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
void Hook_IPanel_SendMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);
bool Hook_IVEngineClient_GetPlayerInfo(int ent_num, player_info_t *pinfo);

class StatusSpecPlugin: public IServerPluginCallbacks
{
public:
	StatusSpecPlugin();
	~StatusSpecPlugin();

	// IServerPluginCallbacks methods
	virtual bool			Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void			Unload(void);
	virtual void			Pause(void);
	virtual void			UnPause(void);
	virtual const char		*GetPluginDescription(void);	  
	virtual void			LevelInit(char const *pMapName);
	virtual void			ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);
	virtual void			GameFrame(bool simulating);
	virtual void			LevelShutdown(void);
	virtual void			ClientActive(edict_t *pEntity);
	virtual void			ClientDisconnect(edict_t *pEntity);
	virtual void			ClientPutInServer(edict_t *pEntity, char const *playername);
	virtual void			SetCommandClient(int index);
	virtual void			ClientSettingsChanged(edict_t *pEdict);
	virtual PLUGIN_RESULT	ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
	virtual PLUGIN_RESULT	ClientCommand(edict_t *pEntity, const CCommand &args);
	virtual PLUGIN_RESULT	NetworkIDValidated(const char *pszUserName, const char *pszNetworkID);
	virtual void			OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue);
	virtual void			OnEdictAllocated(edict_t *edict);
	virtual void			OnEdictFreed(const edict_t *edict);
};