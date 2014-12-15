/*
 *  cameratools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <algorithm>
#include <array>
#include <vector>

#include "convar.h"
#include "vgui/IScheme.h"
#include "vgui_controls/EditablePanel.h"

#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"
#include "../tfdefs.h"

class CameraTools {
public:
	CameraTools();
private:
	KeyValues *specguiSettings;

	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
};

class HLTVCameraOverride : public C_HLTVCamera {
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

extern CameraTools *g_CameraTools;