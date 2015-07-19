/*
 *  cameraautoswitch.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "cameraautoswitch.h"

#include "cbase.h"
#include "c_baseentity.h"
#include "cdll_int.h"
#include "gameeventdefs.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"

#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class CameraAutoSwitch::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName);

	virtual void OnTick();

	void SwitchToKiller(int player, float delay);
private:
	bool killerSwitch;
	int killerSwitchPlayer;
	float killerSwitchTime;
};

CameraAutoSwitch::CameraAutoSwitch() {
	panel = nullptr;

	enabled = new ConVar("statusspec_cameraautoswitch_enabled", "0", FCVAR_NONE, "enable automatic switching of camera", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraAutoSwitch>()->ToggleEnabled(var, pOldValue, flOldValue); });
	killer = new ConVar("statusspec_cameraautoswitch_killer", "0", FCVAR_NONE, "switch to killer upon spectated player death", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraAutoSwitch>()->ToggleKillerEnabled(var, pOldValue, flOldValue); });
	killer_delay = new ConVar("statusspec_cameraautoswitch_killer_delay", "0", FCVAR_NONE, "delay before switching to killer");
}

bool CameraAutoSwitch::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	if (!Interfaces::pGameEventManager) {
		PRINT_TAG();
		Warning("Required interface IGameEventManager2 for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetPrimaryTarget for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<CameraAutoSwitch>().c_str());
	}

	return ready;
}

void CameraAutoSwitch::FireGameEvent(IGameEvent *event) {
	if (enabled->GetBool() && killer->GetBool()) {
		if (strcmp(event->GetName(), GAME_EVENT_PLAYER_DEATH) == 0) {
			Player localPlayer = Interfaces::pEngineClient->GetLocalPlayer();

			if (localPlayer) {
				if (localPlayer.GetObserverMode() == OBS_MODE_FIXED || localPlayer.GetObserverMode() == OBS_MODE_IN_EYE || localPlayer.GetObserverMode() == OBS_MODE_CHASE) {
					Player targetPlayer = localPlayer.GetObserverTarget();

					if (targetPlayer) {
						if (Interfaces::pEngineClient->GetPlayerForUserID(event->GetInt("userid")) == targetPlayer->entindex()) {
							Player killer = Interfaces::pEngineClient->GetPlayerForUserID(event->GetInt("attacker"));

							if (killer) {
								if (killer_delay->GetFloat() > 0.0f) {
									if (panel) {
										panel->SwitchToKiller(killer->entindex(), killer_delay->GetFloat());
									}
								}
								else {
									try {
										Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), killer->entindex());
									}
									catch (bad_pointer &e) {
										Warning("%s\n", e.what());
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void CameraAutoSwitch::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			panel = new Panel(nullptr, "CameraAutoSwitch");
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

void CameraAutoSwitch::ToggleKillerEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!Interfaces::pGameEventManager->FindListener(this, GAME_EVENT_PLAYER_DEATH)) {
			Interfaces::pGameEventManager->AddListener(this, GAME_EVENT_PLAYER_DEATH, false);
		}
	}
	else {
		if (Interfaces::pGameEventManager->FindListener(this, GAME_EVENT_PLAYER_DEATH)) {
			Interfaces::pGameEventManager->RemoveListener(this);
		}
	}
}

CameraAutoSwitch::Panel::Panel(vgui::Panel *parent, const char *panelName) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());
}

void CameraAutoSwitch::Panel::OnTick() {
	if (killerSwitch && Interfaces::pEngineTool->HostTime() > killerSwitchTime) {
		killerSwitch = false;

		try {
			Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), killerSwitchPlayer);
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}
}

void CameraAutoSwitch::Panel::SwitchToKiller(int player, float delay) {
	killerSwitch = true;
	killerSwitchPlayer = player;
	killerSwitchTime = Interfaces::pEngineTool->HostTime() + delay;
}