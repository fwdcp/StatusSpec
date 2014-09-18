/*
*  funcs.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "funcs.h"

void StatusSpecUnloader::ReadyToUnload(SourceHook::Plugin plug) {};

SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_MANUALHOOK5_void(C_TFPlayer_CalcView, OFFSET_CALCVIEW, 0, 0, Vector &, QAngle &, float &, float &, float &);
SH_DECL_MANUALHOOK3_void(C_TFPlayer_GetGlowEffectColor, OFFSET_GETGLOWEFFECTCOLOR, 0, 0, float *, float *, float *);
SH_DECL_MANUALHOOK0(C_TFPlayer_GetObserverMode, OFFSET_GETOBSERVERMODE, 0, 0, int);
SH_DECL_MANUALHOOK0(C_TFPlayer_GetObserverTarget, OFFSET_GETOBSERVERTARGET, 0, 0, C_BaseEntity *);
SH_DECL_HOOK1_void(IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t);
SH_DECL_HOOK1(IClientMode, DoPostScreenSpaceEffects, SH_NOATTRIB, 0, bool, const CViewSetup *);
SH_DECL_HOOK0(IClientRenderable, GetModel, const, 0, const model_t *);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);
SH_DECL_HOOK1(IGameEventManager2, FireEventClientSide, SH_NOATTRIB, 0, bool, IGameEvent *);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

inline bool DataCompare(const BYTE* pData, const BYTE* bSig, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}

	return (*szMask) == NULL;
}

inline DWORD FindPattern(DWORD dwAddress, DWORD dwSize, BYTE* pbSig, const char* szMask)
{
	for (DWORD i = NULL; i < dwSize; i++)
	{
		if (DataCompare((BYTE*)(dwAddress + i), pbSig, szMask))
			return (DWORD)(dwAddress + i);
	}

	return 0;
}

inline GLPI_t GetGLPIFunc() {
#if defined _WIN32
	static DWORD pointer = NULL;
	if (!pointer)
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)GETLOCALPLAYERINDEX_SIG, GETLOCALPLAYERINDEX_MASK);
	return (GLPI_t)(pointer);
#else
	return nullptr;
#endif
}

inline SMI_t GetSMIFunc() {
#if defined _WIN32
	static DWORD pointer = NULL;
	if (!pointer)
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)SETMODELINDEX_SIG, SETMODELINDEX_MASK);
	return (SMI_t)(pointer);
#else
	return nullptr;
#endif
}

inline SMP_t GetSMPFunc() {
#if defined _WIN32
	static DWORD pointer = NULL;
	if (!pointer)
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)SETMODELPOINTER_SIG, SETMODELPOINTER_MASK);
	return (SMP_t)(pointer);
#else
	return nullptr;
#endif
}

GLPI_t Funcs::getLocalPlayerIndexOriginal = nullptr;
SMI_t Funcs::setModelIndexOriginal = nullptr;
SMP_t Funcs::setModelPointerOriginal = nullptr;

bool Funcs::AddDetour(void *target, void *detour, void *&original) {
	MH_STATUS addHookResult = MH_CreateHook(target, detour, &original);

	if (addHookResult != MH_OK && addHookResult != MH_ERROR_ALREADY_CREATED) {
		return false;
	}

	MH_STATUS enableHookResult = MH_EnableHook(target);

	return (enableHookResult == MH_OK || enableHookResult == MH_ERROR_ENABLED);
}

bool Funcs::AddDetour_GetLocalPlayerIndex(GLPI_t detour) {
	void *original;

	if (AddDetour(GetGLPIFunc(), detour, original)) {
		getLocalPlayerIndexOriginal = reinterpret_cast<GLPI_t>(original);
		return true;
	}

	return false;
}

bool Funcs::AddDetour_C_BaseEntity_SetModelIndex(SMIH_t detour) {
	void *original;

	if (AddDetour(GetSMIFunc(), detour, original)) {
		setModelIndexOriginal = reinterpret_cast<SMI_t>(original);
		return true;
	}

	return false;
}

bool Funcs::AddDetour_C_BaseEntity_SetModelPointer(SMPH_t detour) {
	void *original;

	if (AddDetour(GetSMPFunc(), detour, original)) {
		setModelPointerOriginal = reinterpret_cast<SMP_t>(original);
		return true;
	}

	return false;
}

int Funcs::AddHook_IBaseClientDLL_FrameStageNotify(IBaseClientDLL *instance, void(*hook)(ClientFrameStage_t)) {
	return SH_ADD_HOOK(IBaseClientDLL, FrameStageNotify, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(IClientMode *instance, bool(*hook)(const CViewSetup *)) {
	return SH_ADD_HOOK(IClientMode, DoPostScreenSpaceEffects, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IClientRenderable_GetModel(IClientRenderable *instance, const model_t *(*hook)()) {
	return SH_ADD_HOOK(IClientRenderable, GetModel, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IGameEventManager2_FireEvent(IGameEventManager2 *instance, bool(*hook)(IGameEvent *, bool)) {
	return SH_ADD_HOOK(IGameEventManager2, FireEvent, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IGameEventManager2_FireEventClientSide(IGameEventManager2 *instance, bool(*hook)(IGameEvent *)) {
	return SH_ADD_HOOK(IGameEventManager2, FireEventClientSide, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IPanel_PaintTraverse_Pre(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool)) {
	return SH_ADD_HOOK(IPanel, PaintTraverse, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IPanel_PaintTraverse_Post(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, bool, bool)) {
	return SH_ADD_HOOK(IPanel, PaintTraverse, instance, SH_STATIC(hook), true);
}

int Funcs::AddHook_IPanel_SendMessage(vgui::IPanel *instance, void(*hook)(vgui::VPANEL, KeyValues *, vgui::VPANEL)) {
	return SH_ADD_HOOK(IPanel, SendMessage, instance, SH_STATIC(hook), false);
}

int Funcs::AddHook_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, bool(*hook)(int, player_info_t *)) {
	return SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, instance, SH_STATIC(hook), false);
}

int Funcs::CallFunc_GetLocalPlayerIndex() {
	if (getLocalPlayerIndexOriginal) {
		return getLocalPlayerIndexOriginal();
	}
	else {
		return GetGLPIFunc()();
	}
}

void Funcs::CallFunc_C_BaseEntity_SetModelIndex(C_BaseEntity *instance, int index) {
	if (setModelIndexOriginal) {
		setModelIndexOriginal(instance, index);
	}
	else {
		GetSMIFunc()(instance, index);
	}
}

void Funcs::CallFunc_C_BaseEntity_SetModelPointer(C_BaseEntity *instance, const model_t *pModel) {
	if (setModelPointerOriginal) {
		setModelPointerOriginal(instance, pModel);
	}
	else {
		GetSMPFunc()(instance, pModel);
	}
}

int Funcs::CallFunc_C_TFPlayer_GetObserverMode(C_TFPlayer *instance) {
	return SH_MCALL(instance, C_TFPlayer_GetObserverMode)();
}

C_BaseEntity *Funcs::CallFunc_C_TFPlayer_GetObserverTarget(C_TFPlayer *instance) {
	return SH_MCALL(instance, C_TFPlayer_GetObserverTarget)();
}

void Funcs::CallFunc_C_TFPlayer_GetGlowEffectColor(C_TFPlayer *instance, float *r, float *g, float *b) {
	SH_MCALL(instance, C_TFPlayer_GetGlowEffectColor)(r, g, b);
}

const model_t *Funcs::CallFunc_IClientRenderable_GetModel(IClientRenderable *instance) {
	return SH_CALL(instance, &IClientRenderable::GetModel)();
}

bool Funcs::CallFunc_IVEngineClient_GetPlayerInfo(IVEngineClient *instance, int ent_num, player_info_t *pinfo) {
	return SH_CALL(instance, &IVEngineClient::GetPlayerInfo)(ent_num, pinfo);
}

bool Funcs::RemoveDetour_GetLocalPlayerIndex() {
	if (RemoveDetour(GetGLPIFunc())) {
		getLocalPlayerIndexOriginal = nullptr;
		return true;
	}

	return false;
}

bool Funcs::RemoveDetour_C_BaseEntity_SetModelIndex() {
	if (RemoveDetour(GetSMIFunc())) {
		setModelIndexOriginal = nullptr;
		return true;
	}

	return false;
}

bool Funcs::RemoveDetour_C_BaseEntity_SetModelPointer() {
	if (RemoveDetour(GetSMPFunc())) {
		setModelPointerOriginal = nullptr;
		return true;
	}

	return false;
}

bool Funcs::RemoveDetour(void *target) {
	MH_STATUS disableHookResult = MH_DisableHook(target);

	if (disableHookResult != MH_OK && disableHookResult != MH_ERROR_DISABLED) {
		return false;
	}

	MH_STATUS removeHookResult = MH_RemoveHook(target);

	return (removeHookResult == MH_OK || removeHookResult == MH_ERROR_NOT_CREATED);
}

bool Funcs::RemoveHook(int hookID) {
	return SH_REMOVE_HOOK_ID(hookID);
}

bool Funcs::Load() {
	MH_STATUS minHookResult = MH_Initialize();

	return (minHookResult == MH_OK || minHookResult == MH_ERROR_ALREADY_INITIALIZED);
}

bool Funcs::Unload() {
	g_SourceHook.UnloadPlugin(g_PLID, new StatusSpecUnloader());
	MH_STATUS minHookResult = MH_Uninitialize();

	return (minHookResult == MH_OK || minHookResult == MH_ERROR_NOT_INITIALIZED);
}

bool Funcs::Pause() {
	g_SourceHook.PausePlugin(g_PLID);
	MH_STATUS minHookResult = MH_DisableHook(MH_ALL_HOOKS);

	return (minHookResult == MH_OK || minHookResult == MH_ERROR_DISABLED);
}

bool Funcs::Unpause() {
	g_SourceHook.UnpausePlugin(g_PLID);
	MH_STATUS minHookResult = MH_EnableHook(MH_ALL_HOOKS);

	return (minHookResult == MH_OK || minHookResult == MH_ERROR_ENABLED);
}