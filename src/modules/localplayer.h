/*
 *  localplayer.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

class ConCommand;
class ConVar;

#include "cdll_int.h"

#include "../modules.h"

class LocalPlayer : public Module {
public:
	LocalPlayer(std::string name);

	static bool CheckDependencies(std::string name);
private:
	void FrameHook(ClientFrameStage_t curStage);
	int GetLocalPlayerIndexOverride();

	int frameHook;
	bool getLocalPlayerIndexDetoured;

	ConVar *enabled;
	ConVar *player;
	ConVar *track_spec_target;
	ConCommand *set_current_target;
	void SetToCurrentTarget();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleTrackSpecTarget(IConVar *var, const char *pOldValue, float flOldValue);
};