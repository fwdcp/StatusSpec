/*
 *  customtextures.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "customtextures.h"

#include "convar.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/imaterialsystem.h"

#include "../ifaces.h"

CustomTextures::CustomTextures() {
	textureConfig = new KeyValues("textures");
	textureConfig->LoadFromFile(g_pFullFileSystem, "resource/customtextures.res", "mod");

	enabled = new ConVar("statusspec_customtextures_enabled", "0", FCVAR_NONE, "enable custom materials", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CustomTextures>()->ToggleEnabled(var, pOldValue, flOldValue); });
	load_replacement_group = new ConCommand("statusspec_customtextures_load_replacement_group", [](const CCommand &command) { g_ModuleManager->GetModule<CustomTextures>()->LoadReplacementGroup(command); }, "load a texture replacement group", FCVAR_NONE);
	reload_settings = new ConCommand("statusspec_customtextures_reload_settings", []() { g_ModuleManager->GetModule<CustomTextures>()->ReloadSettings(); }, "reload settings for the custom textures from the resource file", FCVAR_NONE);
	unload_replacement_group = new ConCommand("statusspec_customtextures_unload_replacement_group", [](const CCommand &command) { g_ModuleManager->GetModule<CustomTextures>()->UnloadReplacementGroup(command); }, "unload a texture replacement group", FCVAR_NONE);
}

bool CustomTextures::CheckDependencies() {
	bool ready = true;

	if (!g_pFullFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", g_ModuleManager->GetModuleName<CustomTextures>().c_str());

		ready = false;
	}

	if (!g_pMaterialSystem) {
		PRINT_TAG();
		Warning("Required interface IMaterialSystem for module %s not available!\n", g_ModuleManager->GetModuleName<CustomTextures>().c_str());

		ready = false;
	}

	return ready;
}

void CustomTextures::LoadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = textureConfig->FindKey(group);

	if (replacementsConfig) {
		FOR_EACH_VALUE(replacementsConfig, textureReplacement) {
			std::string original = textureReplacement->GetName();

			if (textureReplacements.find(original) != textureReplacements.end()) {
				if (enabled->GetBool()) {
					g_pMaterialSystem->RemoveTextureAlias(original.c_str());
				}

				textureReplacements.erase(original);
			}
				
			textureReplacements[original].group = group;
			textureReplacements[original].replacement = textureReplacement->GetString();

			if (enabled->GetBool()) {
				g_pMaterialSystem->AddTextureAlias(textureReplacement->GetName(), textureReplacement->GetString());
			}
		}
	}
	else {
		Warning("Must specify a valid replacement group to load!\n");
	}
}

void CustomTextures::ReloadSettings() {
	textureConfig = new KeyValues("textures");
	textureConfig->LoadFromFile(g_pFullFileSystem, "resource/customtextures.res", "mod");
}

void CustomTextures::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		for (auto iterator : textureReplacements) {
			g_pMaterialSystem->AddTextureAlias(iterator.first.c_str(), iterator.second.replacement.c_str());
		}
	}
	else {
		for (auto iterator : textureReplacements) {
			g_pMaterialSystem->RemoveTextureAlias(iterator.first.c_str());
		}
	}
}

void CustomTextures::UnloadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = textureConfig->FindKey(group);

	if (replacementsConfig) {
		auto iterator = textureReplacements.begin();

		while (iterator != textureReplacements.end()) {
			if (iterator->second.group.compare(group) == 0) {
				if (enabled->GetBool()) {
					g_pMaterialSystem->RemoveTextureAlias(iterator->first.c_str());
				}

				textureReplacements.erase(iterator++);
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