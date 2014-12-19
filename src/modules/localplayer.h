/*
 *  localplayer.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"

class LocalPlayer : public Module {
public:
	LocalPlayer(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
	int GetLocalPlayerIndexOverride();
private:
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