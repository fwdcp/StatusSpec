/*
*  player.h
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#pragma once

#include "stdafx.h"

#include <cstdint>

#include "icliententity.h"

#include "entities.h"
#include "enums.h"
#include "funcs.h"
#include "ifaces.h"

class Player {
public:
	static bool CheckPlayer(IClientEntity *entity);
	static TFTeam GetTeam(IClientEntity *entity);
	static bool CheckCondition(IClientEntity *entity, TFCond condition);
	static CSteamID GetSteamID(IClientEntity *entity);
};