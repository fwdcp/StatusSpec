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
#include "funcs.h"
#include "glows.h"
#include "ifaces.h"
#include "itemschema.h"
#include "paint.h"

#include "modules/antifreeze.h"
#include "modules/killstreaks.h"
#include "modules/loadouticons.h"
#include "modules/localplayer.h"
#include "modules/mediguninfo.h"
#include "modules/multipanel.h"
#include "modules/playeraliases.h"
#include "modules/playeroutlines.h"
#include "modules/projectileoutlines.h"
#include "modules/statusicons.h"

#define PLUGIN_DESC "StatusSpec v0.18.0"

int Detour_GetLocalPlayerIndex();

void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage);
bool Hook_IClientMode_DoPostScreenSpaceEffects(const CViewSetup *pSetup);
bool Hook_IGameEventManager2_FireEvent(IGameEvent *event, bool bDontBroadcast = false);
bool Hook_IGameEventManager2_FireEventClientSide(IGameEvent *event);
void Hook_IPanel_PaintTraverse_Pre(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
void Hook_IPanel_PaintTraverse_Post(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
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