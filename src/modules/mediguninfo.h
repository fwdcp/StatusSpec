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

class ConCommand;
class ConVar;

#include "../modules.h"

class MedigunInfo : public Module {
public:
	MedigunInfo(std::string name);

	static bool CheckDependencies(std::string name);
private:
	class MainPanel;
	MainPanel *mainPanel;
	class MedigunPanel;
	
	ConVar *enabled;
	ConCommand *reload_settings;
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};