/*
 *  hooks.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include <map>

#define CLIENT_DLL
#define GLOWS_ENABLE

#include "cdll_int.h"
#include "KeyValues.h"
#include "igameevents.h"
#include "igameresources.h"
#include "vgui/vgui.h"
#include "vgui/IPanel.h"
#include "cbase.h"
#include "c_basecombatcharacter.h"
#include "c_playerresource.h"
#include "glow_outline_effect.h"

#include <sourcehook/sourcehook_impl.h>
#include <sourcehook/sourcehook.h>

using namespace vgui;

class C_TFPlayer;

#if defined _WIN32
#define OFFSET_GETGLOWEFFECTCOLOR 223
#define OFFSET_UPDATEGLOWEFFECT 224
#define OFFSET_DESTROYGLOWEFFECT 225
#define OFFSET_CALCVIEW 229
#define OFFSET_GETOBSERVERMODE 240
#define OFFSET_GETOBSERVERTARGET 241
#endif

extern SourceHook::ISourceHook *g_SHPtr;
extern int g_PLID;

class StatusSpecUnloader: public SourceHook::Impl::UnloadListener
{
public:
	virtual void ReadyToUnload(SourceHook::Plugin plug);
};

class Hooks {
public:
	static int AddHook_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, void(*hook)(float *, float *, float *));
	static int AddHook_IBaseClientDLL_FrameStageNotify(IBaseClientDLL *instance, void(*hook)(ClientFrameStage_t));
	static int AddHook_IGameEventManager2_FireEvent(IGameEventManager2 *instance, bool(*hook)(IGameEvent *, bool));
	static int AddHook_IGameEventManager2_FireEventClientSide(IGameEventManager2 *instance, bool(*hook)(IGameEvent *));
	static int AddHook_IGameResources_GetPlayerName(IGameResources *instance, const char *(*hook)(int));
	static int AddHook_IPanel_PaintTraverse(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool));
	static int AddHook_IPanel_SendMessage(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, KeyValues *, vgui::VPANEL));
	static int AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, bool(*hook)(int, player_info_t *));

	static void CallFunc_C_TFPlayer_CalcView(C_TFPlayer *instance, Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
	static int CallFunc_C_TFPlayer_GetObserverMode(C_TFPlayer *instance);
	static C_BaseEntity *CallFunc_C_TFPlayer_GetObserverTarget(C_TFPlayer *instance);
	static void CallFunc_C_TFPlayer_UpdateGlowEffect(C_TFPlayer *instance);

	static void Pause();

	static bool RemoveHook(int hookID);

	static void Unload();

	static void Unpause();
};