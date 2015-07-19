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
#include <thread>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class C_BaseEntity;
#include "cbase.h"
#include "convar.h"
#include "filesystem.h"
#include "usercmd.h"
#include "hltvcamera.h"
#include "icliententity.h"
#include "iclientmode.h"
#include "in_buttons.h"
#include "KeyValues.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
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
	currentlyUpdating = false;
	frameHook = 0;
	setModeHook = 0;
	setPrimaryTargetHook = 0;
	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/spectatortournament.res", "mod");

	force_mode = new ConVar("statusspec_cameratools_force_mode", "0", FCVAR_NONE, "if a valid mode, force the camera mode to this", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ChangeForceMode(var, pOldValue, flOldValue); });
	force_target = new ConVar("statusspec_cameratools_force_target", "-1", FCVAR_NONE, "if a valid target, force the camera target to this", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ChangeForceTarget(var, pOldValue, flOldValue); });
	force_valid_target = new ConVar("statusspec_cameratools_force_valid_target", "0", FCVAR_NONE, "forces the camera to only have valid targets", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ToggleForceValidTarget(var, pOldValue, flOldValue); });
	spec_player = new ConCommand("statusspec_cameratools_spec_player", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>()->SpecPlayer(command); }, "spec a certain player", FCVAR_NONE);
	spec_player_alive = new ConVar("statusspec_cameratools_spec_player_alive", "1", FCVAR_NONE, "prevent speccing dead players");
	spec_pos = new ConCommand("statusspec_cameratools_spec_pos", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>()->SpecPosition(command); }, "spec a certain camera position", FCVAR_NONE);
	state = new ConVar("statusspec_cameratools_state", "{}", FCVAR_NONE, "JSON of camera tools state", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ChangeState(var, pOldValue, flOldValue); });
	state_enabled = new ConVar("statusspec_cameratools_state_enabled", "0", FCVAR_NONE, "enable exposure of camera tools state", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>()->ToggleStateEnabled(var, pOldValue, flOldValue); });
}

