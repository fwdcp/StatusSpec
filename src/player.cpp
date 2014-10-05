/*
*  player.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "player.h"

Player::Player(int entindex) {
	playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);
}

Player::Player(IClientEntity* entity) {
	playerEntity = entity;
}

Player& Player::operator=(int entindex) {
	playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);

	return *this;
}

Player& Player::operator=(IClientEntity* entity) {
	playerEntity = entity;

	return *this;
}

Player& Player::operator=(const Player &player) {
	if (this == &player) {
		return *this;
	}

	playerEntity = player.playerEntity;

	return *this;
}

bool Player::operator==(int entindex) {
	if (playerEntity.IsValid()) {
		return playerEntity->entindex() == entindex;
	}

	return false;
}

bool Player::operator==(IClientEntity *entity) {
	return playerEntity == entity;
}

bool Player::operator==(const Player &player) {
	return playerEntity == player.playerEntity;
}

bool Player::operator!=(int entindex) {
	return !(*this == entindex);
}

bool Player::operator!=(IClientEntity *entity) {
	return !(*this == entity);
}

bool Player::operator!=(const Player &player) {
	return !(playerEntity == player.playerEntity);
}

bool Player::operator<(const Player &player) const {
	if (!player) {
		return false;
	}

	if (GetTeam() < player.GetTeam()) {
		return true;
	}

	static std::array<TFClassType, 10> classes = { TFClass_Unknown, TFClass_Scout, TFClass_Soldier, TFClass_Pyro, TFClass_DemoMan, TFClass_Heavy, TFClass_Engineer, TFClass_Medic, TFClass_Sniper, TFClass_Spy };

	int firstClass = std::distance(classes.begin(), std::find(classes.begin(), classes.end(), GetClass()));
	int secondClass = std::distance(classes.begin(), std::find(classes.begin(), classes.end(), player.GetClass()));

	if (firstClass < secondClass) {
		return true;
	}

	return false;
}

Player::operator bool() const {
	try {
		return playerEntity.IsValid() && Entities::CheckClassBaseclass(playerEntity->GetClientClass(), "DT_TFPlayer") && Interfaces::GetGameResources()->IsConnected(playerEntity->entindex());
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}

	return false;
}

Player::operator IClientEntity *() const {
	return playerEntity;
}

IClientEntity *Player::operator->() const {
	return playerEntity;
}

IClientEntity *Player::GetEntity() {
	return playerEntity;
}

bool Player::CheckCondition(TFCond condition) const {
	if (playerEntity.IsValid()) {
		uint32_t playerCond = *MAKE_PTR(uint32_t*, playerEntity.Get(), Entities::pCTFPlayer__m_nPlayerCond);
		uint32_t condBits = *MAKE_PTR(uint32_t*, playerEntity.Get(), Entities::pCTFPlayer___condition_bits);
		uint32_t playerCondEx = *MAKE_PTR(uint32_t*, playerEntity.Get(), Entities::pCTFPlayer__m_nPlayerCondEx);
		uint32_t playerCondEx2 = *MAKE_PTR(uint32_t*, playerEntity.Get(), Entities::pCTFPlayer__m_nPlayerCondEx2);

		uint32_t conditions[3];
		conditions[0] = playerCond | condBits;
		conditions[1] = playerCondEx;
		conditions[2] = playerCondEx2;

		if (condition < 32) {
			if (conditions[0] & (1 << condition)) {
				return true;
			}
		}
		else if (condition < 64) {
			if (conditions[1] & (1 << (condition - 32))) {
				return true;
			}
		}
		else if (condition < 96) {
			if (conditions[2] & (1 << (condition - 64))) {
				return true;
			}
		}
	}

	return false;
}

TFClassType Player::GetClass() const {
	if (playerEntity.IsValid()) {
		return (TFClassType)*MAKE_PTR(int*, playerEntity.Get(), Entities::pCTFPlayer__m_iClass);
	}

	return TFClass_Unknown;
}

int Player::GetHealth() const {
	try {
		if (playerEntity.IsValid()) {
			return *MAKE_PTR(int*, Interfaces::GetGameResources(), Entities::pCTFPlayerResource__m_iHealth[playerEntity->entindex()]);
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());

		if (playerEntity.IsValid()) {
			return *MAKE_PTR(int*, playerEntity.Get(), Entities::pCTFPlayer__m_iHealth);
		}
	}

	return 0;
}

int Player::GetMaxHealth() const {
	try {
		if (playerEntity.IsValid()) {
			return *MAKE_PTR(int*, Interfaces::GetGameResources(), Entities::pCTFPlayerResource__m_iMaxHealth[playerEntity->entindex()]);
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}

	return 0;
}

const char *Player::GetName() const {
	try {
		if (playerEntity.IsValid()) {
			return Interfaces::GetGameResources()->GetPlayerName(playerEntity->entindex());
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}
	
	return "";
}

CSteamID Player::GetSteamID() const {
	if (playerEntity.IsValid()) {
		player_info_t playerInfo;

		if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, playerEntity->entindex(), &playerInfo)) {
			if (playerInfo.friendsID) {
				static EUniverse universe = k_EUniverseInvalid;

				if (universe == k_EUniverseInvalid) {
					universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
				}

				return CSteamID(playerInfo.friendsID, 1, universe, k_EAccountTypeIndividual);
			}
		}
	}

	return CSteamID();
}

TFTeam Player::GetTeam() const {
	try {
		if (playerEntity.IsValid()) {
			return (TFTeam)Interfaces::GetGameResources()->GetTeam(playerEntity->entindex());
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());

		if (playerEntity.IsValid()) {
			return (TFTeam)*MAKE_PTR(int*, playerEntity.Get(), Entities::pCTFPlayer__m_iTeamNum);
		}
	}

	return TFTeam_Unassigned;
}

bool Player::IsAlive() const {
	try {
		if (playerEntity.IsValid()) {
			return Interfaces::GetGameResources()->IsAlive(playerEntity->entindex());
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}

	return false;
}