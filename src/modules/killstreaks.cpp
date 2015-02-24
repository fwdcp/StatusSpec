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

Killstreaks::Killstreaks(std::string name) : Module(name) {
	bluTopKillstreak = 0;
	bluTopKillstreakPlayer = 0;
	fireEventClientSideHook = 0;
	frameHook = 0;
	redTopKillstreak = 0;
	redTopKillstreakPlayer = 0;

	enabled = new ConVar("statusspec_killstreaks_enabled", "0", FCVAR_NONE, "enable killstreaks display", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<Killstreaks>("Killstreaks")->ToggleEnabled(var, pOldValue, flOldValue); });
	total_killfeed = new ConVar("statusspec_killstreaks_total_killfeed", "0", FCVAR_NONE, "display total kills for player in killfeed instead of only kills with single weapon");
}

bool Killstreaks::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pGameEventManager) {
		PRINT_TAG();
		Warning("Required interface IGameEventManager2 for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	for (int i = 0; i < MAX_WEAPONS; i++) {
		std::stringstream ss;
		std::string arrayIndex;
		ss << std::setfill('0') << std::setw(3) << i;
		ss >> arrayIndex;

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_hMyWeapons", arrayIndex })) {
			PRINT_TAG();
			Warning("Required property table m_hMyWeapons for CTFPlayer for module %s not available!\n", name.c_str());

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
			Warning("Required property table m_nStreaks for CTFPlayer for module %s not available!\n", name.c_str());

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
			Warning("Required property table m_iStreaks for CTFPlayerResource for module %s not available!\n", name.c_str());

			ready = false;

			break;
		}
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_bHealing" })) {
		PRINT_TAG();
		Warning("Required property m_bHealing for CWeaponMedigun for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_hHealingTarget" })) {
		PRINT_TAG();
		Warning("Required property m_hHealingTarget for CWeaponMedigun for module %s not available!\n", name.c_str());

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
				currentKillstreaks[attackerUserID][weapon]++;

				event->SetInt("kill_streak_total", GetCurrentPlayerKillstreak(attackerUserID));

				if (total_killfeed->GetBool()) {
					event->SetInt("kill_streak_wep", GetCurrentPlayerKillstreak(attackerUserID));
				}
				else {
					if (IsAttributableKill(weapon)) {
						event->SetInt("kill_streak_wep", GetCurrentSlotKillstreak(attackerUserID, GetKillTypeSlot(weapon)));
					}
					else {
						event->SetInt("kill_streak_wep", 0);
					}
				}

				Player attacker = Interfaces::pEngineClient->GetPlayerForUserID(attackerUserID);

				if (attacker) {
					if (attacker.GetTeam() == TFTeam_Red) {
						if (GetCurrentPlayerKillstreak(attackerUserID) > redTopKillstreak) {
							redTopKillstreak = GetCurrentPlayerKillstreak(attackerUserID);
							redTopKillstreakPlayer = attackerUserID;
						}
					}
					else if (attacker.GetTeam() == TFTeam_Blue) {
						if (GetCurrentPlayerKillstreak(attackerUserID) > bluTopKillstreak) {
							bluTopKillstreak = GetCurrentPlayerKillstreak(attackerUserID);
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
							// add medigun killstreak onto a special "kill type"
							currentKillstreaks[assisterUserID]["medigun"]++;

							if (assister.GetTeam() == TFTeam_Red) {
								if (GetCurrentPlayerKillstreak(assisterUserID) > redTopKillstreak) {
									redTopKillstreak = GetCurrentPlayerKillstreak(assisterUserID);
									redTopKillstreakPlayer = assisterUserID;
								}
							}
							else if (assister.GetTeam() == TFTeam_Blue) {
								if (GetCurrentPlayerKillstreak(assisterUserID) > bluTopKillstreak) {
									bluTopKillstreak = GetCurrentPlayerKillstreak(assisterUserID);
									bluTopKillstreakPlayer = assisterUserID;
								}
							}
						}
					}
				}
			}

			event->SetInt("kill_streak_assist", GetCurrentPlayerKillstreak(assisterUserID));
		}

		if (victimUserID != -1) {
			event->SetInt("kill_streak_victim", GetCurrentPlayerKillstreak(victimUserID));
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

		for (auto iterator = Player::begin(); iterator != Player::end(); ++iterator) {
			Player player = *iterator;

			int *killstreakPrimary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "000" });
			int *killstreakSecondary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "001" });
			int *killstreakMelee = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "002" });

			int userid = player.GetUserID();

			if (currentKillstreaks.find(userid) != currentKillstreaks.end()) {
				*killstreakPrimary = GetCurrentSlotKillstreak(userid, 0);
				*killstreakSecondary = GetCurrentSlotKillstreak(userid, 1);
				*killstreakMelee = GetCurrentSlotKillstreak(userid, 2);

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					std::stringstream ss;
					std::string arrayIndex;
					ss << std::setfill('0') << std::setw(3) << player->entindex();
					ss >> arrayIndex;

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", arrayIndex });
					*killstreakGlobal = GetCurrentPlayerKillstreak(userid);
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

int Killstreaks::GetCurrentPlayerKillstreak(int userid) {
	int killstreak = 0;

	for (auto iterator = currentKillstreaks[userid].begin(); iterator != currentKillstreaks[userid].end(); ++iterator) {
		killstreak += iterator->second;
	}

	return killstreak;
}

int Killstreaks::GetCurrentSlotKillstreak(int userid, int slot) {
	int killstreak = 0;

	for (auto iterator = TFDefinitions::slotKillIcons.find(slot)->second.begin(); iterator != TFDefinitions::slotKillIcons.find(slot)->second.end(); ++iterator) {
		killstreak += currentKillstreaks[userid][*iterator];
	}

	if (slot == 1) {
		// mediguns aren't kill icons, so let's add them in manually to the secondary slot
		killstreak += currentKillstreaks[userid]["medigun"];
	}

	return killstreak;
}

int Killstreaks::GetKillTypeSlot(std::string killType) {
	for (auto slotIterator = TFDefinitions::slotKillIcons.begin(); slotIterator != TFDefinitions::slotKillIcons.end(); ++slotIterator) {
		for (auto iterator = slotIterator->second.begin(); iterator != slotIterator->second.end(); ++iterator) {
			if (killType.compare(*iterator) == 0) {
				return slotIterator->first;
			}
		}
	}

	return -1;
}

bool Killstreaks::IsAttributableKill(std::string killType) {
	for (auto slotIterator = TFDefinitions::slotKillIcons.begin(); slotIterator != TFDefinitions::slotKillIcons.end(); ++slotIterator) {
		for (auto iterator = slotIterator->second.begin(); iterator != slotIterator->second.end(); ++iterator) {
			if (killType.compare(*iterator) == 0) {
				return true;
			}
		}
	}

	return false;
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

		for (auto iterator = Player::begin(); iterator != Player::end(); ++iterator) {
			Player player = *iterator;

			int *killstreakPrimary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "000" });
			int *killstreakSecondary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "001" });
			int *killstreakMelee = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "002" });

			*killstreakPrimary = 0;
			*killstreakSecondary = 0;
			*killstreakMelee = 0;
		}
	}
}