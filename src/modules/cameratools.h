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
	void FrameHook(ClientFrameStage_t curStage);
private:
	int calcViewHook;
	bool currentlyUpdating;
	int frameHook;
	QAngle smoothCurrentAngles;
	Vector smoothCurrentPosition;
	int smoothEndMode;
	int smoothEndTarget;
	float smoothEndTime;
	bool smoothInProgress;
	QAngle smoothStartAngles;
	Vector smoothStartPosition;
	float smoothStartTime;
	KeyValues *specguiSettings;

	void CalcViewOverride(C_HLTVCamera *instance, Vector &origin, QAngle &angles, float &fov);
	void UpdateState();

	class HLTVCameraOverride;

	ConVar *killer_follow_enabled;
	ConVar *smooth_camera_switches_enabled;
	ConVar *smooth_camera_switches_max_distance;
	ConVar *smooth_camera_switches_time;
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