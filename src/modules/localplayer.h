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

#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class LocalPlayer {
public:
	LocalPlayer();

	bool IsEnabled();

	void PostEntityUpdate();

	int GetLocalPlayerIndexOverride();
private:
	ConVar *enabled;
	ConVar *player;
	ConVar *track_spec_target;
	ConCommand *set_current_target;
	void SetToCurrentTarget();
};

extern LocalPlayer *g_LocalPlayer;