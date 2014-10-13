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
	frameHook = 0;
	getLocalPlayerIndexDetoured = false;

	enabled = new ConVar("statusspec_localplayer_enabled", "0", FCVAR_NONE, "enable local player override", [](IConVar *var, const char *pOldValue, float flOldValue) { g_LocalPlayer->ToggleEnabled(var, pOldValue, flOldValue); });
	player = new ConVar("statusspec_localplayer_player", "0", FCVAR_NONE, "player index to set as the local player");
	track_spec_target = new ConVar("statusspec_localplayer_track_spec_target", "0", FCVAR_NONE, "have the local player value track the spectator target", [](IConVar *var, const char *pOldValue, float flOldValue) { g_LocalPlayer->ToggleTrackSpecTarget(var, pOldValue, flOldValue); });
	set_current_target = new ConCommand("statusspec_localplayer_set_current_target", []() { g_LocalPlayer->SetToCurrentTarget(); }, "set the local player to the current spectator target", FCVAR_NONE);
}

void LocalPlayer::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_START) {
		if (track_spec_target->GetBool() && Interfaces::pEngineClient->IsInGame()) {
			SetToCurrentTarget();
		}
	}
}

int LocalPlayer::GetLocalPlayerIndexOverride() {
	if (enabled->GetBool()) {
		Player localPlayer = player->GetInt();

		if (localPlayer) {
			return player->GetInt();
		}
	}

	return Funcs::CallFunc_GetLocalPlayerIndex();
}

void LocalPlayer::SetToCurrentTarget() {
	Player localPlayer = Interfaces::pEngineClient->GetLocalPlayer();

	if (localPlayer) {
		if (localPlayer.GetObserverMode() == OBS_MODE_FIXED || localPlayer.GetObserverMode() == OBS_MODE_IN_EYE || localPlayer.GetObserverMode() == OBS_MODE_CHASE) {
			Player targetPlayer = localPlayer.GetObserverTarget();

			if (targetPlayer) {
				player->SetValue(targetPlayer->entindex());

				return;
			}
		}
	}

	player->SetValue(Interfaces::pEngineClient->GetLocalPlayer());
}

void LocalPlayer::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!getLocalPlayerIndexDetoured) {
			getLocalPlayerIndexDetoured = Funcs::AddDetour_GetLocalPlayerIndex([]()->int { return g_LocalPlayer->GetLocalPlayerIndexOverride(); });
		}
	}
	else {
		if (getLocalPlayerIndexDetoured) {
			getLocalPlayerIndexDetoured = !Funcs::RemoveDetour_GetLocalPlayerIndex();
		}
	}
}

void LocalPlayer::ToggleTrackSpecTarget(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &LocalPlayer::FrameHook), true);
		}
	}
	else {
		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}