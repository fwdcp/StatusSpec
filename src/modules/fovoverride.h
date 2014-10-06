/*
*  antifreeze.h
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

	bool IsEnabled();

	float GetFOVOverride(C_TFPlayer *instance);
private:
	ConVar *enabled;
	ConVar *fov;
	ConVar *zoomed;
};

extern FOVOverride *g_FOVOverride;