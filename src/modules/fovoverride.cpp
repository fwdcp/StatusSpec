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

FOVOverride::FOVOverride() {
	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable FOV override");
	fov = new ConVar("statusspec_antifreeze_fov", "90", FCVAR_NONE, "the FOV value used");
	zoomed = new ConVar("statusspec_antifreeze_zoomed", "0", FCVAR_NONE, "enable FOV override even when sniper rifle is zoomed");
}

bool FOVOverride::IsEnabled() {
	return enabled->GetBool();
}

float FOVOverride::GetFOVOverride(C_TFPlayer *instance) {
	if (!zoomed->GetBool()) {
		Player player = (IClientEntity *) instance;

		if (player.CheckCondition(TFCond_Zoomed)) {
			return Funcs::CallFunc_C_TFPlayer_GetFOV(instance);
		}
	}

	return fov->GetFloat();
}