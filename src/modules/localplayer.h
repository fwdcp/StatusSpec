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
class IConVar;

#include "../modules.h"

class LocalPlayer : public Module {
public:
	LocalPlayer();

	static bool CheckDependencies();
private:
	int GetLocalPlayerIndexOverride();

	class Panel;
	Panel *panel;

	bool getLocalPlayerIndexDetoured;

	ConVar *enabled;
	ConVar *player;
	ConCommand *set_current_target;
	ConVar *track_spec_target;
	void SetToCurrentTarget();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleTrackSpecTarget(IConVar *var, const char *pOldValue, float flOldValue);
};