/*
 *  playermodels.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playermodels.h"

#include "cbase.h"
#include "convar.h"
#include "filesystem.h"
#include "KeyValues.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

PlayerModels::PlayerModels() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(g_pFullFileSystem, "resource/playermodels.res", "mod");
	setModelHook = 0;

	enabled = new ConVar("statusspec_playermodels_enabled", "0", FCVAR_NONE, "enable custom player models", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<PlayerModels>()->ToggleEnabled(var, pOldValue, flOldValue); });
	reload_settings = new ConCommand("statusspec_playermodels_reload_settings", []() { g_ModuleManager->GetModule<PlayerModels>()->ReloadSettings(); }, "reload settings for the player models from the resource file", FCVAR_NONE);
}

bool PlayerModels::CheckDependencies() {
	bool ready = true;

	if (!g_pFullFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	if (!Interfaces::pModelInfoClient) {
		PRINT_TAG();
		Warning("Required interface IVModelInfoClient for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CTFRagdoll", { "m_iPlayerIndex" })) {
		PRINT_TAG();
		Warning("Required property m_iPlayerIndex for CTFRagdoll for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_BaseEntity_SetModelIndex();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_BaseEntity::SetModelIndex for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_BaseEntity_SetModelPointer();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_BaseEntity::SetModelPointer for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	if (!Player::steamIDRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player Steam ID retrieval for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerModels>().c_str());

		ready = false;
	}

	return ready;
}

void PlayerModels::SetModelOverride(C_BaseEntity *entity, const model_t *&model) {
	Player player = entity;

	if (!player) {
		if (Entities::CheckEntityBaseclass(entity, "TFRagdoll")) {
			player = *Entities::GetEntityProp<int *>(entity, { "m_iPlayerIndex" });
		}
		else {
			return;
		}
	}

	CSteamID playerSteamID = player.GetSteamID();
	std::stringstream stringstream;
	std::string playerSteamID64;
	stringstream << playerSteamID.ConvertToUint64();
	stringstream >> playerSteamID64;

	KeyValues *playersConfig = modelConfig->FindKey("players");
	const char *playerGroup = playersConfig->GetString(playerSteamID64.c_str());

	if (strcmp(playerGroup, "") == 0) {
		return;
	}

	KeyValues *groupsConfig = modelConfig->FindKey("groups");
	KeyValues *groupConfig = groupsConfig->FindKey(playerGroup);

	if (groupConfig == NULL) {
		return;
	}

	const char *modelName = Interfaces::pModelInfoClient->GetModelName(model);

	FOR_EACH_VALUE(groupConfig, replacementModelConfig) {
		if (strcmp(replacementModelConfig->GetName(), modelName) == 0) {
			model = Interfaces::pModelInfoClient->GetModel(Interfaces::pModelInfoClient->RegisterDynamicModel(replacementModelConfig->GetString(), true));
		}
	}
}

void PlayerModels::ReloadSettings() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(g_pFullFileSystem, "resource/playermodels.res", "mod");
}

void PlayerModels::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!setModelHook) {
			setModelHook = Funcs::AddHook_C_BaseEntity_SetModel(std::bind(&PlayerModels::SetModelOverride, this, std::placeholders::_1, std::placeholders::_2));
		}
	}
	else {
		if (setModelHook) {
			Funcs::RemoveHook_C_BaseEntity_SetModel(setModelHook);
			setModelHook = 0;
		}
	}
}