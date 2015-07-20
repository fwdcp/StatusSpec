/*
 *  killstreaks.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <map>
#include <string>

class C_BaseEntity;
class ConVar;
class IConVar;

#include "ehandle.h"

#include "../modules.h"

class Killstreaks : public Module {
public:
	Killstreaks();

	static bool CheckDependencies();
private:
	class Panel;
	Panel *panel;

	ConVar *enabled;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};