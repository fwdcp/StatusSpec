/*
 *  cameraautoswitch.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "igameevents.h"

#include "../modules.h"

class CCommand;
class ConCommand;
class ConVar;
class IConVar;

 class CameraAutoSwitch : public Module, IGameEventListener2 {
 public:
	 CameraAutoSwitch();

 	static bool CheckDependencies();

 	virtual void FireGameEvent(IGameEvent *event);
 private:
	 class Panel;
	 Panel *panel;

	 ConVar *enabled;
	 ConVar *killer;
	 ConVar *killer_delay;
	 void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	 void ToggleKillerEnabled(IConVar *var, const char *pOldValue, float flOldValue);
 };
