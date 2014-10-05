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

#include <algorithm>
#include <array>
#include <cstdint>

#include "icliententity.h"

#include "entities.h"
#include "enums.h"
#include "funcs.h"
#include "ifaces.h"

class Player {
public:
	Player(int entindex);
	Player(IClientEntity *entity);
	Player() {};

	Player& operator=(int entindex);
	Player& operator=(IClientEntity *entity);
	Player& operator=(const Player &player);

	bool operator==(int entindex);
	bool operator==(IClientEntity *entity);
	bool operator==(const Player &player);

	bool operator!=(int entindex);
	bool operator!=(IClientEntity *entity);
	bool operator!=(const Player &player);

	bool operator<(const Player &player) const;

	operator bool() const;
	operator IClientEntity *() const;

	IClientEntity *operator->() const;

	IClientEntity *GetEntity();

	bool CheckCondition(TFCond condition) const;
	TFClassType GetClass() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	const char *GetName() const;
	CSteamID GetSteamID() const;
	TFTeam GetTeam() const;
	bool IsAlive() const;
private:
	CHandle<IClientEntity> playerEntity;
};