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
	std::map<int, std::map<std::string, int>> currentKillstreaks;
	int fireEventClientSideHook;
	int frameHook;
	CHandle<C_BaseEntity> gameResourcesEntity;
	int redTopKillstreak;
	int redTopKillstreakPlayer;

	int GetCurrentSlotKillstreak(int userid, int slot);
	int GetCurrentPlayerKillstreak(int userid);
	int GetKillTypeSlot(std::string killType);
	bool IsAttributableKill(std::string killType);

	ConVar *enabled;
	ConVar *total_killfeed;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};