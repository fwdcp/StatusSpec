/*
 *  funcs.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <functional>
#include <map>

#include "sourcehook.h"

#include "cdll_int.h"
#include "vgui/VGUI.h"

#include "gamedata.h"

class C_TFPlayer;
class IClientEngineTools;
class IClientMode;
class ICvar;
class IGameEvent;
class IGameEventManager2;
class IMaterialSystem;
class IPrediction;

namespace vgui {
	class IPanel;
};

class Funcs {
public:
	static bool AddDetour_GetLocalPlayerIndex(GLPI_t detour);

	static int AddHook_C_BaseEntity_SetModel(std::function<void(C_BaseEntity *, const model_t *&)> hook);
	static int AddHook_C_HLTVCamera_SetMode(std::function<void(C_HLTVCamera *, int &)> hook);
	static int AddHook_C_HLTVCamera_SetPrimaryTarget(std::function<void(C_HLTVCamera *, int &)> hook);

	static int AddGlobalHook_C_TFPlayer_GetFOV(C_TFPlayer *instance, fastdelegate::FastDelegate0<float> hook, bool post);
	static int AddHook_IClientEngineTools_InToolMode(IClientEngineTools *instance, fastdelegate::FastDelegate0<bool> hook, bool post);
	static int AddHook_IClientEngineTools_IsThirdPersonCamera(IClientEngineTools *instance, fastdelegate::FastDelegate0<bool> hook, bool post);
	static int AddHook_IClientEngineTools_SetupEngineView(IClientEngineTools *instance, fastdelegate::FastDelegate3<Vector &, QAngle &, float &, bool> hook, bool post);
	static int AddHook_IClientMode_DoPostScreenSpaceEffects(IClientMode *instance, fastdelegate::FastDelegate1<const CViewSetup *, bool> hook, bool post);
	static int AddHook_ICvar_ConsoleColorPrintf(ICvar *instance, fastdelegate::FastDelegate2<const Color &, const char *> hook, bool post);
	static int AddHook_ICvar_ConsoleDPrintf(ICvar *instance, fastdelegate::FastDelegate1<const char *> hook, bool post);
	static int AddHook_ICvar_ConsolePrintf(ICvar *instance, fastdelegate::FastDelegate1<const char *> hook, bool post);
	static int AddHook_IGameEventManager2_FireEventClientSide(IGameEventManager2 *instance, fastdelegate::FastDelegate1<IGameEvent *, bool> hook, bool post);
	static int AddHook_IMaterialSystem_FindMaterial(IMaterialSystem *instance, fastdelegate::FastDelegate4<char const *, const char *, bool, const char *, IMaterial *> hook, bool post);
	static int AddHook_IPrediction_PostEntityPacketReceived(IPrediction *instance, fastdelegate::FastDelegate0<> hook, bool post);
	static int AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, fastdelegate::FastDelegate2<int, player_info_t *, bool> hook, bool post);

	static int CallFunc_GetLocalPlayerIndex();
	static void CallFunc_C_BaseEntity_SetModelIndex(C_BaseEntity *instance, int index);
	static void CallFunc_C_BaseEntity_SetModelPointer(C_BaseEntity *instance, const model_t *pModel);
	static void CallFunc_C_HLTVCamera_SetCameraAngle(C_HLTVCamera *instance, QAngle &targetAngle);
	static void CallFunc_C_HLTVCamera_SetMode(C_HLTVCamera *instance, int iMode);
	static void CallFunc_C_HLTVCamera_SetPrimaryTarget(C_HLTVCamera *instance, int nEntity);

	static float CallFunc_C_TFPlayer_GetFOV(C_TFPlayer *instance);
	static void CallFunc_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, float *r, float *g, float *b);
	static bool CallFunc_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, int ent_num, player_info_t *pinfo);

	static GLPI_t GetFunc_GetLocalPlayerIndex();
	static SMI_t GetFunc_C_BaseEntity_SetModelIndex();
	static SMP_t GetFunc_C_BaseEntity_SetModelPointer();
	static SCA_t GetFunc_C_HLTVCamera_SetCameraAngle();
	static SM_t GetFunc_C_HLTVCamera_SetMode();
	static SPT_t GetFunc_C_HLTVCamera_SetPrimaryTarget();

	static bool RemoveDetour_GetLocalPlayerIndex();

	static void RemoveHook_C_BaseEntity_SetModel(int hookID);
	static void RemoveHook_C_HLTVCamera_SetMode(int hookID);
	static void RemoveHook_C_HLTVCamera_SetPrimaryTarget(int hookID);

	static bool RemoveHook(int hookID);

	static bool Load();

	static bool Unload();

	static bool Pause();

	static bool Unpause();
private:
	static int setModeLastHookRegistered;
	static std::map<int, std::function<void(C_HLTVCamera *, int &)>> setModeHooks;
	static int setModelLastHookRegistered;
	static std::map<int, std::function<void(C_BaseEntity *, const model_t *&)>> setModelHooks;
	static int setPrimaryTargetLastHookRegistered;
	static std::map<int, std::function<void(C_HLTVCamera *, int &)>> setPrimaryTargetHooks;

	static GLPI_t getLocalPlayerIndexOriginal;
	static SM_t setModeOriginal;
	static SPT_t setPrimaryTargetOriginal;
	static SMI_t setModelIndexOriginal;
	static SMP_t setModelPointerOriginal;

	static bool AddDetour(void *target, void *detour, void *&original);

	static bool AddDetour_C_BaseEntity_SetModelIndex(SMIH_t detour);
	static bool AddDetour_C_BaseEntity_SetModelPointer(SMPH_t detour);
	static bool AddDetour_C_HLTVCamera_SetMode(SMH_t detour);
	static bool AddDetour_C_HLTVCamera_SetPrimaryTarget(SPTH_t detour);

	static void __fastcall Detour_C_BaseEntity_SetModelIndex(C_BaseEntity *, void *, int);
	static void __fastcall Detour_C_BaseEntity_SetModelPointer(C_BaseEntity *, void *, const model_t *);
	static void __fastcall Detour_C_HLTVCamera_SetMode(C_HLTVCamera *, void *, int);
	static void __fastcall Detour_C_HLTVCamera_SetPrimaryTarget(C_HLTVCamera *, void *, int);

	static bool RemoveDetour_C_BaseEntity_SetModelIndex();
	static bool RemoveDetour_C_BaseEntity_SetModelPointer();
	static bool RemoveDetour_C_HLTVCamera_SetMode();
	static bool RemoveDetour_C_HLTVCamera_SetPrimaryTarget();

	static bool RemoveDetour(void *target);
};

extern SourceHook::ISourceHook *g_SHPtr;