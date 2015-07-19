/*
 *  mediguninfo.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <string>

#include "../modules.h"

class ConCommand;
class ConVar;
class IConVar;

class MedigunInfo : public Module {
public:
	MedigunInfo();

	static bool CheckDependencies();
private:
	class MainPanel;
	MainPanel *mainPanel;
	class MedigunPanel;
	
	ConVar *enabled;
	ConCommand *reload_settings;
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};