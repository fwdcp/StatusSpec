/*
 *  fovoverride.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include "convar.h"

#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class FOVOverride {
public:
	FOVOverride();

	void FrameHook(ClientFrameStage_t curStage);
	float GetFOVOverride();
private:
	bool HookGetFOV();

	int frameHook;
	int getFOVHook;

	ConVar *enabled;
	ConVar *fov;
	ConVar *zoomed;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern FOVOverride *g_FOVOverride;