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
			Player assister = Interfaces::pEngineClient->GetPlayerForUserID(assisterUserID);

			if (assister) {
				for (int i = 0; i < MAX_WEAPONS; i++) {
					IClientEntity *weapon = Interfaces::pClientEntityList->GetClientEntity(ENTITY_INDEX_FROM_ENTITY_OFFSET(assister.GetEntity(), Entities::pCTFPlayer__m_hMyWeapons[i]));

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
	try {
		Player player = entity;

		if (!player) {
			return;
		}

		if (IsEnabled()) {
			int *killstreakPlayer = MAKE_PTR(int *, player.GetEntity(), Entities::pCTFPlayer__m_iKillStreak);
			*killstreakPlayer = 0;

			int *killstreakGlobal = MAKE_PTR(int *, Interfaces::GetGameResources(), Entities::pCTFPlayerResource__m_iKillstreak[player->entindex()]);
			*killstreakGlobal = 0;
		}
	}
	catch (bad_pointer &e) {
		Warning(e.what());
	}
}

void Killstreaks::PostEntityUpdate() {
	if (!Interfaces::pEngineClient->IsInGame()) {
		currentKillstreaks.clear();
		return;
	}

	for (auto iterator = currentKillstreaks.begin(); iterator != currentKillstreaks.end(); ++iterator) {
		try {
			Player player = Interfaces::pEngineClient->GetPlayerForUserID(iterator->first);

			if (IsEnabled() && player.IsAlive()) {
				int currentKillstreak = GetCurrentKillstreak(iterator->first);

				int *killstreakPlayer = MAKE_PTR(int *, player.GetEntity(), Entities::pCTFPlayer__m_iKillStreak);
				*killstreakPlayer = currentKillstreak;

				int *killstreakGlobal = MAKE_PTR(int *, Interfaces::GetGameResources(), Entities::pCTFPlayerResource__m_iKillstreak[player]);
				*killstreakGlobal = currentKillstreak;
			}
		}
		catch (bad_pointer &e) {
			Warning(e.what());
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
			try {
				Player player = i;

				if (!player) {
					continue;
				}

				int *killstreak = MAKE_PTR(int *, Interfaces::GetGameResources(), Entities::pCTFPlayerResource__m_iKillstreak[player->entindex()]);
				*killstreak = 0;
			}
			catch (bad_pointer &e) {
				Warning(e.what());
			}
		}
	}
}