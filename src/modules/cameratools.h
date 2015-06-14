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
	void UpdateState();

	bool currentlyUpdating;
	int frameHook;
	int setModeHook;
	int setPrimaryTargetHook;
	KeyValues *specguiSettings;

	void FrameHook(ClientFrameStage_t curStage);
	void SetModeOverride(C_HLTVCamera *hltvcamera, int &iMode);
	void SetPrimaryTargetOverride(C_HLTVCamera *hltvcamera, int &nEntity);

	class HLTVCameraOverride;

	ConVar *force_mode;
	ConVar *force_target;
	ConVar *force_valid_target;
	ConVar *killer_follow_enabled;
	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	ConVar *state;
	ConVar *state_enabled;
	void ChangeForceMode(IConVar *var, const char *pOldValue, float flOldValue);
	void ChangeForceTarget(IConVar *var, const char *pOldValue, float flOldValue);
	void ChangeState(IConVar *var, const char *pOldValue, float flOldValue);
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
	void ToggleForceValidTarget(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleKillerFollowEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleStateEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};