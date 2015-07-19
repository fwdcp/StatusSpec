/*
 *  antifreeze.h
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

class AntiFreeze : public Module {
public:
	AntiFreeze();

	static bool CheckDependencies();
private:
	class Panel;
	Panel *panel;

	ConVar *enabled;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};