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

#define CLIENT_DLL
#include "cbase.h"
#include "c_baseentity.h"
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

	bool operator==(int entindex) const;
	bool operator==(IClientEntity *entity) const;
	bool operator==(const Player &player) const;

	bool operator!=(int entindex) const;
	bool operator!=(IClientEntity *entity) const;
	bool operator!=(const Player &player) const;

	bool operator<(int entindex) const;
	bool operator<(IClientEntity *entity) const;
	bool operator<(const Player &player) const;

	bool operator<=(int entindex) const;
	bool operator<=(IClientEntity *entity) const;
	bool operator<=(const Player &player) const;

	bool operator>(int entindex) const;
	bool operator>(IClientEntity *entity) const;
	bool operator>(const Player &player) const;

	bool operator>=(int entindex) const;
	bool operator>=(IClientEntity *entity) const;
	bool operator>=(const Player &player) const;

	operator bool() const;
	operator IClientEntity *() const;

	IClientEntity *operator->() const;

	IClientEntity *GetEntity() const;

	bool CheckCondition(TFCond condition) const;
	TFClassType GetClass() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	const char *GetName() const;
	int GetObserverMode() const;
	C_BaseEntity *GetObserverTarget() const;
	CSteamID GetSteamID() const;
	TFTeam GetTeam() const;
	int GetUserID() const;
	bool IsAlive() const;
private:
	CHandle<IClientEntity> playerEntity;

	bool IsValid() const;
	bool IsEqualTo(const Player &player) const;
	bool IsNotEqualTo(const Player &player) const;
	bool IsLessThan(const Player &player) const;
	bool IsLessThanOrEqualTo(const Player &player) const;
	bool IsGreaterThan(const Player &player) const;
	bool IsGreaterThanOrEqualTo(const Player &player) const;
};