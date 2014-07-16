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
	set_current_target = new ConCommand("statusspec_localplayer_set_current_target", LocalPlayer::SetToCurrentTarget, "set the local player to the current spectator target", FCVAR_NONE);
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
		return Funcs::CallFunc_GetLocalPlayerIndex();
	}
}

void LocalPlayer::SetToCurrentTarget() {
	int localPlayer = Interfaces::pEngineClient->GetLocalPlayer();
	IClientEntity *localPlayerEntity = Interfaces::pClientEntityList->GetClientEntity(localPlayer);

	if (Entities::CheckClassBaseclass(localPlayerEntity->GetClientClass(), "DT_TFPlayer")) {
		C_BaseEntity *targetEntity = Funcs::CallFunc_C_TFPlayer_GetObserverTarget((C_TFPlayer *)localPlayerEntity);

		if (Entities::CheckClassBaseclass(targetEntity->GetClientClass(), "DT_TFPlayer")) {
			g_LocalPlayer->player->SetValue(targetEntity->entindex());
			return;
		}
	}

	Warning("Unable to set local player to current spectator target.\n");
}