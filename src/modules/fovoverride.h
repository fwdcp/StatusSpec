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

#include "../modules.h"

#include "cdll_int.h"

class ConVar;
class IConVar;

class FOVOverride : public Module {
public:
	FOVOverride(std::string name);

	static bool CheckDependencies(std::string name);

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