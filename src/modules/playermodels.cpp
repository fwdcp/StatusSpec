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

inline CSteamID GetClientSteamID(int client) {
	player_info_t playerInfo;

	if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, client, &playerInfo)) {
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

PlayerModels::PlayerModels() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(Interfaces::pFileSystem, "resource/playermodels.res", "mod");

	enabled = new ConVar("statusspec_playermodels_enabled", "0", FCVAR_NONE, "enable custom player models");
}

bool PlayerModels::IsEnabled() {
	return enabled->GetBool();
}

const model_t *PlayerModels::GetModelOverride(IClientRenderable *entity) {
	const model_t *model = Funcs::CallFunc_IClientRenderable_GetModel(entity);

	if (!Entities::CheckClassBaseclass(entity->GetIClientUnknown()->GetIClientEntity()->GetClientClass(), "DT_TFPlayer")) {
		return model;
	}

	const char *modelName = Interfaces::pModelInfoClient->GetModelName(model);
	CSteamID playerSteamID = GetClientSteamID(entity->GetIClientUnknown()->GetIClientEntity()->entindex());
	char *playerSteamID64 = new char[32];
	V_snprintf(playerSteamID64, sizeof(playerSteamID64), "%llu", playerSteamID.ConvertToUint64());

	KeyValues *playersConfig = modelConfig->FindKey("players");
	const char *playerGroup = playersConfig->GetString(playerSteamID64);

	if (strcmp(playerGroup, "") == 0) {
		return model;
	}

	KeyValues *groupsConfig = modelConfig->FindKey("groups");
	KeyValues *groupConfig = groupsConfig->FindKey(playerGroup);

	if (groupConfig == NULL) {
		return model;
	}

	const char *replacementModelName = groupConfig->GetString(modelName);

	if (strcmp(replacementModelName, "") == 0) {
		return model;
	}

	if (models.find(replacementModelName) == models.end()) {
		models[replacementModelName] = Interfaces::pModelInfoClient->FindOrLoadModel(replacementModelName);
	}

	return models[replacementModelName];
}

void PlayerModels::ProcessEntity(IClientEntity* entity) {
	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return;
	}

	EHANDLE entityHandle = entity->GetBaseEntity();

	if (hooks.find(entityHandle) == hooks.end()) {
		Funcs::AddHook_IClientRenderable_GetModel(entity->GetClientRenderable(), Hook_IClientRenderable_GetModel);
	}
}