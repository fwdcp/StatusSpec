/*
*  hooks.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "hooks.h"

void StatusSpecUnloader::ReadyToUnload(SourceHook::Plugin plug) {};

SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_MANUALHOOK5_void(C_TFPlayer_CalcView, OFFSET_CALCVIEW, 0, 0, Vector &, QAngle &, float &, float &, float &);
SH_DECL_MANUALHOOK3_void(C_TFPlayer_GetGlowEffectColor, OFFSET_GETGLOWEFFECTCOLOR, 0, 0, float *, float *, float *);
SH_DECL_MANUALHOOK0(C_TFPlayer_GetObserverMode, OFFSET_GETOBSERVERMODE, 0, 0, int);
SH_DECL_MANUALHOOK0(C_TFPlayer_GetObserverTarget, OFFSET_GETOBSERVERTARGET, 0, 0, C_BaseEntity *);
SH_DECL_MANUALHOOK0_void(C_TFPlayer_UpdateGlowEffect, OFFSET_UPDATEGLOWEFFECT, 0, 0);
SH_DECL_HOOK1_void(IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t);
SH_DECL_HOOK1(IGameResources, GetPlayerName, SH_NOATTRIB, 0, const char *, int);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

int Hooks::AddHook_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, void(*hook)(float *, float *, float *)) {
	return SH_ADD_MANUALVPHOOK(C_TFPlayer_GetGlowEffectColor, instance, hook, false);
}

int Hooks::AddHook_IBaseClientDLL_FrameStageNotify(IBaseClientDLL *instance, void(*hook)(ClientFrameStage_t)) {
	return SH_ADD_HOOK(IBaseClientDLL, FrameStageNotify, instance, hook, false);
}

int Hooks::AddHook_IGameResources_GetPlayerName(IGameResources *instance, const char *(*hook)(int)) {
	return SH_ADD_HOOK(IGameResources, GetPlayerName, instance, hook, false);
}

int Hooks::AddHook_IPanel_PaintTraverse(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool)) {
	return SH_ADD_HOOK(IPanel, PaintTraverse, instance, hook, false);
}

int Hooks::AddHook_IPanel_SendMessage(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, KeyValues *, vgui::VPANEL)) {
	return SH_ADD_HOOK(IPanel, SendMessage, instance, hook, false);
}

int Hooks::AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, bool(*hook)(int, player_info_t *)) {
	return SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, instance, hook, false);
}

void Hooks::CallFunc_C_TFPlayer_CalcView(C_TFPlayer *instance, Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov) {
	SH_MCALL(instance, C_TFPlayer_CalcView)(eyeOrigin, eyeAngles, zNear, zFar, fov);
}

int Hooks::CallFunc_C_TFPlayer_GetObserverMode(C_TFPlayer *instance) {
	return SH_MCALL(instance, C_TFPlayer_GetObserverMode)();
}

C_BaseEntity *Hooks::CallFunc_C_TFPlayer_GetObserverTarget(C_TFPlayer *instance) {
	return SH_MCALL(instance, C_TFPlayer_GetObserverTarget)();
}

void Hooks::CallFunc_C_TFPlayer_UpdateGlowEffect(C_TFPlayer *instance) {
	SH_MCALL(instance, C_TFPlayer_UpdateGlowEffect)();
}

void Hooks::Pause() {
	g_SourceHook.PausePlugin(g_PLID);
}

bool Hooks::RemoveHook(int hookID) {
	return SH_REMOVE_HOOK_ID(hookID);
}

void Hooks::Unload() {
	g_SourceHook.UnloadPlugin(g_PLID, new StatusSpecUnloader());
}

void Hooks::Unpause() {
	g_SourceHook.UnpausePlugin(g_PLID);
}