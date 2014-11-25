/*
 *  playermodels.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playermodels.h"

PlayerModels::PlayerModels() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(Interfaces::pFileSystem, "resource/playermodels.res", "mod");
	setModelHook = 0;

	enabled = new ConVar("statusspec_playermodels_enabled", "0", FCVAR_NONE, "enable custom player models", [](IConVar *var, const char *pOldValue, float flOldValue) { g_PlayerModels->ToggleEnabled(var, pOldValue, flOldValue); });
}

void PlayerModels::SetModelOverride(C_BaseEntity *entity, const model_t *&model) {
	Player player = entity;

	if (!player) {
		return;
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