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
	setModelIndexDetoured = false;
	setModelPointerDetoured = false;

	enabled = new ConVar("statusspec_playermodels_enabled", "0", FCVAR_NONE, "enable custom player models", [](IConVar *var, const char *pOldValue, float flOldValue) { g_PlayerModels->ToggleEnabled(var, pOldValue, flOldValue); });
}

void PlayerModels::SetModelIndexOverride(C_BaseEntity *instance, int index) {
	const model_t *oldModel = Interfaces::pModelInfoClient->GetModel(index);
	const model_t *newModel = GetModelOverride(instance, oldModel);
	int newIndex = Interfaces::pModelInfoClient->GetModelIndex(Interfaces::pModelInfoClient->GetModelName(newModel));

	Funcs::CallFunc_C_BaseEntity_SetModelIndex(instance, newIndex);
}

void PlayerModels::SetModelPointerOverride(C_BaseEntity *instance, const model_t *pModel) {
	const model_t *oldModel = pModel;
	const model_t *newModel = GetModelOverride(instance, oldModel);

	Funcs::CallFunc_C_BaseEntity_SetModelPointer(instance, newModel);
}

const model_t *PlayerModels::GetModelOverride(C_BaseEntity *entity, const model_t *model) {
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


inline void __fastcall Detour_C_BaseEntity_SetModelIndex(C_BaseEntity *instance, void *, int index) {
	g_PlayerModels->SetModelIndexOverride(instance, index);
}

inline void __fastcall Detour_C_BaseEntity_SetModelPointer(C_BaseEntity *instance, void *, const model_t *pModel) {
	g_PlayerModels->SetModelPointerOverride(instance, pModel);
}

void PlayerModels::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!setModelIndexDetoured) {
			setModelIndexDetoured = Funcs::AddDetour_C_BaseEntity_SetModelIndex(Detour_C_BaseEntity_SetModelIndex);
		}
		if (!setModelPointerDetoured) {
			setModelPointerDetoured = Funcs::AddDetour_C_BaseEntity_SetModelPointer(Detour_C_BaseEntity_SetModelPointer);
		}
	}
	else {
		if (setModelIndexDetoured) {
			setModelIndexDetoured = !Funcs::RemoveDetour_C_BaseEntity_SetModelIndex();
		}
		if (setModelPointerDetoured) {
			setModelPointerDetoured = !Funcs::RemoveDetour_C_BaseEntity_SetModelPointer();
		}
	}
}