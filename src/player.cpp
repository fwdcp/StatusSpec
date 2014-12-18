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

	int firstClass = std::distance(TFDefinitions::defaultClassOrder.begin(), std::find(TFDefinitions::defaultClassOrder.begin(), TFDefinitions::defaultClassOrder.end(), GetClass()));
	int secondClass = std::distance(TFDefinitions::defaultClassOrder.begin(), std::find(TFDefinitions::defaultClassOrder.begin(), TFDefinitions::defaultClassOrder.end(), player.GetClass()));

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

	int firstClass = std::distance(TFDefinitions::defaultClassOrder.begin(), std::find(TFDefinitions::defaultClassOrder.begin(), TFDefinitions::defaultClassOrder.end(), GetClass()));
	int secondClass = std::distance(TFDefinitions::defaultClassOrder.begin(), std::find(TFDefinitions::defaultClassOrder.begin(), TFDefinitions::defaultClassOrder.end(), player.GetClass()));

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
		return playerEntity.IsValid() && playerEntity.Get() && playerEntity->entindex() >= 1 && playerEntity->entindex() <= MAX_PLAYERS && Entities::CheckEntityBaseclass(playerEntity, "TFPlayer");
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
		uint32_t playerCond = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCond" });
		uint32_t condBits = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "_condition_bits" });
		uint32_t playerCondEx = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCondEx" });
		uint32_t playerCondEx2 = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCondEx2" });

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
		return (TFClassType)*Entities::GetEntityProp<int *>(playerEntity.Get(), { "m_iClass" });
	}

	return TFClass_Unknown;
}

int Player::GetHealth() const {
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity *>(playerEntity.Get())->GetHealth();
	}

	return 0;
}

int Player::GetMaxHealth() const {
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity *>(playerEntity.Get())->GetMaxHealth();
	}

	return 0;
}

const char *Player::GetName() const {
	if (IsValid()) {
		player_info_t playerInfo;

		if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, playerEntity->entindex(), &playerInfo)) {
			return playerInfo.name;
		}
	}
	
	return "";
}

int Player::GetObserverMode() const {
	if (IsValid()) {
		return dynamic_cast<C_BasePlayer *>(playerEntity.Get())->GetObserverMode();
	}

	return OBS_MODE_NONE;
}

C_BaseEntity *Player::GetObserverTarget() const {
	if (IsValid()) {
		return dynamic_cast<C_BasePlayer *>(playerEntity.Get())->GetObserverTarget();
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
	if (IsValid()) {
		TFTeam team = (TFTeam)dynamic_cast<C_BaseEntity *>(playerEntity.Get())->GetTeamNumber();

		return team;
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
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity *>(playerEntity.Get())->IsAlive();
	}

	return false;
}