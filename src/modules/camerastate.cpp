/*
 *  camerastate.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "camerastate.h"

#include <functional>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "cbase.h"
#include "hltvcamera.h"
#include "in_buttons.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../exceptions.h"
#include "../funcs.h"
#include "../ifaces.h"

class CameraState::HLTVCameraOverride : public C_HLTVCamera {
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

class CameraState::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName, std::function<void()> updateFunction);

	virtual void OnTick();
private:
	std::function<void()> updateState;
};

CameraState::CameraState() {
	panel = nullptr;

	change = new ConVar("statusspec_camerastate_change", "{}", FCVAR_NONE, "JSON to change camera state to", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraState>()->ChangeState(var, pOldValue, flOldValue); });
	current = new ConVar("statusspec_camerastate_current", "{}", FCVAR_NONE, "JSON of current camera state");
	enabled = new ConVar("statusspec_camerastate_enabled", "0", FCVAR_NONE, "enable exposure of camera state", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraState>()->ToggleEnabled(var, pOldValue, flOldValue); });
}

bool CameraState::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<CameraState>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetCameraAngle();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetCameraAngle for module %s not available!\n", g_ModuleManager->GetModuleName<CameraState>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetMode for module %s not available!\n", g_ModuleManager->GetModuleName<CameraState>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetPrimaryTarget for module %s not available!\n", g_ModuleManager->GetModuleName<CameraState>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<CameraState>().c_str());
	}

	return ready;
}

void CameraState::UpdateState() {
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

	current->SetValue(currentState.c_str());
}

void CameraState::ChangeState(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!enabled->GetBool()) {
		return;
	}

	try {
		HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

		rapidjson::Document newState;
		newState.Parse(change->GetString());

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
				Funcs::GetFunc_C_HLTVCamera_SetMode()(Interfaces::GetHLTVCamera(), newState["mode"].GetInt());
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
						Funcs::GetFunc_C_HLTVCamera_SetCameraAngle()(Interfaces::GetHLTVCamera(), angle);
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
						Funcs::GetFunc_C_HLTVCamera_SetCameraAngle()(Interfaces::GetHLTVCamera(), angle);
					}
				}

				if (newState.HasMember("fov") && newState["fov"].IsDouble()) {
					hltvcamera->m_flFOV = newState["fov"].GetDouble();
				}

				if (newState.HasMember("target") && newState["target"].IsArray()) {
					if (newState["target"][0].IsInt()) {
						if (hltvcamera->m_iTraget1 != newState["target"][0].GetInt()) {
							Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
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
							Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
						}
					}
				}
			}
			else if (mode == OBS_MODE_CHASE) {
				if (newState.HasMember("target") && newState["target"].IsArray()) {
					if (newState["target"][0].IsInt()) {
						if (hltvcamera->m_iTraget1 != newState["target"][0].GetInt()) {
							Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget()(Interfaces::GetHLTVCamera(), newState["target"][0].GetInt());
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
						Funcs::GetFunc_C_HLTVCamera_SetCameraAngle()(Interfaces::GetHLTVCamera(), angle);
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
	}
	catch (bad_pointer &e) {
		Warning("%s\n", e.what());
	}
}

void CameraState::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			panel = new Panel(nullptr, "CameraState", std::bind(&CameraState::UpdateState, this));
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

CameraState::Panel::Panel(vgui::Panel *parent, const char *panelName, std::function<void()> updateFunction) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());

	updateState = updateFunction;
}

void CameraState::Panel::OnTick() {
	updateState();
}
