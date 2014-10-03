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

bool Player::CheckPlayer(IClientEntity *entity) {
	try {
		return entity && Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer") && Interfaces::GetGameResources()->IsConnected(entity->entindex());
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}

	return false;
}

TFClassType Player::GetClass(IClientEntity *entity) {
	return (TFClassType)*MAKE_PTR(int*, entity, Entities::pCTFPlayer__m_iClass);
}

TFTeam Player::GetTeam(IClientEntity *entity) {
	return (TFTeam)*MAKE_PTR(int*, entity, Entities::pCTFPlayer__m_iTeamNum);
}

bool Player::CheckCondition(IClientEntity *entity, TFCond condition) {
	uint32_t playerCond = *MAKE_PTR(uint32_t*, entity, Entities::pCTFPlayer__m_nPlayerCond);
	uint32_t condBits = *MAKE_PTR(uint32_t*, entity, Entities::pCTFPlayer___condition_bits);
	uint32_t playerCondEx = *MAKE_PTR(uint32_t*, entity, Entities::pCTFPlayer__m_nPlayerCondEx);
	uint32_t playerCondEx2 = *MAKE_PTR(uint32_t*, entity, Entities::pCTFPlayer__m_nPlayerCondEx2);

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

	return false;
}

CSteamID Player::GetSteamID(IClientEntity *entity) {
	player_info_t playerInfo;

	if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, entity->entindex(), &playerInfo)) {
		if (playerInfo.friendsID) {
			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(playerInfo.friendsID, 1, universe, k_EAccountTypeIndividual);
		}
	}

	return CSteamID();
}