/*
 *  fovoverride.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "fovoverride.h"

FOVOverride::FOVOverride(std::string name) : Module(name) {
	frameHook = 0;
	getFOVHook = 0;

	enabled = new ConVar("statusspec_fovoverride_enabled", "0", FCVAR_NONE, "enable FOV override", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<FOVOverride>("FOV Override")->ToggleEnabled(var, pOldValue, flOldValue); });
	fov = new ConVar("statusspec_fovoverride_fov", "90", FCVAR_NONE, "the FOV value used");
	zoomed = new ConVar("statusspec_fovoverride_zoomed", "0", FCVAR_NONE, "enable FOV override even when sniper rifle is zoomed");
}

bool FOVOverride::CheckDependencies(std::string name) {
	bool ready = true;
	
	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::conditionsRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player condition retrieval for module %s not available!\n", name.c_str());

		ready = false;
	}

	return ready;
}

void FOVOverride::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		if (HookGetFOV()) {
			Funcs::RemoveHook(frameHook);
			frameHook = 0;
		}
	}

	RETURN_META(MRES_IGNORED);
}

float FOVOverride::GetFOVOverride() {
	if (!zoomed->GetBool()) {
		Player player = (IClientEntity *) META_IFACEPTR(C_TFPlayer);

		if (player && player.CheckCondition(TFCond_Zoomed)) {
			RETURN_META_VALUE(MRES_IGNORED, 0.0f);
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, fov->GetFloat());
}

bool FOVOverride::HookGetFOV() {
	if (getFOVHook) {
		return true;
	}

	for (auto iterator = Player::begin(); iterator != Player::end(); ++iterator) {
		Player player = *iterator;

		getFOVHook = Funcs::AddGlobalHook_C_TFPlayer_GetFOV((C_TFPlayer *)player.GetEntity(), SH_MEMBER(this, &FOVOverride::GetFOVOverride), false);

		if (getFOVHook) {
			return true;
		}
	}

	return false;
}

void FOVOverride::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!HookGetFOV() && !frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &FOVOverride::FrameHook), true);
		}
	}
	else {
		if (getFOVHook) {
			if (Funcs::RemoveHook(getFOVHook)) {
				getFOVHook = 0;
			}
		}

		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}