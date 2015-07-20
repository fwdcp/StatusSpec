/*
 *  localplayer.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "localplayer.h"

#include <functional>
#include <string>

#include "cbase.h"
#include "c_baseplayer.h"
#include "cdll_int.h"
#include "convar.h"
#include "shareddefs.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class LocalPlayer::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName, std::function<void()> setFunction);

	virtual void OnTick();
private:
	std::function<void()> setToCurrentTarget;
};

LocalPlayer::LocalPlayer() {
	getLocalPlayerIndexDetoured = false;
	panel = nullptr;

	enabled = new ConVar("statusspec_localplayer_enabled", "0", FCVAR_NONE, "enable local player override", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<LocalPlayer>()->ToggleEnabled(var, pOldValue, flOldValue); });
	player = new ConVar("statusspec_localplayer_player", "0", FCVAR_NONE, "player index to set as the local player");
	set_current_target = new ConCommand("statusspec_localplayer_set_current_target", []() { g_ModuleManager->GetModule<LocalPlayer>()->SetToCurrentTarget(); }, "set the local player to the current spectator target", FCVAR_NONE);
	track_spec_target = new ConVar("statusspec_localplayer_track_spec_target", "0", FCVAR_NONE, "have the local player value track the spectator target", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<LocalPlayer>()->ToggleTrackSpecTarget(var, pOldValue, flOldValue); });
}

bool LocalPlayer::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", g_ModuleManager->GetModuleName<LocalPlayer>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<LocalPlayer>().c_str());

		ready = false;
	}

	if (!Player::conditionsRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player condition retrieval for module %s not available!\n", g_ModuleManager->GetModuleName<LocalPlayer>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_GetLocalPlayerIndex();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function GetLocalPlayerIndex for module %s not available!\n", g_ModuleManager->GetModuleName<LocalPlayer>().c_str());

		ready = false;
	}

	return ready;
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
			getLocalPlayerIndexDetoured = Funcs::AddDetour_GetLocalPlayerIndex([]()->int { return g_ModuleManager->GetModule<LocalPlayer>()->GetLocalPlayerIndexOverride(); });
		}
	}
	else {
		if (getLocalPlayerIndexDetoured) {
			getLocalPlayerIndexDetoured = !Funcs::RemoveDetour_GetLocalPlayerIndex();
		}
	}
}

void LocalPlayer::ToggleTrackSpecTarget(IConVar *var, const char *pOldValue, float flOldValue) {
	if (track_spec_target->GetBool()) {
		if (!panel) {
			panel = new Panel(nullptr, "LocalPlayer", std::bind(&LocalPlayer::SetToCurrentTarget, this));
		}

		if (panel) {
			panel->SetEnabled(true);
		}
	}
	else {
		if (panel) {
			delete panel;
			panel = nullptr;
		}
	}
}

LocalPlayer::Panel::Panel(vgui::Panel *parent, const char *panelName, std::function<void()> setFunction) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());

	setToCurrentTarget = setFunction;
}

void LocalPlayer::Panel::OnTick() {
	if (Interfaces::pEngineClient->IsInGame()) {
		setToCurrentTarget();
	}
}