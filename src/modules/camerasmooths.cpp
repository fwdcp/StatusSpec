/*
 *  camerasmooths.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "camerasmooths.h"

#include "cbase.h"
#include "convar.h"
#include "hltvcamera.h"
#include "shareddefs.h"
#include "toolframework/ienginetool.h"

#include "../common.h"
#include "../exceptions.h"
#include "../funcs.h"
#include "../ifaces.h"

class CameraSmooths::HLTVCameraOverride : public C_HLTVCamera {
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

CameraSmooths::CameraSmooths() {
	inToolModeHook = 0;
	isThirdPersonCameraHook = 0;
	setupEngineViewHook = 0;
	smoothEnding = false;
	smoothEndMode = OBS_MODE_NONE;
	smoothEndTarget = 0;
	smoothInProgress = false;
	smoothLastAngles = QAngle();
	smoothLastOrigin = Vector();
	smoothLastTime = 0;

	enabled = new ConVar("statusspec_camerasmooths_enabled", "0", FCVAR_NONE, "smooth transition between camera positions", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraSmooths>()->ToggleEnabled(var, pOldValue, flOldValue); });
	max_angle_difference = new ConVar("statusspec_camerasmooths_max_angle_difference", "90", FCVAR_NONE, "max angle difference at which smoothing will be performed");
	max_distance = new ConVar("statusspec_camerasmooths_max_distance", "800", FCVAR_NONE, "max distance at which smoothing will be performed");
	move_speed = new ConVar("statusspec_camerasmooths_move_speed", "800", FCVAR_NONE, "speed to move view per second");
}

bool CameraSmooths::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientEngineTools) {
		PRINT_TAG();
		Warning("Required interface IClientEngineTools for module %s not available!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetCameraAngle();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetCameraAngle for module %s not available!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetMode for module %s not available!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<CameraSmooths>().c_str());
	}

	return ready;
}

bool CameraSmooths::InToolModeOverride() {
	if (!Interfaces::pEngineClient->IsHLTV()) {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	if (smoothInProgress) {
		RETURN_META_VALUE(MRES_OVERRIDE, true);
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}

bool CameraSmooths::IsThirdPersonCameraOverride() {
	if (!Interfaces::pEngineClient->IsHLTV()) {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	if (smoothInProgress) {
		RETURN_META_VALUE(MRES_OVERRIDE, true);
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}

bool CameraSmooths::SetupEngineViewOverride(Vector &origin, QAngle &angles, float &fov) {
	if (!Interfaces::pEngineClient->IsHLTV()) {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

	if (hltvcamera->m_nCameraMode == OBS_MODE_IN_EYE || hltvcamera->m_nCameraMode == OBS_MODE_CHASE) {
		if (hltvcamera->m_iTraget1 != smoothEndTarget || (hltvcamera->m_nCameraMode != smoothEndMode && !smoothInProgress)) {
			smoothEndMode = hltvcamera->m_nCameraMode;
			smoothEndTarget = hltvcamera->m_iTraget1;

			Vector moveVector = origin - smoothLastOrigin;
			Vector currentAngleVector(smoothLastAngles.x, smoothLastAngles.y, smoothLastAngles.z);
			Vector targetAngleVector(angles.x, angles.y, angles.z);

			float angle = acos(currentAngleVector.Dot(targetAngleVector) / (currentAngleVector.Length() * targetAngleVector.Length())) * 180.f / 3.14159265358979323846f;

			smoothInProgress = moveVector.Length() < max_distance->GetFloat() && angle < max_angle_difference->GetFloat();
		}
	}
	else {
		smoothInProgress = false;
	}

	if (smoothInProgress) {
		float moveDistance = move_speed->GetFloat() * (Interfaces::pEngineTool->HostTime() - smoothLastTime);
		
		Vector moveVector = origin - smoothLastOrigin;
		Vector currentAngleVector(smoothLastAngles.x, smoothLastAngles.y, smoothLastAngles.z);
		Vector targetAngleVector(angles.x, angles.y, angles.z);

		float angle = acos(currentAngleVector.Dot(targetAngleVector) / (currentAngleVector.Length() * targetAngleVector.Length())) * 180.f / 3.14159265358979323846f;

		if (moveDistance < moveVector.Length() && moveVector.Length() < max_distance->GetFloat() && angle < max_angle_difference->GetFloat()) {
			float movePercentage = moveDistance / moveVector.Length();

			moveVector *= movePercentage;

			origin = smoothLastOrigin + moveVector;

			angles.x = smoothLastAngles.x + ((angles.x - smoothLastAngles.x) * movePercentage);
			angles.y = smoothLastAngles.y + ((angles.y - smoothLastAngles.y) * movePercentage);
			angles.z = smoothLastAngles.z + ((angles.z - smoothLastAngles.z) * movePercentage);

			Funcs::CallFunc_C_HLTVCamera_SetCameraAngle(hltvcamera, angles);
			hltvcamera->m_vCamOrigin = origin;

			smoothEnding = false;
			smoothInProgress = true;
		}
		else {
			C_HLTVCamera *hltvcamera = Interfaces::GetHLTVCamera();

			if (hltvcamera) {
				Funcs::CallFunc_C_HLTVCamera_SetMode(hltvcamera, OBS_MODE_ROAMING);
			}

			smoothEnding = true;
			smoothInProgress = false;
		}

		smoothLastAngles = angles;
		smoothLastOrigin = origin;
		smoothLastTime = Interfaces::pEngineTool->HostTime();

		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}
	else if (smoothEnding) {
		C_HLTVCamera *hltvcamera = Interfaces::GetHLTVCamera();

		if (hltvcamera) {
			Funcs::CallFunc_C_HLTVCamera_SetMode(hltvcamera, smoothEndMode);
		}
	}

	smoothEnding = false;
	smoothEndMode = hltvcamera->m_nCameraMode;
	smoothEndTarget = hltvcamera->m_iTraget1;
	smoothInProgress = false;
	smoothLastAngles = angles;
	smoothLastOrigin = origin;
	smoothLastTime = Interfaces::pEngineTool->HostTime();

	RETURN_META_VALUE(MRES_IGNORED, false);
}

void CameraSmooths::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!inToolModeHook) {
			inToolModeHook = Funcs::AddHook_IClientEngineTools_InToolMode(Interfaces::pClientEngineTools, SH_MEMBER(this, &CameraSmooths::InToolModeOverride), false);
		}

		if (!isThirdPersonCameraHook) {
			isThirdPersonCameraHook = Funcs::AddHook_IClientEngineTools_IsThirdPersonCamera(Interfaces::pClientEngineTools, SH_MEMBER(this, &CameraSmooths::IsThirdPersonCameraOverride), false);
		}

		if (!setupEngineViewHook) {
			setupEngineViewHook = Funcs::AddHook_IClientEngineTools_SetupEngineView(Interfaces::pClientEngineTools, SH_MEMBER(this, &CameraSmooths::SetupEngineViewOverride), false);
		}
	}
	else {
		if (inToolModeHook) {
			Funcs::RemoveHook(inToolModeHook);
			inToolModeHook = 0;
		}

		if (isThirdPersonCameraHook) {
			Funcs::RemoveHook(isThirdPersonCameraHook);
			isThirdPersonCameraHook = 0;
		}

		if (setupEngineViewHook) {
			Funcs::RemoveHook(setupEngineViewHook);
			setupEngineViewHook = 0;
		}
	}
}