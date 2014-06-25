/*
*  killstreaks.h
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#pragma once

#include "../stdafx.h"

#include <map>
#include <string>

#include "../entities.h"
#include "../enums.h"
#include "../hooks.h"
#include "../ifaces.h"

class Killstreaks {
public:
	Killstreaks();

	bool IsEnabled();

	bool FireEvent(IGameEvent *event);

	void ProcessEntity(IClientEntity* entity);
	void PostEntityUpdate();
private:
	std::map<int, std::map<int, int>> currentKillstreaks;
	int GetCurrentKillstreak(int userid);

	ConVar *enabled;
	static void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern Killstreaks *g_Killstreaks;