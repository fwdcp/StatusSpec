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
#include <iomanip>
#include <string>

#include "../entities.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

class Killstreaks : public Module {
public:
	Killstreaks(std::string name);

	static bool CheckDependencies(std::string name);

	bool FireEventClientSideOverride(IGameEvent *event);
	void FrameHook(ClientFrameStage_t curStage);
private:
	int bluTopKillstreak;
	int bluTopKillstreakPlayer;
	std::map<int, std::map<std::string, int>> currentKillstreaks;
	int fireEventClientSideHook;
	int frameHook;
	EHANDLE gameResourcesEntity;
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