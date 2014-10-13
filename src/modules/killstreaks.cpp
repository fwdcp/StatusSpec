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
	fireEventClientSideHook = 0;
	frameHook = 0;

	enabled = new ConVar("statusspec_killstreaks_enabled", "0", FCVAR_NONE, "enable killstreaks display", [](IConVar *var, const char *pOldValue, float flOldValue) { g_Killstreaks->ToggleEnabled(var, pOldValue, flOldValue); });
	total_killfeed = new ConVar("statusspec_killstreaks_total_killfeed", "0", FCVAR_NONE, "display total kills for player in killfeed instead of only kills with single weapon");
}

bool Killstreaks::FireEventClientSideOverride(IGameEvent *event) {
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

				event->SetInt("kill_streak_total", GetCurrentKillstreak(attackerUserID));

				if (total_killfeed->GetBool()) {
					event->SetInt("kill_streak_wep", GetCurrentKillstreak(attackerUserID));
				}
				else {
					if (weaponID != TF_WEAPON_NONE) {
						event->SetInt("kill_streak_wep", currentKillstreaks[attackerUserID][weaponID]);
					}
					else {
						event->SetInt("kill_streak_wep", 0);
					}
				}
			}
			else {
				event->SetInt("kill_streak_total", 0);
				event->SetInt("kill_streak_wep", 0);
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

			event->SetInt("kill_streak_assist", GetCurrentKillstreak(assisterUserID));
		}

		if (victimUserID != -1) {
			event->SetInt("kill_streak_victim", GetCurrentKillstreak(victimUserID));
		}
	}

	RETURN_META_VALUE(MRES_HANDLED, false);
}

void Killstreaks::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		if (!gameResourcesEntity.IsValid() || !gameResourcesEntity.Get()) {
			int maxEntity = Interfaces::pClientEntityList->GetMaxEntities();

			for (int i = 1; i <= maxEntity; i++) {
				IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

				if (!entity) {
					continue;
				}

				if (Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayerResource")) {
					gameResourcesEntity = dynamic_cast<C_BaseEntity *>(entity);

					break;
				}
			}
		}

		for (int i = 1; i <= MAX_PLAYERS; i++) {
			Player player = i;

			if (!player) {
				continue;
			}

			int *killstreakPlayer = MAKE_PTR(int *, player.GetEntity(), Entities::pCTFPlayer__m_iKillStreak);
			int *killstreakGlobal = MAKE_PTR(int *, gameResourcesEntity.Get(), Entities::pCTFPlayerResource__m_iKillstreak[player->entindex()]);

			int userid = player.GetUserID();

			if (currentKillstreaks.find(userid) != currentKillstreaks.end()) {
				*killstreakPlayer = GetCurrentKillstreak(userid);

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					*killstreakGlobal = GetCurrentKillstreak(userid);
				}
			}
			else {
				*killstreakPlayer = 0;

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					*killstreakGlobal = 0;
				}
			}
		}
	}
	else if (curStage == FRAME_START) {
		if (!Interfaces::pEngineClient->IsInGame()) {
			currentKillstreaks.clear();
		}
	}

	RETURN_META(MRES_IGNORED);
}

int Killstreaks::GetCurrentKillstreak(int userid) {
	int killstreak = 0;

	for (auto iterator = currentKillstreaks[userid].begin(); iterator != currentKillstreaks[userid].end(); ++iterator) {
		killstreak += iterator->second;
	}

	return killstreak;
}

void Killstreaks::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!fireEventClientSideHook) {
			fireEventClientSideHook = Funcs::AddHook_IGameEventManager2_FireEventClientSide(Interfaces::pGameEventManager, SH_MEMBER(this, &Killstreaks::FireEventClientSideOverride), false);
		}

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &Killstreaks::FrameHook), true);
		}
	}
	else {
		if (fireEventClientSideHook) {
			if (Funcs::RemoveHook(fireEventClientSideHook)) {
				fireEventClientSideHook = 0;
			}
		}

		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

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