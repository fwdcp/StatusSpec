/*
*  killstreaks.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "killstreaks.h"

Killstreaks::Killstreaks() {
	enabled = new ConVar("statusspec_killstreaks_enabled", "0", FCVAR_NONE, "enable killstreaks display", Killstreaks::ToggleEnabled);
}

bool Killstreaks::IsEnabled() {
	return enabled->GetBool();
}

bool Killstreaks::FireEvent(IGameEvent *event) {
	if (strcmp(event->GetName(), "player_spawn") == 0) {
		int userID = event->GetInt("userid", -1);

		if (userID != -1) {
			currentKillstreaks.erase(userID);
		}
	}
	else if (strcmp(event->GetName(), "player_death") == 0) {
		int victimUserID = event->GetInt("userid", -1);
		int attackerUserID = event->GetInt("attacker", -1);
		int assisterUserID = event->GetInt("assister", -1);
		int weaponID = event->GetInt("weaponid");

		if (attackerUserID != -1) {
			if (attackerUserID != victimUserID) {
				currentKillstreaks[attackerUserID][weaponID]++;

				if (IsEnabled()) {
					int attackerKillstreak = 0;

					for (auto iterator = currentKillstreaks[victimUserID].begin(); iterator != currentKillstreaks[victimUserID].end(); ++iterator) {
						attackerKillstreak += iterator->second;
					}

					event->SetInt("kill_streak_total", GetCurrentKillstreak(attackerUserID));

					if (weaponID != TF_WEAPON_NONE) {
						event->SetInt("kill_streak_wep", currentKillstreaks[attackerUserID][weaponID]);
					}
					else {
						event->SetInt("kill_streak_wep", 0);
					}
				
				}
			}
			else {
				if (IsEnabled()) {
					event->SetInt("kill_streak_total", 0);
					event->SetInt("kill_streak_wep", 0);
				}
			}
		}

		if (assisterUserID != -1) {
			IClientEntity *assister = Interfaces::pClientEntityList->GetClientEntity(Interfaces::pEngineClient->GetPlayerForUserID(assisterUserID));

			if (assister && Entities::CheckClassBaseclass(assister->GetClientClass(), "DT_TFPlayer")) {
				for (int i = 0; i < MAX_WEAPONS; i++) {
					IClientEntity *weapon = Interfaces::pClientEntityList->GetClientEntity(ENTITY_INDEX_FROM_ENTITY_OFFSET(assister, Entities::pCTFPlayer__m_hMyWeapons[i]));

					if (!weapon || !Entities::CheckClassBaseclass(weapon->GetClientClass(), "DT_WeaponMedigun")) {
						continue;
					}

					bool healing = *MAKE_PTR(bool *, weapon, Entities::pCWeaponMedigun__m_bHealing);

					if (healing) {
						int healingTarget = ENTITY_INDEX_FROM_ENTITY_OFFSET(weapon, Entities::pCWeaponMedigun__m_hHealingTarget);

						if (healingTarget == Interfaces::pEngineClient->GetPlayerForUserID(attackerUserID)) {
							currentKillstreaks[assisterUserID][TF_WEAPON_MEDIGUN]++;
						}
					}
				}
			}

			if (IsEnabled()) {
				event->SetInt("kill_streak_assist", GetCurrentKillstreak(assisterUserID));
			}
		}

		if (victimUserID != -1) {
			if (IsEnabled()) {
				event->SetInt("kill_streak_victim", GetCurrentKillstreak(victimUserID));
			}
		}

		if (IsEnabled()) {
			return true;
		}
	}

	return false;
}

void Killstreaks::ProcessEntity(IClientEntity* entity) {
	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return;
	}

	if (IsEnabled()) {
		int *killstreakPlayer = MAKE_PTR(int *, entity, Entities::pCTFPlayer__m_iKillStreak);
		*killstreakPlayer = 0;

		C_PlayerResource *playerResource = dynamic_cast<C_PlayerResource *>(Interfaces::GetGameResources());

		if (playerResource) {
			player_info_t playerInfo;

			if (Interfaces::pEngineClient->GetPlayerInfo(entity->entindex(), &playerInfo)) {
				int *killstreakGlobal = MAKE_PTR(int *, playerResource, Entities::pCTFPlayerResource__m_iKillstreak[entity->entindex()]);
				*killstreakGlobal = 0;
			}
		}
	}
}

void Killstreaks::PostEntityUpdate() {
	if (!Interfaces::pEngineClient->IsInGame()) {
		currentKillstreaks.clear();
		return;
	}

	C_PlayerResource *playerResource = dynamic_cast<C_PlayerResource *>(Interfaces::GetGameResources());

	for (auto iterator = currentKillstreaks.begin(); iterator != currentKillstreaks.end(); ++iterator) {
		int player = Interfaces::pEngineClient->GetPlayerForUserID(iterator->first);
		IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);

		if (IsEnabled() && Interfaces::GetGameResources()->IsAlive(player)) {
			int currentKillstreak = GetCurrentKillstreak(iterator->first);

			int *killstreakPlayer = MAKE_PTR(int *, playerEntity, Entities::pCTFPlayer__m_iKillStreak);
			*killstreakPlayer = currentKillstreak;

			if (playerResource) {
				int *killstreakGlobal = MAKE_PTR(int *, playerResource, Entities::pCTFPlayerResource__m_iKillstreak[player]);
				*killstreakGlobal = currentKillstreak;
			}
		}
	}
}

int Killstreaks::GetCurrentKillstreak(int userid) {
	int killstreak = 0;

	for (auto iterator = currentKillstreaks[userid].begin(); iterator != currentKillstreaks[userid].end(); ++iterator) {
		killstreak += iterator->second;
	}

	return killstreak;
}

void Killstreaks::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!g_Killstreaks->IsEnabled()) {
		for (int i = 0; i <= MAX_PLAYERS; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity) {
				continue;
			}

			if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
				continue;
			}

			C_PlayerResource *playerResource = dynamic_cast<C_PlayerResource *>(Interfaces::GetGameResources());

			if (playerResource) {
				player_info_t playerInfo;

				if (Interfaces::pEngineClient->GetPlayerInfo(entity->entindex(), &playerInfo)) {
					int *killstreak = MAKE_PTR(int *, playerResource, Entities::pCTFPlayerResource__m_iKillstreak[entity->entindex()]);
					*killstreak = 0;
				}
			}
		}
	}
}