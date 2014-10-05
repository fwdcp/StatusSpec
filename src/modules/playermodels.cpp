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

	enabled = new ConVar("statusspec_playermodels_enabled", "0", FCVAR_NONE, "enable custom player models");
}

bool PlayerModels::IsEnabled() {
	return enabled->GetBool();
}

const model_t *PlayerModels::SetModelOverride(C_BaseEntity *entity, const model_t *model) {
	Player player = entity;

	if (!player) {
		return model;
	}

	CSteamID playerSteamID = player.GetSteamID();
	std::stringstream stringstream;
	std::string playerSteamID64;
	stringstream << playerSteamID.ConvertToUint64();
	stringstream >> playerSteamID64;

	KeyValues *playersConfig = modelConfig->FindKey("players");
	const char *playerGroup = playersConfig->GetString(playerSteamID64.c_str());

	if (strcmp(playerGroup, "") == 0) {
		return model;
	}

	KeyValues *groupsConfig = modelConfig->FindKey("groups");
	KeyValues *groupConfig = groupsConfig->FindKey(playerGroup);

	if (groupConfig == NULL) {
		return model;
	}

	const char *modelName = Interfaces::pModelInfoClient->GetModelName(model);

	FOR_EACH_VALUE(groupConfig, replacementModelConfig) {
		if (strcmp(replacementModelConfig->GetName(), modelName) == 0) {
			return Interfaces::pModelInfoClient->GetModel(Interfaces::pModelInfoClient->RegisterDynamicModel(replacementModelConfig->GetString(), true));
		}
	}

	return model;
}