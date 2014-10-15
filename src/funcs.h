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
#include "hltvcamera.h"
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
typedef void(__thiscall *SMI_t)(C_BaseEntity *, int);
typedef void(__thiscall *SMP_t)(C_BaseEntity *, const model_t *);
typedef void(__thiscall *SPT_t)(C_HLTVCamera *, int);
typedef void(__fastcall *SMIH_t)(C_BaseEntity *, void *, int);
typedef void(__fastcall *SMPH_t)(C_BaseEntity *, void *, const model_t *);

#if defined _WIN32
#define GetFuncAddress(pAddress, szFunction) ::GetProcAddress((HMODULE)pAddress, szFunction)
#define GetHandleOfModule(szModuleName) GetModuleHandleA(szModuleName".dll")
#elif defined __linux__
#define GetFuncAddress(pAddress, szFunction) dlsym(pAddress, szFunction)
#define GetHandleOfModule(szModuleName) dlopen(szModuleName".so", RTLD_NOLOAD)
#endif

#if defined _WIN32
#define OFFSET_GETHEALTH 106
#define OFFSET_GETMAXHEALTH 107
#define OFFSET_GETGLOWEFFECTCOLOR 224
#define OFFSET_UPDATEGLOWEFFECT 225
#define OFFSET_DESTROYGLOWEFFECT 226
#define OFFSET_CALCVIEW 230
#define OFFSET_GETOBSERVERMODE 241
#define OFFSET_GETOBSERVERTARGET 242
#define OFFSET_GETFOV 269
#define CLIENT_MODULE_SIZE 0xC74EC0
#define GETLOCALPLAYERINDEX_SIG "\xE8\x00\x00\x00\x00\x85\xC0\x74\x08\x8D\x48\x08\x8B\x01\xFF\x60\x24\x33\xC0\xC3"
#define GETLOCALPLAYERINDEX_MASK "x????xxxxxxxxxxxxxxx"
#define SETMODEL_SIG "\x55\x8B\xEC\x8B\x55\x08\x56\x57\x8B\xF9\x85\xD2"
#define SETMODEL_MASK "xxxxxxxxxxxx"
#define SETMODELINDEX_SIG "\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x57\x66\x89\x86\x00\x00\x00\x00"
#define SETMODELINDEX_MASK "xxxxxxxxxxxxx????"
#define SETMODELPOINTER_SIG "\x55\x8B\xEC\x56\x8B\xF1\x57\x8B\x7D\x08\x3B\x7E\x00\x74\x00"
#define SETMODELPOINTER_MASK "xxxxxxxxxxxx?x?"
#define SETPRIMARYTARGET_SIG "\x55\x8B\xEC\x8B\x45\x08\x83\xEC\x00\x53\x56\x8B\xF1"
#define SETPRIMARYTARGET_MASK "xxxxxxxx?xxxx"
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
	static bool AddDetour_C_BaseEntity_SetModelIndex(SMIH_t detour);
	static bool AddDetour_C_BaseEntity_SetModelPointer(SMPH_t detour);

	static int AddGlobalHook_C_TFPlayer_GetFOV(C_TFPlayer *instance, fastdelegate::FastDelegate0<float> hook, bool post);
	static int AddHook_IBaseClientDLL_FrameStageNotify(IBaseClientDLL *instance, fastdelegate::FastDelegate1<ClientFrameStage_t> hook, bool post);
	static int AddHook_IClientMode_DoPostScreenSpaceEffects(IClientMode *instance, fastdelegate::FastDelegate1<const CViewSetup *, bool> hook, bool post);
	static int AddHook_IGameEventManager2_FireEventClientSide(IGameEventManager2 *instance, fastdelegate::FastDelegate1<IGameEvent *, bool> hook, bool post);
	static int AddHook_IMaterialSystem_FindMaterial(IMaterialSystem *instance, fastdelegate::FastDelegate4<char const *, const char *, bool, const char *, IMaterial *> hook, bool post);
	static int AddHook_IPanel_SendMessage(vgui::IPanel *instance, fastdelegate::FastDelegate3<vgui::VPANEL, KeyValues *, vgui::VPANEL> hook, bool post);
	static int AddHook_IPanel_SetPos(vgui::IPanel *instance, fastdelegate::FastDelegate3<vgui::VPANEL, int, int> hook, bool post);
	static int AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, fastdelegate::FastDelegate2<int, player_info_t *, bool> hook, bool post);

	static int CallFunc_GetLocalPlayerIndex();
	static void CallFunc_C_BaseEntity_SetModelIndex(C_BaseEntity *instance, int index);
	static void CallFunc_C_BaseEntity_SetModelPointer(C_BaseEntity *instance, const model_t *pModel);
	static void CallFunc_C_HLTVCamera_SetPrimaryTarget(C_HLTVCamera *instance, int nEntity);

	static float CallFunc_C_TFPlayer_GetFOV(C_TFPlayer *instance);
	static void CallFunc_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, float *r, float *g, float *b);
	static int CallFunc_C_TFPlayer_GetHealth(C_TFPlayer *instance);
	static int CallFunc_C_TFPlayer_GetMaxHealth(C_TFPlayer *instance);
	static int CallFunc_C_TFPlayer_GetObserverMode(C_TFPlayer *instance);
	static C_BaseEntity *CallFunc_C_TFPlayer_GetObserverTarget(C_TFPlayer *instance);
	static bool CallFunc_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, int ent_num, player_info_t *pinfo);

	static bool RemoveDetour_GetLocalPlayerIndex();
	static bool RemoveDetour_C_BaseEntity_SetModelIndex();
	static bool RemoveDetour_C_BaseEntity_SetModelPointer();

	static bool RemoveHook(int hookID);

	static bool Load();

	static bool Unload();

	static bool Pause();

	static bool Unpause();
private:
	static GLPI_t getLocalPlayerIndexOriginal;
	static SMI_t setModelIndexOriginal;
	static SMP_t setModelPointerOriginal;

	static bool AddDetour(void *target, void *detour, void *&original);

	static bool RemoveDetour(void *target);
};