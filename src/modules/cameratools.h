/*
 *  cameratools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

class CCommand;
class C_HLTVCamera;
class ConCommand;
class ConVar;
class KeyValues;
class IConVar;

class CameraTools : public Module {
public:
	CameraTools();

	static bool CheckDependencies();
private:
	int setModeHook;
	int setPrimaryTargetHook;
	KeyValues *specguiSettings;

	void SetModeOverride(C_HLTVCamera *hltvcamera, int &iMode);
	void SetPrimaryTargetOverride(C_HLTVCamera *hltvcamera, int &nEntity);

	class HLTVCameraOverride;

	ConVar *force_mode;
	ConVar *force_target;
	ConVar *force_valid_target;
	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	void ChangeForceMode(IConVar *var, const char *pOldValue, float flOldValue);
	void ChangeForceTarget(IConVar *var, const char *pOldValue, float flOldValue);
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
	void ToggleForceValidTarget(IConVar *var, const char *pOldValue, float flOldValue);
};