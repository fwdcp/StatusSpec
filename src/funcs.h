/*
 *  funcs.h
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

#include "cbase.h"
#include "cdll_int.h"
#include "iclientmode.h"
#include "igameevents.h"
#include "igameresources.h"
#include "vgui/vgui.h"
#include "vgui/IPanel.h"

#include <sourcehook_impl.h>
#include <sourcehook.h>
#include <MinHook.h>

using namespace vgui;

class C_TFPlayer;

typedef int(*GLPI_t)(void);
typedef void(__thiscall *SM_t)(C_BaseEntity *, const char *);
typedef void(__thiscall *SMI_t)(C_BaseEntity *, int);
typedef void(__thiscall *SMP_t)(C_BaseEntity *, const model_t *);

#if defined _WIN32
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#elif defined __linux__
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#endif

#if defined _WIN32
#define OFFSET_GETGLOWEFFECTCOLOR 224
#define OFFSET_UPDATEGLOWEFFECT 225
#define OFFSET_DESTROYGLOWEFFECT 226
#define OFFSET_CALCVIEW 230
#define OFFSET_GETOBSERVERMODE 241
#define OFFSET_GETOBSERVERTARGET 242
#define CLIENT_MODULE_SIZE 0xC74EC0
#define GETLOCALPLAYERINDEX_SIG "\xE8\x00\x00\x00\x00\x85\xC0\x74\x08\x8D\x48\x08\x8B\x01\xFF\x60\x24\x33\xC0\xC3"
#define GETLOCALPLAYERINDEX_MASK "x????xxxxxxxxxxxxxxx"
#define SETMODEL_SIG "\x55\x8B\xEC\x8B\x55\x08\x56\x57\x8B\xF9\x85\xD2"
#define SETMODEL_MASK "xxxxxxxxxxxx"
#define SETMODELINDEX_SIG "\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x57\x66\x89\x86\x00\x00\x00\x00"
#define SETMODELINDEX_MASK "xxxxxxxxxxxxx????"
#define SETMODELPOINTER_SIG "\x55\x8B\xEC\x56\x8B\xF1\x57\x8B\x7D\x08\x3B\x7E\x00\x74\x00"
#define SETMODELPOINTER_MASK "xxxxxxxxxxxx?x?"
#endif

extern SourceHook::ISourceHook *g_SHPtr;
extern int g_PLID;

class StatusSpecUnloader: public SourceHook::Impl::UnloadListener
{
public:
	virtual void ReadyToUnload(SourceHook::Plugin plug);
};

class Funcs {
public:
	static bool AddDetour_GetLocalPlayerIndex(GLPI_t detour);
	static bool AddDetour_C_BaseEntity_SetModel(SM_t detour);
	static bool AddDetour_C_BaseEntity_SetModelIndex(SMI_t detour);
	static bool AddDetour_C_BaseEntity_SetModelPointer(SMP_t detour);

	static int AddHook_IBaseClientDLL_FrameStageNotify(IBaseClientDLL *instance, void(*hook)(ClientFrameStage_t));
	static int AddHook_IClientMode_DoPostScreenSpaceEffects(IClientMode *instance, bool(*hook)(const CViewSetup *));
	static int AddHook_IClientRenderable_GetModel(IClientRenderable *instance, const model_t *(*hook)());
	static int AddHook_IGameEventManager2_FireEvent(IGameEventManager2 *instance, bool(*hook)(IGameEvent *, bool));
	static int AddHook_IGameEventManager2_FireEventClientSide(IGameEventManager2 *instance, bool(*hook)(IGameEvent *));
	static int AddHook_IPanel_PaintTraverse_Pre(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool));
	static int AddHook_IPanel_PaintTraverse_Post(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool));
	static int AddHook_IPanel_SendMessage(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, KeyValues *, vgui::VPANEL));
	static int AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, bool(*hook)(int, player_info_t *));

	static int CallFunc_GetLocalPlayerIndex();
	static void CallFunc_C_BaseEntity_SetModel(C_BaseEntity *instance, const char *pModelName);
	static void CallFunc_C_BaseEntity_SetModelIndex(C_BaseEntity *instance, int index);
	static void CallFunc_C_BaseEntity_SetModelPointer(C_BaseEntity *instance, const model_t *pModel);

	static void CallFunc_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, float *r, float *g, float *b);
	static int CallFunc_C_TFPlayer_GetObserverMode(C_TFPlayer *instance);
	static C_BaseEntity *CallFunc_C_TFPlayer_GetObserverTarget(C_TFPlayer *instance);
	static const model_t *CallFunc_IClientRenderable_GetModel(IClientRenderable *instance);
	static bool CallFunc_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, int ent_num, player_info_t *pinfo);

	static bool RemoveDetour_GetLocalPlayerIndex();
	static bool RemoveDetour_C_BaseEntity_SetModel();
	static bool RemoveDetour_C_BaseEntity_SetModelIndex();
	static bool RemoveDetour_C_BaseEntity_SetModelPointer();

	static bool RemoveHook(int hookID);

	static bool Load();

	static bool Unload();

	static bool Pause();

	static bool Unpause();
private:
	static GLPI_t getLocalPlayerIndexOriginal;
	static SM_t setModelOriginal;
	static SMI_t setModelIndexOriginal;
	static SMP_t setModelPointerOriginal;

	static bool AddDetour(void *target, void *detour, void *&original);

	static bool RemoveDetour(void *target);
};