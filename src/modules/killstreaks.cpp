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

#include "cbase.h"
#include "c_baseentity.h"
#include "convar.h"
#include "igameevents.h"
#include "shareddefs.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class Killstreaks::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName);
	virtual ~Panel();

	virtual void OnTick();
private:
	bool FireEventClientSideOverride(IGameEvent *event);

	int bluTopKillstreak;
	int bluTopKillstreakPlayer;
	std::map<int, int> currentKillstreaks;
	int fireEventClientSideHook;
	CHandle<C_BaseEntity> gameResourcesEntity;
	int redTopKillstreak;
	int redTopKillstreakPlayer;
};

Killstreaks::Killstreaks() {
	panel = nullptr;

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
		char index[4];
		GetPropIndexString(i, index);

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_hMyWeapons", index })) {
			PRINT_TAG();
			Warning("Required property table m_hMyWeapons for CTFPlayer for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

			ready = false;

			break;
		}
	}

	for (int i = 0; i < 3; i++) {
		char index[4];
		GetPropIndexString(i, index);

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nStreaks", index })) {
			PRINT_TAG();
			Warning("Required property table m_nStreaks for CTFPlayer for module %s not available!\n", g_ModuleManager->GetModuleName<Killstreaks>().c_str());

			ready = false;

			break;
		}
	}

	for (int i = 0; i <= MAX_PLAYERS; i++) {
		char index[4];
		GetPropIndexString(i, index);

		if (!Entities::RetrieveClassPropOffset("CTFPlayerResource", { "m_iStreaks", index })) {
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

void Killstreaks::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			panel = new Panel(nullptr, "Killstreaks");
		}

		if (panel) {
			panel->SetEnabled(true);
		}
	}
	else {
		if (panel) {
			delete panel;
			panel = nullptr;
		}
	}
}

Killstreaks::Panel::Panel(vgui::Panel *parent, const char *panelName) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());

	bluTopKillstreak = 0;
	bluTopKillstreakPlayer = 0;
	fireEventClientSideHook = Funcs::AddHook_IGameEventManager2_FireEventClientSide(Interfaces::pGameEventManager, SH_MEMBER(this, &Killstreaks::Panel::FireEventClientSideOverride), false);
	redTopKillstreak = 0;
	redTopKillstreakPlayer = 0;
}

Killstreaks::Panel::~Panel() {
	Funcs::RemoveHook(fireEventClientSideHook);

	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 1; i <= maxEntity; i++) {
		IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TFPlayerResource")) {
			for (int i = 1; i <= MAX_PLAYERS; i++) {
				char index[4];
				GetPropIndexString(i, index);

				int *killstreakGlobal = Entities::GetEntityProp<int *>(entity, { "m_iStreaks", index });
				*killstreakGlobal = 0;
			}

			break;
		}
	}

	for (Player player : Player::Iterable()) {
		int *killstreakPrimary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "000" });
		int *killstreakSecondary = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "001" });
		int *killstreakMelee = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "002" });
		int *killstreakPDA = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "003" });

		*killstreakPrimary = 0;
		*killstreakSecondary = 0;
		*killstreakMelee = 0;
		*killstreakPDA = 0;
	}
}

void Killstreaks::Panel::OnTick() {
	if (Interfaces::pEngineClient->IsConnected()) {
		if (!gameResourcesEntity.IsValid() || !gameResourcesEntity.Get()) {
			int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

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
			int *killstreakPDA = Entities::GetEntityProp<int *>(player.GetEntity(), { "m_nStreaks", "003" });

			int userid = player.GetUserID();

			if (currentKillstreaks.find(userid) != currentKillstreaks.end()) {
				*killstreakPrimary = currentKillstreaks[userid];
				*killstreakSecondary = currentKillstreaks[userid];
				*killstreakMelee = currentKillstreaks[userid];
				*killstreakPDA = currentKillstreaks[userid];

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					char index[4];
					GetPropIndexString(player->entindex(), index);

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", index });
					*killstreakGlobal = currentKillstreaks[userid];
				}
			}
			else {
				*killstreakPrimary = 0;
				*killstreakSecondary = 0;
				*killstreakMelee = 0;
				*killstreakPDA = 0;

				if (gameResourcesEntity.IsValid() && gameResourcesEntity.Get()) {
					char index[4];
					GetPropIndexString(player->entindex(), index);

					int *killstreakGlobal = Entities::GetEntityProp<int *>(gameResourcesEntity.Get(), { "m_iStreaks", index });
					*killstreakGlobal = 0;
				}
			}
		}
	}
	else {
		currentKillstreaks.clear();
	}
}

bool Killstreaks::Panel::FireEventClientSideOverride(IGameEvent *event) {
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
					char index[4];
					GetPropIndexString(i, index);

					IClientEntity *weapon = Entities::GetEntityProp<EHANDLE *>(assister.GetEntity(), { "m_hMyWeapons", index })->Get();

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