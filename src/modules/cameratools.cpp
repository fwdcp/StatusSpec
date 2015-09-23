/*
 *  cameratools.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "cameratools.h"

#include <functional>

#include "cbase.h"
#include "filesystem.h"
#include "hltvcamera.h"
#include "icliententity.h"
#include "icliententitylist.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
#include "vgui/IScheme.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../exceptions.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class CameraTools::HLTVCameraOverride : public C_HLTVCamera {
public:
	using C_HLTVCamera::m_nCameraMode;
	using C_HLTVCamera::m_iCameraMan;
	using C_HLTVCamera::m_vCamOrigin;
	using C_HLTVCamera::m_aCamAngle;
	using C_HLTVCamera::m_iTraget1;
	using C_HLTVCamera::m_iTraget2;
	using C_HLTVCamera::m_flFOV;
	using C_HLTVCamera::m_flOffset;
	using C_HLTVCamera::m_flDistance;
	using C_HLTVCamera::m_flLastDistance;
	using C_HLTVCamera::m_flTheta;
	using C_HLTVCamera::m_flPhi;
	using C_HLTVCamera::m_flInertia;
	using C_HLTVCamera::m_flLastAngleUpdateTime;
	using C_HLTVCamera::m_bEntityPacketReceived;
	using C_HLTVCamera::m_nNumSpectators;
	using C_HLTVCamera::m_szTitleText;
	using C_HLTVCamera::m_LastCmd;
	using C_HLTVCamera::m_vecVelocity;
};

CameraTools::CameraTools() {
	setModeHook = 0;
	setPrimaryTargetHook = 0;
	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(g_pFullFileSystem, "resource/ui/spectatortournament.res", "mod");

	force_mode = new ConVar("statusspec_cameratools_force_mode", "0", FCVAR_NONE, "if a valid mode, force the camera mode to this", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ChangeForceMode(var, pOldValue, flOldValue); });
	force_target = new ConVar("statusspec_cameratools_force_target", "-1", FCVAR_NONE, "if a valid target, force the camera target to this", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ChangeForceTarget(var, pOldValue, flOldValue); });
	force_valid_target = new ConVar("statusspec_cameratools_force_valid_target", "0", FCVAR_NONE, "forces the camera to only have valid targets", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ToggleForceValidTarget(var, pOldValue, flOldValue); });
	spec_player = new ConCommand("statusspec_cameratools_spec_player", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>()->SpecPlayer(command); }, "spec a certain player", FCVAR_NONE);
	spec_player_alive = new ConVar("statusspec_cameratools_spec_player_alive", "1", FCVAR_NONE, "prevent speccing dead players");
	spec_pos = new ConCommand("statusspec_cameratools_spec_pos", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>()->SpecPosition(command); }, "spec a certain camera position", FCVAR_NONE);
}

bool CameraTools::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!g_pFullFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetCameraAngle();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetCameraAngle for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetMode for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetPrimaryTarget for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!g_pVGuiSchemeManager) {
		PRINT_TAG();
		Warning("Required interface vgui::ISchemeManager for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!Player::nameRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player name retrieval for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());
	}

	return ready;
}

void CameraTools::SetModeOverride(C_HLTVCamera *hltvcamera, int &iMode) {
	int forceMode = force_mode->GetInt();

	if (forceMode == OBS_MODE_FIXED || forceMode == OBS_MODE_IN_EYE || forceMode == OBS_MODE_CHASE || forceMode == OBS_MODE_ROAMING) {
		iMode = forceMode;
	}
}

void CameraTools::SetPrimaryTargetOverride(C_HLTVCamera *hltvcamera, int &nEntity) {
	int forceTarget = force_target->GetInt();

	if (Interfaces::pClientEntityList->GetClientEntity(forceTarget)) {
		nEntity = forceTarget;
	}

	if (!Interfaces::pClientEntityList->GetClientEntity(nEntity)) {
		nEntity = ((HLTVCameraOverride *)hltvcamera)->m_iTraget1;
	}
}

void CameraTools::ChangeForceMode(IConVar *var, const char *pOldValue, float flOldValue) {
	int forceMode = force_mode->GetInt();

	if (forceMode == OBS_MODE_FIXED || forceMode == OBS_MODE_IN_EYE || forceMode == OBS_MODE_CHASE || forceMode == OBS_MODE_ROAMING) {
		if (!setModeHook) {
			setModeHook = Funcs::AddHook_C_HLTVCamera_SetMode(std::bind(&CameraTools::SetModeOverride, this, std::placeholders::_1, std::placeholders::_2));
		}

		try {
			Funcs::GetFunc_C_HLTVCamera_SetMode()(Interfaces::GetHLTVCamera(), forceMode);
		}
		catch (bad_pointer) {
			Warning("Error in setting mode.\n");
		}
	}
	else {
		var->SetValue(OBS_MODE_NONE);

		if (setModeHook) {
			Funcs::RemoveHook_C_HLTVCamera_SetMode(setModeHook);
			setModeHook = 0;
		}
	}
}

void CameraTools::ChangeForceTarget(IConVar *var, const char *pOldValue, float flOldValue) {
	int forceTarget = force_target->GetInt();

	if (Interfaces::pClientEntityList->GetClientEntity(forceTarget)) {
		if (!setPrimaryTargetHook) {
			setPrimaryTargetHook = Funcs::AddHook_C_HLTVCamera_SetPrimaryTarget(std::bind(&CameraTools::SetPrimaryTargetOverride, this, std::placeholders::_1, std::placeholders::_2));
		}

		try {
			Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), forceTarget);
		}
		catch (bad_pointer) {
			Warning("Error in setting target.\n");
		}
	}
	else {
		if (!force_valid_target->GetBool()) {
			if (setPrimaryTargetHook) {
				Funcs::RemoveHook_C_HLTVCamera_SetPrimaryTarget(setPrimaryTargetHook);
				setPrimaryTargetHook = 0;
			}
		}
	}
}

void CameraTools::SpecPlayer(const CCommand &command) {
	if (command.ArgC() >= 3 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2))) {
		try {
			if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
				vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
					vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

					if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
						for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
							vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

							if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
								vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(playerPanel, GAME_PANEL_MODULE));

								if (panel) {
									KeyValues *dialogVariables = panel->GetDialogVariables();

									if (dialogVariables) {
										const char *name = dialogVariables->GetString("playername");

										for (Player player : Player::Iterable()) {
											if (player.GetName().compare(name) == 0) {
												int baseX = 0;
												int baseY = 0;
												int deltaX = 0;
												int deltaY = 0;

												if (player.GetTeam() == TFTeam_Red) {
													if (atoi(command.Arg(1)) != TFTeam_Red) {
														continue;
													}

													baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_offset_x"));
													baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_y"));
													deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_x"));
													deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_y"));
												}
												else if (player.GetTeam() == TFTeam_Blue) {
													if (atoi(command.Arg(1)) != TFTeam_Blue) {
														continue;
													}

													baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_offset_x"));
													baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_y"));
													deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_x"));
													deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_y"));
												}

												int position = atoi(command.Arg(2));

												int x, y;
												g_pVGuiPanel->GetPos(playerPanel, x, y);

												int relativeX = x - baseX;
												int relativeY = y - baseY;

												if (deltaX != 0 && relativeX / deltaX != position) {
													continue;
												}
												else if (relativeX != 0 && deltaX == 0) {
													continue;
												}

												if (deltaY != 0 && relativeY / deltaY != position) {
													continue;
												}
												else if (relativeY != 0 && deltaY == 0) {
													continue;
												}

												if (!spec_player_alive->GetBool() || player.IsAlive()) {
													Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), player->entindex());
												}

												return;
											}
										}
									}
								}
							}
						}

						break;
					}
				}
			}

			Warning("No suitable player to switch to found!\n");
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}
	else if (command.ArgC() == 2 && IsInteger(command.Arg(1))) {
		Player player = atoi(command.Arg(1));

		if (player) {
			if (!spec_player_alive->GetBool() || player.IsAlive()) {
				try {
					Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), player->entindex());
				}
				catch (bad_pointer &e) {
					Warning("%s\n", e.what());
				}
			}
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_player <team> <position> || statusspec_cameratools_spec_player <index>\n");

		return;
	}
}

void CameraTools::SpecPosition(const CCommand &command) {
	if (command.ArgC() >= 6 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)) && IsInteger(command.Arg(4)) && IsInteger(command.Arg(5))) {
		try {
			HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

			hltvcamera->m_nCameraMode = OBS_MODE_FIXED;
			hltvcamera->m_iCameraMan = 0;
			hltvcamera->m_vCamOrigin.x = atoi(command.Arg(1));
			hltvcamera->m_vCamOrigin.y = atoi(command.Arg(2));
			hltvcamera->m_vCamOrigin.z = atoi(command.Arg(3));
			hltvcamera->m_aCamAngle.x = atoi(command.Arg(4));
			hltvcamera->m_aCamAngle.y = atoi(command.Arg(5));
			hltvcamera->m_iTraget1 = 0;
			hltvcamera->m_iTraget2 = 0;
			hltvcamera->m_flLastAngleUpdateTime = Interfaces::pEngineTool->GetRealTime();

			Funcs::GetFunc_C_HLTVCamera_SetCameraAngle()(hltvcamera, hltvcamera->m_aCamAngle);
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>\n");

		return;
	}
}

void CameraTools::ToggleForceValidTarget(IConVar *var, const char *pOldValue, float flOldValue) {
	if (force_valid_target->GetBool()) {
		if (!setPrimaryTargetHook) {
			setPrimaryTargetHook = Funcs::AddHook_C_HLTVCamera_SetPrimaryTarget(std::bind(&CameraTools::SetPrimaryTargetOverride, this, std::placeholders::_1, std::placeholders::_2));
		}
	}
	else {
		if (!Interfaces::pClientEntityList->GetClientEntity(force_target->GetInt())) {
			if (setPrimaryTargetHook) {
				Funcs::RemoveHook_C_HLTVCamera_SetPrimaryTarget(setPrimaryTargetHook);
				setPrimaryTargetHook = 0;
			}
		}
	}
}