/*
*  custommodels.cpp
*  StatusSpec project
*
*  Copyright (c) 2014-2015 Forward Command Post
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "custommodels.h"

#include "convar.h"
#include "engine/ivmodelinfo.h"
#include "filesystem.h"
#include "KeyValues.h"

#include "../exceptions.h"
#include "../funcs.h"
#include "../ifaces.h"

CustomModels::CustomModels() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(g_pFullFileSystem, "resource/custommodels.res", "mod");
	setModelHook = 0;

	enabled = new ConVar("statusspec_custommodels_enabled", "0", FCVAR_NONE, "enable custom models", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CustomModels>()->ToggleEnabled(var, pOldValue, flOldValue); });
	load_replacement_group = new ConCommand("statusspec_custommodels_load_replacement_group", [](const CCommand &command) { g_ModuleManager->GetModule<CustomModels>()->LoadReplacementGroup(command); }, "load a model replacement group", FCVAR_NONE);
	reload_settings = new ConCommand("statusspec_custommodels_reload_settings", []() { g_ModuleManager->GetModule<CustomModels>()->ReloadSettings(); }, "reload settings for the custom models from the resource file", FCVAR_NONE);
	unload_replacement_group = new ConCommand("statusspec_custommodels_unload_replacement_group", [](const CCommand &command) { g_ModuleManager->GetModule<CustomModels>()->UnloadReplacementGroup(command); }, "unload a model replacement group", FCVAR_NONE);
}

bool CustomModels::CheckDependencies() {
	bool ready = true;

	if (!g_pFullFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", g_ModuleManager->GetModuleName<CustomModels>().c_str());

		ready = false;
	}

	if (!Interfaces::pModelInfoClient) {
		PRINT_TAG();
		Warning("Required interface IVModelInfoClient for module %s not available!\n", g_ModuleManager->GetModuleName<CustomModels>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_BaseEntity_SetModelIndex();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_BaseEntity::SetModelIndex for module %s not available!\n", g_ModuleManager->GetModuleName<CustomModels>().c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_BaseEntity_SetModelPointer();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required function C_BaseEntity::SetModelPointer for module %s not available!\n", g_ModuleManager->GetModuleName<CustomModels>().c_str());

		ready = false;
	}

	return ready;
}

void CustomModels::SetModelOverride(C_BaseEntity *entity, const model_t *&model) {
	const char *modelName = Interfaces::pModelInfoClient->GetModelName(model);

	if (modelReplacements.find(modelName) != modelReplacements.end()) {
		model = Interfaces::pModelInfoClient->GetModel(Interfaces::pModelInfoClient->RegisterDynamicModel(modelReplacements[modelName].replacement.c_str(), true));
	}
}

void CustomModels::LoadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = modelConfig->FindKey(group);

	if (replacementsConfig) {
		FOR_EACH_VALUE(replacementsConfig, modelReplacement) {
			std::string original = modelReplacement->GetName();

			if (modelReplacements.find(original) != modelReplacements.end()) {
				modelReplacements.erase(original);
			}

			modelReplacements[original].group = group;
			modelReplacements[original].replacement = modelReplacement->GetString();
		}
	}
	else {
		Warning("Must specify a valid replacement group to load!\n");
	}
}

void CustomModels::ReloadSettings() {
	modelConfig = new KeyValues("models");
	modelConfig->LoadFromFile(g_pFullFileSystem, "resource/custommodels.res", "mod");
}

void CustomModels::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!setModelHook) {
			setModelHook = Funcs::AddHook_C_BaseEntity_SetModel(std::bind(&CustomModels::SetModelOverride, this, std::placeholders::_1, std::placeholders::_2));
		}
	}
	else {
		if (setModelHook) {
			Funcs::RemoveHook_C_BaseEntity_SetModel(setModelHook);
			setModelHook = 0;
		}
	}
}

void CustomModels::UnloadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = modelConfig->FindKey(group);

	if (replacementsConfig) {
		auto iterator = modelReplacements.begin();

		while (iterator != modelReplacements.end()) {
			if (iterator->second.group.compare(group) == 0) {
				modelReplacements.erase(iterator++);
			}
			else {
				++iterator;
			}
		}
	}
	else {
		Warning("Must specify a valid replacement group to unload!\n");
	}
}