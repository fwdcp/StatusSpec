/*
 *  freezeinfo.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

class ConCommand;
class ConVar;
class IConVar;

class FreezeInfo : public Module {
public:
	FreezeInfo();

	static bool CheckDependencies();
private:
	class Panel;
	Panel *panel;

	ConVar *enabled;
	ConCommand *reload_settings;
	ConVar *threshold;
	void ChangeThreshold(IConVar *var, const char *pOldValue, float flOldValue);
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};