bool CameraTools::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<CameraTools>().c_str());

		ready = false;
	}

	if (!Interfaces::pFileSystem) {
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

void CameraTools::UpdateState() {
	std::string currentState;

	if (Interfaces::pEngineClient->IsInGame()) {
		try {
			HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

			rapidjson::Document cameraState;
			cameraState.SetObject();
			cameraState.AddMember("mode", hltvcamera->m_nCameraMode, cameraState.GetAllocator());
			cameraState.AddMember("camera", hltvcamera->m_iCameraMan, cameraState.GetAllocator());
			cameraState.AddMember("origin", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["origin"].AddMember("x", hltvcamera->m_vCamOrigin.x, cameraState.GetAllocator());
			cameraState["origin"].AddMember("y", hltvcamera->m_vCamOrigin.y, cameraState.GetAllocator());
			cameraState["origin"].AddMember("z", hltvcamera->m_vCamOrigin.z, cameraState.GetAllocator());
			cameraState.AddMember("angle", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["angle"].AddMember("x", hltvcamera->m_aCamAngle.x, cameraState.GetAllocator());
			cameraState["angle"].AddMember("y", hltvcamera->m_aCamAngle.y, cameraState.GetAllocator());
			cameraState["angle"].AddMember("z", hltvcamera->m_aCamAngle.z, cameraState.GetAllocator());
			cameraState.AddMember("target", rapidjson::Value(rapidjson::kArrayType), cameraState.GetAllocator());
			cameraState["target"].PushBack(hltvcamera->m_iTraget1, cameraState.GetAllocator());
			cameraState["target"].PushBack(hltvcamera->m_iTraget2, cameraState.GetAllocator());
			cameraState.AddMember("fov", hltvcamera->m_flFOV, cameraState.GetAllocator());
			cameraState.AddMember("offset", hltvcamera->m_flOffset, cameraState.GetAllocator());
			cameraState.AddMember("distance", hltvcamera->m_flDistance, cameraState.GetAllocator());
			cameraState.AddMember("lastdistance", hltvcamera->m_flLastDistance, cameraState.GetAllocator());
			cameraState.AddMember("theta", hltvcamera->m_flTheta, cameraState.GetAllocator());
			cameraState.AddMember("phi", hltvcamera->m_flPhi, cameraState.GetAllocator());
			cameraState.AddMember("inertia", hltvcamera->m_flInertia, cameraState.GetAllocator());
			cameraState.AddMember("cmd", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["cmd"].AddMember("angle", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["cmd"]["angle"].AddMember("x", hltvcamera->m_LastCmd.viewangles.x, cameraState.GetAllocator());
			cameraState["cmd"]["angle"].AddMember("y", hltvcamera->m_LastCmd.viewangles.y, cameraState.GetAllocator());
			cameraState["cmd"]["angle"].AddMember("z", hltvcamera->m_LastCmd.viewangles.z, cameraState.GetAllocator());
			cameraState["cmd"].AddMember("buttons", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["cmd"]["buttons"].AddMember("speed", (hltvcamera->m_LastCmd.buttons & IN_SPEED) != 0, cameraState.GetAllocator());
			cameraState["cmd"].AddMember("forwardmove", hltvcamera->m_LastCmd.forwardmove, cameraState.GetAllocator());
			cameraState["cmd"].AddMember("sidemove", hltvcamera->m_LastCmd.sidemove, cameraState.GetAllocator());
			cameraState["cmd"].AddMember("upmove", hltvcamera->m_LastCmd.upmove, cameraState.GetAllocator());
			cameraState.AddMember("velocity", rapidjson::Value(rapidjson::kObjectType), cameraState.GetAllocator());
			cameraState["velocity"].AddMember("x", hltvcamera->m_vecVelocity.x, cameraState.GetAllocator());
			cameraState["velocity"].AddMember("y", hltvcamera->m_vecVelocity.y, cameraState.GetAllocator());
			cameraState["velocity"].AddMember("z", hltvcamera->m_vecVelocity.z, cameraState.GetAllocator());

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			cameraState.Accept(writer);

			currentState = buffer.GetString();
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}
	else {
		currentState = "{}";
	}

	currentlyUpdating = true;
	state->SetValue(currentState.c_str());
	currentlyUpdating = false;
}

void CameraTools::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_START) {
		std::thread update(std::bind(&CameraTools::UpdateState, this));
		update.detach();
	}

	RETURN_META(MRES_IGNORED);
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
			Funcs::CallFunc_C_HLTVCamera_SetMode(Interfaces::GetHLTVCamera(), forceMode);
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
			Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), forceTarget);
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

void CameraTools::ChangeState(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!state_enabled->GetBool()) {
		return;
	}

	if (currentlyUpdating) {
		return;
	}

	try {
		HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

		rapidjson::Document newState;
		newState.Parse(state->GetString());

		if (!newState.IsObject()) {
			return;
		}

		int cameraman = hltvcamera->m_iCameraMan;
		if (newState.HasMember("camera") && newState["camera"].IsInt()) {
			cameraman = newState["camera"].GetInt();
		}

		hltvcamera->m_iCameraMan = cameraman;

		if (cameraman <= 0) {
			int mode = hltvcamera->m_nCameraMode;
			if (newState.HasMember("mode") && newState["mode"].IsInt()) {
				mode = newState["mode"].GetInt();
			}

			if (hltvcamera->m_nCameraMode != mode) {
				Funcs::CallFunc_C_HLTVCamera_SetMode(Interfaces::GetHLTVCamera(), newState["mode"].GetInt());
			}

			if (mode == OBS_MODE_ROAMING) {
				if (newState.HasMember("cmd") && newState["cmd"].IsObject()) {
					if (newState["cmd"].HasMember("angle") && newState["cmd"]["angle"].IsObject()) {
						if (newState["cmd"]["angle"].HasMember("x") && newState["cmd"]["angle"]["x"].IsDouble()) {
							hltvcamera->m_LastCmd.viewangles.x = newState["cmd"]["angle"]["x"].GetDouble();
						}

						if (newState["cmd"]["angle"].HasMember("y") && newState["cmd"]["angle"]["y"].IsDouble()) {
							hltvcamera->m_LastCmd.viewangles.y = newState["cmd"]["angle"]["y"].GetDouble();
						}

						if (newState["cmd"]["angle"].HasMember("z") && newState["cmd"]["angle"]["z"].IsDouble()) {
							hltvcamera->m_LastCmd.viewangles.z = newState["cmd"]["angle"]["z"].GetDouble();
						}
					}

					if (newState["cmd"].HasMember("buttons") && newState["cmd"]["buttons"].IsObject()) {
						if (newState["cmd"]["buttons"].HasMember("speed") && newState["cmd"]["buttons"]["speed"].IsBool()) {
							if (newState["cmd"]["buttons"]["speed"].GetBool()) {
								hltvcamera->m_LastCmd.buttons |= IN_SPEED;
							}
							else {
								hltvcamera->m_LastCmd.buttons &= ~(IN_SPEED);
							}
						}
					}

					if (newState["cmd"].HasMember("forwardmove") && newState["cmd"]["forwardmove"].IsDouble()) {
						hltvcamera->m_LastCmd.forwardmove = newState["cmd"]["forwardmove"].GetDouble();
					}

					if (newState["cmd"].HasMember("sidemove") && newState["cmd"]["sidemove"].IsDouble()) {
						hltvcamera->m_LastCmd.sidemove = newState["cmd"]["sidemove"].GetDouble();
					}

					if (newState["cmd"].HasMember("upmove") && newState["cmd"]["upmove"].IsDouble()) {
						hltvcamera->m_LastCmd.upmove = newState["cmd"]["upmove"].GetDouble();
					}
				}

				if (newState.HasMember("velocity") && newState["velocity"].IsObject()) {
					if (newState["velocity"].HasMember("x") && newState["velocity"]["x"].IsDouble()) {
						hltvcamera->m_vecVelocity.x = newState["velocity"]["x"].GetDouble();
					}

					if (newState["velocity"].HasMember("y") && newState["velocity"]["y"].IsDouble()) {
						hltvcamera->m_vecVelocity.y = newState["velocity"]["y"].GetDouble();
					}

					if (newState["velocity"].HasMember("z") && newState["velocity"]["z"].IsDouble()) {
						hltvcamera->m_vecVelocity.z = newState["velocity"]["z"].GetDouble();
					}
				}

				if (newState.HasMember("origin") && newState["origin"].IsObject()) {
					if (newState["origin"].HasMember("x") && newState["origin"]["x"].IsDouble()) {
						hltvcamera->m_vCamOrigin.x = newState["origin"]["x"].GetDouble();
					}

					if (newState["origin"].HasMember("y") && newState["origin"]["y"].IsDouble()) {
						hltvcamera->m_vCamOrigin.y = newState["origin"]["y"].GetDouble();
					}

					if (newState["origin"].HasMember("z") && newState["origin"]["z"].IsDouble()) {
						hltvcamera->m_vCamOrigin.z = newState["origin"]["z"].GetDouble();
					}
				}

				if (newState.HasMember("angle") && newState["angle"].IsObject()) {
					QAngle angle;

					if (newState["angle"].HasMember("x") && newState["angle"]["x"].IsDouble()) {
						angle.x = newState["angle"]["x"].GetDouble();
					}
					else {
						angle.x = hltvcamera->m_aCamAngle.x;
					}

					if (newState["angle"].HasMember("y") && newState["angle"]["y"].IsDouble()) {
						angle.y = newState["angle"]["y"].GetDouble();
					}
					else {
						angle.y = hltvcamera->m_aCamAngle.y;
					}

					if (newState["angle"].HasMember("z") && newState["angle"]["z"].IsDouble()) {
						angle.z = newState["angle"]["z"].GetDouble();
					}
					else {
						angle.z = hltvcamera->m_aCamAngle.z;
					}

					if (hltvcamera->m_aCamAngle != angle) {
						Funcs::CallFunc_C_HLTVCamera_SetCameraAngle(Interfaces::GetHLTVCamera(), angle);
					}
				}

				if (newState.HasMember("fov") && newState["fov"].IsDouble()) {
					hltvcamera->m_flFOV = newState["fov"].GetDouble();
				}
			}
			else if (mode == OBS_MODE_FIXED) {
				if (newState.HasMember("origin") && newState["origin"].IsObject()) {
					if (newState["origin"].HasMember("x") && newState["origin"]["x"].IsDouble()) {
						hltvcamera->m_vCamOrigin.x = newState["origin"]["x"].GetDouble();
					}

					if (newState["origin"].HasMember("y") && newState["origin"]["y"].IsDouble()) {
						hltvcamera->m_vCamOrigin.y = newState["origin"]["y"].GetDouble();
					}

					if (newState["origin"].HasMember("z") && newState["origin"]["z"].IsDouble()) {
						hltvcamera->m_vCamOrigin.z = newState["origin"]["z"].GetDouble();
					}
				}

				if (newState.HasMember("angle") && newState["angle"].IsObject()) {
					QAngle angle;

					if (newState["angle"].HasMember("x") && newState["angle"]["x"].IsDouble()) {
						angle.x = newState["angle"]["x"].GetDouble();
					}
					else {
						angle.x = hltvcamera->m_aCamAngle.x;
					}

					if (newState["angle"].HasMember("y") && newState["angle"]["y"].IsDouble()) {
						angle.y = newState["angle"]["y"].GetDouble();
					}
					else {
						angle.y = hltvcamera->m_aCamAngle.y;
					}

					if (newState["angle"].HasMember("z") && newState["angle"]["z"].IsDouble()) {
						angle.z = newState["angle"]["z"].GetDouble();
					}
					else {
						angle.z = hltvcamera->m_aCamAngle.z;
					}

					if (hltvcamera->m_aCamAngle != angle) {
						Funcs::CallFunc_C_HLTVCamera_SetCameraAngle(Interfaces::GetHLTVCamera(), angle);
					}
				}

				if (newState.HasMember("fov") && newState["fov"].IsDouble()) {
					hltvcamera->m_flFOV = newState["fov"].GetDouble();
				}

				if (newState.HasMember("target") && newState["target"].IsArray()) {
					if (newState["target"][0].IsInt()) {
						if (hltvcamera->m_iTraget1 != newState["target"][0].GetInt()) {
							Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
						}
					}
				}

				if (newState.HasMember("inertia") && newState["inertia"].IsDouble()) {
					hltvcamera->m_flInertia = newState["inertia"].GetDouble();
				}
			}
			else if (mode == OBS_MODE_IN_EYE) {
				if (newState.HasMember("target") && newState["target"].IsArray()) {
					if (newState["target"][0].IsInt()) {
						if (hltvcamera->m_iTraget1 != newState["target"][0].GetInt()) {
							Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
						}
					}
				}
			}
			else if (mode == OBS_MODE_CHASE) {
				if (newState.HasMember("target") && newState["target"].IsArray()) {
					if (newState["target"][0].IsInt()) {
						if (hltvcamera->m_iTraget1 != newState["target"][0].GetInt()) {
							Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
						}
					}

					if (newState["target"][1].IsInt()) {
						hltvcamera->m_iTraget2 = newState["target"][1].GetInt();
					}
				}

				if (newState.HasMember("phi") && newState["phi"].IsDouble()) {
					hltvcamera->m_flPhi = newState["phi"].GetDouble();
				}

				if (newState.HasMember("theta") && newState["theta"].IsDouble()) {
					hltvcamera->m_flTheta = newState["theta"].GetDouble();
				}

				if (newState.HasMember("angle") && newState["angle"].IsObject()) {
					QAngle angle;

					if (newState["angle"].HasMember("x") && newState["angle"]["x"].IsDouble()) {
						angle.x = newState["angle"]["x"].GetDouble();
					}
					else {
						angle.x = hltvcamera->m_aCamAngle.x;
					}

					if (newState["angle"].HasMember("y") && newState["angle"]["y"].IsDouble()) {
						angle.y = newState["angle"]["y"].GetDouble();
					}
					else {
						angle.y = hltvcamera->m_aCamAngle.y;
					}

					if (newState["angle"].HasMember("z") && newState["angle"]["z"].IsDouble()) {
						angle.z = newState["angle"]["z"].GetDouble();
					}
					else {
						angle.z = hltvcamera->m_aCamAngle.z;
					}

					if (hltvcamera->m_aCamAngle != angle) {
						Funcs::CallFunc_C_HLTVCamera_SetCameraAngle(Interfaces::GetHLTVCamera(), angle);
					}
				}

				if (newState.HasMember("distance") && newState["distance"].IsDouble()) {
					hltvcamera->m_flDistance = newState["distance"].GetDouble();
				}

				if (newState.HasMember("offset") && newState["offset"].IsDouble()) {
					hltvcamera->m_flOffset = newState["offset"].GetDouble();
				}

				if (newState.HasMember("lastdistance") && newState["lastdistance"].IsDouble()) {
					hltvcamera->m_flLastDistance = newState["lastdistance"].GetDouble();
				}

				if (newState.HasMember("inertia") && newState["inertia"].IsDouble()) {
					hltvcamera->m_flInertia = newState["inertia"].GetDouble();
				}

				if (newState.HasMember("origin") && newState["origin"].IsObject()) {
					if (newState["origin"].HasMember("x") && newState["origin"]["x"].IsDouble()) {
						hltvcamera->m_vCamOrigin.x = newState["origin"]["x"].GetDouble();
					}

					if (newState["origin"].HasMember("y") && newState["origin"]["y"].IsDouble()) {
						hltvcamera->m_vCamOrigin.y = newState["origin"]["y"].GetDouble();
					}

					if (newState["origin"].HasMember("z") && newState["origin"]["z"].IsDouble()) {
						hltvcamera->m_vCamOrigin.z = newState["origin"]["z"].GetDouble();
					}
				}
			}
		}

		std::thread update(std::bind(&CameraTools::UpdateState, this));
		update.detach();
	}
	catch (bad_pointer &e) {
		Warning("%s\n", e.what());
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
													Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), player->entindex());
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
					Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), player->entindex());
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

			Interfaces::pEngineClient->SetViewAngles(hltvcamera->m_aCamAngle);
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

void CameraTools::ToggleStateEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (state_enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &CameraTools::FrameHook), true);
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