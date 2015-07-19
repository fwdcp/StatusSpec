/*
 *  killstreaks.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "killstreaks.h"

#include <iomanip>

#include "cbase.h"
#include "c_baseentity.h"
#include "convar.h"
#include "igameevents.h"
#include "shareddefs.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

Killstreaks::Killstreaks() {
	bluTopKillstreak = 0;
	bluTopKillstreakPlayer = 0;
	fireEventClientSideHook = 0;
	frameHook = 0;
	redTopKillstreak = 0;
	redTopKillstreakPlayer = 0;

	enabled = new ConVar("statusspec_killstreaks_enabled", "0", FCVAR_NONE, "enable killstreaks display", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<Killstreaks>()->ToggleEnabled(var, pOldValue, flOldValue); });
}

bool Killstreaks::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	if (!Interfaces::pGameEventManager) {
		PRINT_TAG();
		Warning("Required interface IGameEventManager2 for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	for (int i = 0; i < MAX_WEAPONS; i++) {
		std::stringstream ss;
		std::string arrayIndex;
		ss << std::setfill('0') << std::setw(3) << i;
		ss >> arrayIndex;

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_hMyWeapons", arrayIndex })) {
			PRINT_TAG();
			Warning("Required property table m_hMyWeapons for CTFPlayer for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

			ready = false;

			break;
		}
	}

	for (int i = 0; i < 3; i++) {
		std::stringstream ss;
		std::string arrayIndex;
		ss << std::setfill('0') << std::setw(3) << i;
		ss >> arrayIndex;

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nStreaks", arrayIndex })) {
			PRINT_TAG();
			Warning("Required property table m_nStreaks for CTFPlayer for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

			ready = false;

			break;
		}
	}

	for (int i = 0; i <= MAX_PLAYERS; i++) {
		std::stringstream ss;
		std::string arrayIndex;
		ss << std::setfill('0') << std::setw(3) << i;
		ss >> arrayIndex;

		if (!Entities::RetrieveClassPropOffset("CTFPlayerResource", { "m_iStreaks", arrayIndex })) {
			PRINT_TAG();
			Warning("Required property table m_iStreaks for CTFPlayerResource for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

			ready = false;

			break;
		}
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_bHealing" })) {
		PRINT_TAG();
		Warning("Required property m_bHealing for CWeaponMedigun for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_hHealingTarget" })) {
		PRINT_TAG();
		Warning("Required property m_hHealingTarget for CWeaponMedigun for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

		ready = false;
	}

	return ready;
}

bool Killstreaks::FireEventClientSideOverride(IGameEvent *event) {
	if (strcmp(event->GetName(), "player_spawn") == 0) {
		int userID = event->GetInt("userid", -1);

		if (userID != -1) {
			currentKillstreaks.erase(userID);
		}

		RETURN_META_VALUE(MRES_HANDLED, false);
	}
	else if (strcmp(event->GetName(), "player_death") == 0) {
		int victimUserID = event->GetInt("userid", -1);
		int attackerUserID = event->GetInt("attacker", -1);
		int assisterUserID = event->GetInt("assister", -1);
		std::string weapon = event->GetString("weapon");

		if (attackerUserID != -1) {
			if (attackerUserID != victimUserID) {
				currentKillstreaks[attackerUserID]++;

				event->SetInt("kill_streak_total", currentKillstreaks[attackerUserID]);
				event->SetInt("kill_streak_wep", currentKillstreaks[attackerUserID]);

				Player attacker = Interfaces::pEngineClient->GetPlayerForUserID(attackerUserID);

				if (attacker) {
					if (attacker.GetTeam() == TFTeam_Red) {
						if (currentKillstreaks[attackerUserID] > redTopKillstreak) {
							redTopKillstreak = currentKillstreaks[attackerUserID];
							redTopKillstreakPlayer = attackerUserID;
						}
					}
					else if (attacker.GetTeam() == TFTeam_Blue) {
						if (currentKillstreaks[attackerUserID] > bluTopKillstreak) {
							bluTopKillstreak = currentKillstreaks[attackerUserID];
							bluTopKillstreakPlayer = attackerUserID;
						}
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
					std::stringstream ss;
					std::string arrayIndex;
					ss << std::setfill('0') << std::setw(3) << i;
					ss >> arrayIndex;

					IClientEntity *weapon = Entities::GetEntityProp<EHANDLE *>(assister.GetEntity(), { "m_hMyWeapons", arrayIndex })->Get();

					if (!weapon || !Entities::CheckEntityBaseclass(weapon, "WeaponMedigun")) {
						continue;
					}

					bool healing = *Entities::GetEntityProp<bool *>(weapon, { "m_bHealing" });

					if (healing) {
						int healingTarget = Entities::GetEntityProp<EHANDLE *>(weapon, { "m_hHealingTarget" })->GetEntryIndex();

						if (healingTarget == Interfaces::pEngineClient->GetPlayerForUserID(attackerUserID)) {
							currentKillstreaks[assisterUserID]++;

							if (assister.GetTeam() == TFTeam_Red) {
								if (currentKillstreaks[assisterUserID] > redTopKillstreak) {
									redTopKillstreak = currentKillstreaks[assisterUserID];
									redTopKillstreakPlayer = assisterUserID;
								}
							}
							else if (assister.GetTeam() == TFTeam_Blue) {
								if (currentKillstreaks[assisterUserID] > bluTopKillstreak) {
									bluTopKillstreak = currentKillstreaks[assisterUserID];
									bluTopKillstreakPlayer = assisterUserID;
								}
							}
						}
					}
				}
			}

			event->SetInt("kill_streak_assist", currentKillstreaks[assisterUserID]);
		}

		if (victimUserID != -1) {
			event->SetInt("kill_streak_victim", currentKillstreaks[victimUserID]);
		}

		RETURN_META_VALUE(MRES_HANDLED, false);
	}
	else if (strcmp(event->GetName(), "teamplay_win_panel") == 0) {
		if (event->GetInt("winning_team") == TFTeam_Red) {
			event->SetInt("killstreak_player_1", Interfaces::pEngineClient->GetPlayerForUserID(redTopKillstreakPlayer));
			event->SetInt("killstreak_player_1_count", redTopKillstreak);
		}
		else if (event->GetInt("winning_team") == TFTeam_Blue) {
			event->SetInt("killstreak_player_1", Interfaces::pEngineClient->GetPlayerForUserID(bluTopKillstreakPlayer));
			event->SetInt("killstreak_player_1_count", bluTopKillstreak);
		}

		RETURN_META_VALUE(MRES_HANDLED, false);
	}
	else if (strcmp(event->GetName(), "teamplay_round_start") == 0) {
		bluTopKillstreak = 0;
		bluTopKillstreakPlayer = 0;
		redTopKillstreak = 0;
		redTopKillstreakPlayer = 0;

		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
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

				if (Entities::CheckEntityBaseclass(entity, "TFPlayerResource")) {
					gameResourcesEntity = dynamic_cast<C_BaseEntity *>(entity);

					break;
				}
			}
		}

		for (Player player : Player::Iterable()) {
			int *killstreakPrimary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "000" });
			int *killstreakSecondary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "001" });
			int *killstreakMelee = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "002" });

			int userid = player.GetUserID();

			if (currentKillstreaks.find(userid) != currentKillstreaks.end()) {
				*killstreakPrimary = currentKillstreaks[userid];
				*killstreakSecondary = currentKillstreaks[userid];
				*killstreakMelee = currentKillstreaks[userid];

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					std::stringstream ss;
					std::string arrayIndex;
					ss << std::setfill('0') << std::setw(3) << player->entindex();
					ss >> arrayIndex;

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", arrayIndex });
					*killstreakGlobal = currentKillstreaks[userid];
				}
			}
			else {
				*killstreakPrimary = 0;
				*killstreakSecondary = 0;
				*killstreakMelee = 0;

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					std::stringstream ss;
					std::string arrayIndex;
					ss << std::setfill('0') << std::setw(3) << player->entindex();
					ss >> arrayIndex;

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", arrayIndex });
					*killstreakGlobal = 0;
				}
			}
		}

		RETURN_META(MRES_HANDLED);
	}
	else if (curStage == FRAME_START) {
		if (!Interfaces::pEngineClient->IsInGame()) {
			currentKillstreaks.clear();
		}
	}

	RETURN_META(MRES_IGNORED);
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

		int maxEntity = Interfaces::pClientEntityList->GetMaxEntities();

		for (int i = 1; i <= maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity) {
				continue;
			}

			if (Entities::CheckEntityBaseclass(entity, "TFPlayerResource")) {
				gameResourcesEntity = dynamic_cast<C_BaseEntity *>(entity);

				for (int i = 1; i <= MAX_PLAYERS; i++) {
					std::stringstream ss;
					std::string arrayIndex;
					ss << std::setfill('0') << std::setw(3) << i;
					ss >> arrayIndex;

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", arrayIndex });
					*killstreakGlobal = 0;
				}

				break;
			}
		}

		for (Player player : Player::Iterable()) {
			int *killstreakPrimary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "000" });
			int *killstreakSecondary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "001" });
			int *killstreakMelee = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "002" });

			*killstreakPrimary = 0;
			*killstreakSecondary = 0;
			*killstreakMelee = 0;
		}
	}
}