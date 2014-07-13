/*
*  localplayer.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "localplayer.h"

LocalPlayer::LocalPlayer() {
	enabled = new ConVar("statusspec_localplayer_enabled", "0", FCVAR_NONE, "enable local player override");
	player = new ConVar("statusspec_localplayer_player", "0", FCVAR_NONE, "player to set as the local player");
}

bool LocalPlayer::IsEnabled() {
	return enabled->GetBool();
}

int LocalPlayer::GetLocalPlayerIndexOverride() {
	IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(player->GetInt());

	if (IsEnabled() && entity && Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return player->GetInt();
	}
	else {
		return Hooks::CallFunc_GetLocalPlayerIndex();
	}
}