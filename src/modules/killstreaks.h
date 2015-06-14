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

#include <map>
#include <string>

class C_BaseEntity;
class ConVar;
class IGameEvent;

#include "cdll_int.h"
#include "ehandle.h"

#include "../modules.h"

class Killstreaks : public Module {
public:
	Killstreaks(std::string name);

	static bool CheckDependencies(std::string name);
private:
	bool FireEventClientSideOverride(IGameEvent *event);
	void FrameHook(ClientFrameStage_t curStage);

	int bluTopKillstreak;
	int bluTopKillstreakPlayer;
	std::map<int, int> currentKillstreaks;
	int fireEventClientSideHook;
	int frameHook;
	CHandle<C_BaseEntity> gameResourcesEntity;
	int redTopKillstreak;
	int redTopKillstreakPlayer;

	ConVar *enabled;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};