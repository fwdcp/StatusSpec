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

Player::Player(IClientEntity *entity) {
	playerEntity = entity;
}

Player& Player::operator=(int entindex) {
	playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);

	return *this;
}

Player& Player::operator=(IClientEntity *entity) {
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

bool Player::operator==(int entindex) const {
	return IsEqualTo(Player(entindex));
}

bool Player::operator==(IClientEntity *entity) const {
	return IsEqualTo(Player(entity));
}

bool Player::operator==(const Player &player) const {
	return IsEqualTo(player);
}

bool Player::operator!=(int entindex) const {
	return IsNotEqualTo(Player(entindex));
}

bool Player::operator!=(IClientEntity *entity) const {
	return IsNotEqualTo(Player(entity));
}

bool Player::operator!=(const Player &player) const {
	return IsNotEqualTo(player);
}

bool Player::operator<(int entindex) const {
	return IsLessThan(Player(entindex));
}

bool Player::operator<(IClientEntity *entity) const {
	return IsLessThan(Player(entity));
}

bool Player::operator<(const Player &player) const {
	return IsLessThan(player);
}

bool Player::operator<=(int entindex) const {
	return IsLessThanOrEqualTo(Player(entindex));
}

bool Player::operator<=(IClientEntity *entity) const {
	return IsLessThanOrEqualTo(Player(entity));
}

bool Player::operator<=(const Player &player) const {
	return IsLessThanOrEqualTo(player);
}

bool Player::operator>(int entindex) const {
	return IsGreaterThan(Player(entindex));
}

bool Player::operator>(IClientEntity *entity) const {
	return IsGreaterThan(Player(entity));
}

bool Player::operator>(const Player &player) const {
	return IsGreaterThan(player);
}

bool Player::operator>=(int entindex) const {
	return IsGreaterThanOrEqualTo(Player(entindex));
}

bool Player::operator>=(IClientEntity *entity) const {
	return IsGreaterThanOrEqualTo(Player(entity));
}

bool Player::operator>=(const Player &player) const {
	return IsGreaterThanOrEqualTo(player);
}

bool Player::IsEqualTo(const Player &player) const {
	if (IsValid() && player.IsValid()) {
		return playerEntity == player.playerEntity;
	}

	return false;
}

bool Player::IsNotEqualTo(const Player &player) const {
	return !IsEqualTo(player);
}

bool Player::IsLessThan(const Player &player) const {
	if (!IsValid()) {
		return true;
	}

	if (!player.IsValid()) {
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

	if (this->GetEntity()->entindex() < player.GetEntity()->entindex()) {
		return true;
	}

	return false;
}

bool Player::IsLessThanOrEqualTo(const Player &player) const {
	return IsEqualTo(player) || IsLessThan(player);
}

bool Player::IsGreaterThan(const Player &player) const {
	if (!IsValid()) {
		return false;
	}

	if (!player.IsValid()) {
		return true;
	}

	if (GetTeam() > player.GetTeam()) {
		return false;
	}

	static std::array<TFClassType, 10> classes = { TFClass_Unknown, TFClass_Scout, TFClass_Soldier, TFClass_Pyro, TFClass_DemoMan, TFClass_Heavy, TFClass_Engineer, TFClass_Medic, TFClass_Sniper, TFClass_Spy };

	int firstClass = std::distance(classes.begin(), std::find(classes.begin(), classes.end(), GetClass()));
	int secondClass = std::distance(classes.begin(), std::find(classes.begin(), classes.end(), player.GetClass()));

	if (firstClass > secondClass) {
		return true;
	}

	if (this->GetEntity()->entindex() > player.GetEntity()->entindex()) {
		return true;
	}

	return false;
}

bool Player::IsGreaterThanOrEqualTo(const Player &player) const {
	return IsEqualTo(player) || IsGreaterThan(player);
}

Player::operator bool() const {
	return IsValid();
}

bool Player::IsValid() const {
	try {
		return playerEntity.IsValid() && playerEntity.Get() && playerEntity->entindex() >= 1 && playerEntity->entindex() <= MAX_PLAYERS && Entities::CheckClassBaseclass(playerEntity->GetClientClass(), "DT_TFPlayer") && Interfaces::GetGameResources()->IsConnected(playerEntity->entindex());
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

IClientEntity *Player::GetEntity() const {
	return playerEntity;
}

bool Player::CheckCondition(TFCond condition) const {
	if (IsValid()) {
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
	if (IsValid()) {
		return (TFClassType)*MAKE_PTR(int*, playerEntity.Get(), Entities::pCTFPlayer__m_iClass);
	}

	return TFClass_Unknown;
}

int Player::GetHealth() const {
	if (IsValid()) {
		return Funcs::CallFunc_C_TFPlayer_GetHealth((C_TFPlayer *)playerEntity.Get());
	}

	return 0;
}

int Player::GetMaxHealth() const {
	if (IsValid()) {
		return Funcs::CallFunc_C_TFPlayer_GetMaxHealth((C_TFPlayer *)playerEntity.Get());
	}

	return 0;
}

const char *Player::GetName() const {
	try {
		if (IsValid()) {
			return Interfaces::GetGameResources()->GetPlayerName(playerEntity->entindex());
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}
	
	return "";
}

int Player::GetObserverMode() const {
	if (IsValid()) {
		return Funcs::CallFunc_C_TFPlayer_GetObserverMode((C_TFPlayer *)playerEntity.Get());
	}

	return OBS_MODE_NONE;
}

C_BaseEntity *Player::GetObserverTarget() const {
	if (IsValid()) {
		return Funcs::CallFunc_C_TFPlayer_GetObserverTarget((C_TFPlayer *)playerEntity.Get());
	}

	return playerEntity->GetBaseEntity();
}

CSteamID Player::GetSteamID() const {
	if (IsValid()) {
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
		if (IsValid()) {
			int entindex = playerEntity->entindex();
			IGameResources *gameResources = Interfaces::GetGameResources();
			TFTeam team = (TFTeam)gameResources->GetTeam(entindex);

			return team;
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());

		if (IsValid()) {
			return (TFTeam)*MAKE_PTR(int*, playerEntity.Get(), Entities::pCTFPlayer__m_iTeamNum);
		}
	}

	return TFTeam_Unassigned;
}

int Player::GetUserID() const {
	if (IsValid()) {
		player_info_t playerInfo;

		if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, playerEntity->entindex(), &playerInfo)) {
			return playerInfo.userID;
		}
	}

	return 0;
}

bool Player::IsAlive() const {
	try {
		if (IsValid()) {
			return Interfaces::GetGameResources()->IsAlive(playerEntity->entindex());
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}

	return false;
}