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

#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"

class LocalPlayer {
public:
	LocalPlayer();

	bool IsEnabled();

	int GetLocalPlayerIndexOverride();
private:
	ConVar *enabled;
	ConVar *player;
	ConCommand *set_current_target;

	static void SetToCurrentTarget();
};

extern LocalPlayer *g_LocalPlayer;