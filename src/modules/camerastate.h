/*
 *  camerastate.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

class ConVar;
class IConVar;

class CameraState : public Module {
public:
	CameraState();

	static bool CheckDependencies();
private:
	void UpdateState();

	class Panel;
	Panel *panel;

	bool currentlyUpdating;

	class HLTVCameraOverride;

	ConVar *change;
	ConVar *current;
	ConVar *enabled;
	void ChangeState(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};
