/*
 *  fovoverride.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "mathlib/vector.h"

#include "../modules.h"

class ConVar;
class IConVar;

class FOVOverride : public Module {
public:
	FOVOverride();

	static bool CheckDependencies();
private:
	int inToolModeHook;
	int setupEngineViewHook;
	bool InToolModeOverride();
	bool SetupEngineViewOverride(Vector &origin, QAngle &angles, float &fov);

	class HLTVCameraOverride;

	ConVar *enabled;
	ConVar *fov;
	ConVar *zoomed;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};