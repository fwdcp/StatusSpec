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

#include "cdll_int.h"
#include "convar.h"
#include "igameevents.h"

#include "../modules.h"

class CCommand;
class C_HLTVCamera;
class ConCommand;
class ConVar;
class KeyValues;

class CameraTools : public Module, IGameEventListener2 {
public:
	CameraTools(std::string name);

	static bool CheckDependencies(std::string name);

	virtual void FireGameEvent(IGameEvent *event);
private:
	void FrameHook(ClientFrameStage_t curStage);

	bool currentlyUpdating;
	int frameHook;
	int inToolModeHook;
	int isThirdPersonCameraHook;
	int setupEngineViewHook;
	bool smoothEnding;
	int smoothEndMode;
	int smoothEndTarget;
	bool smoothInProgress;
	QAngle smoothLastAngles;
	Vector smoothLastOrigin;
	float smoothLastTime;
	KeyValues *specguiSettings;

	bool InToolModeOverride();
	bool IsThirdPersonCameraOverride();
	bool SetupEngineViewOverride(Vector &origin, QAngle &angles, float &fov);
	void UpdateState();

	class HLTVCameraOverride;

	ConVar *killer_follow_enabled;
	ConVar *smooth_camera_switches_enabled;
	ConVar *smooth_camera_switches_max_angle_difference;
	ConVar *smooth_camera_switches_max_distance;
	ConVar *smooth_camera_switches_move_speed;
	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	ConVar *state;
	ConVar *state_enabled;
	void ChangeState(IConVar *var, const char *pOldValue, float flOldValue);
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
	void ToggleKillerFollowEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleSmoothCameraSwitchesEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleStateEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};