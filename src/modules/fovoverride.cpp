/*
 *  fovoverride.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "fovoverride.h"

#include "cbase.h"
#include "c_baseplayer.h"
#include "convar.h"
#include "icliententity.h"
#include "shareddefs.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"
#include "../tfdefs.h"

FOVOverride::FOVOverride() {
	inToolModeHook = 0;
	setupEngineViewHook = 0;

	enabled = new ConVar("statusspec_fovoverride_enabled", "0", FCVAR_NONE, "enable FOV override", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<FOVOverride>()->ToggleEnabled(var, pOldValue, flOldValue); });
	fov = new ConVar("statusspec_fovoverride_fov", "90", FCVAR_NONE, "the FOV value used");
	zoomed = new ConVar("statusspec_fovoverride_zoomed", "0", FCVAR_NONE, "enable FOV override even when sniper rifle is zoomed");
}

bool FOVOverride::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientEngineTools) {
		PRINT_TAG();
		Warning("Required interface IClientEngineTools for module %s not available!\n", g_ModuleManager->GetModuleName<FOVOverride>().c_str());

		ready = false;
	}
	
	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<FOVOverride>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<FOVOverride>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<FOVOverride>().c_str());

		ready = false;
	}

	if (!Player::conditionsRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player condition retrieval for module %s not available!\n", g_ModuleManager->GetModuleName<FOVOverride>().c_str());

		ready = false;
	}

	return ready;
}

bool FOVOverride::InToolModeOverride() {
	RETURN_META_VALUE(MRES_OVERRIDE, true);
}

bool FOVOverride::SetupEngineViewOverride(Vector &origin, QAngle &angles, float &fov) {
	Player localPlayer = Interfaces::pEngineClient->GetLocalPlayer();

	if (localPlayer) {
		if (localPlayer.CheckCondition(TFCond_Zoomed)) {
			RETURN_META_VALUE(MRES_IGNORED, false);
		}
		else if (localPlayer.GetObserverMode() == OBS_MODE_IN_EYE) {
			Player targetPlayer = localPlayer.GetObserverTarget();

			if (targetPlayer && targetPlayer.CheckCondition(TFCond_Zoomed)) {
				RETURN_META_VALUE(MRES_IGNORED, false);
			}
		}
	}

	fov = this->fov->GetFloat();
	RETURN_META_VALUE(MRES_SUPERCEDE, true);
}

void FOVOverride::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!inToolModeHook) {
			inToolModeHook = Funcs::AddHook_IClientEngineTools_InToolMode(Interfaces::pClientEngineTools, SH_MEMBER(this, &FOVOverride::InToolModeOverride), false);
		}

		if (!setupEngineViewHook) {
			setupEngineViewHook = Funcs::AddHook_IClientEngineTools_SetupEngineView(Interfaces::pClientEngineTools, SH_MEMBER(this, &FOVOverride::SetupEngineViewOverride), false);
		}
	}
	else {
		if (inToolModeHook) {
			Funcs::RemoveHook(inToolModeHook);
			inToolModeHook = 0;
		}

		if (setupEngineViewHook) {
			Funcs::RemoveHook(setupEngineViewHook);
			setupEngineViewHook = 0;
		}
	}
}