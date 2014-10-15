/*
 *  customtextures.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "customtextures.h"

CustomTextures::CustomTextures() {
	textureConfig = new KeyValues("textures");
	textureConfig->LoadFromFile(Interfaces::pFileSystem, "resource/customtextures.res", "mod");

	enabled = new ConVar("statusspec_customtextures_enabled", "0", FCVAR_NONE, "enable custom materials", [](IConVar *var, const char *pOldValue, float flOldValue) { g_CustomTextures->ToggleEnabled(var, pOldValue, flOldValue); });
	load_replacement_group = new ConCommand("statusspec_customtextures_load_replacement_group", [](const CCommand &command) { g_CustomTextures->LoadReplacementGroup(command); }, "load a texture replacement group", FCVAR_NONE);
	unload_replacement_group = new ConCommand("statusspec_customtextures_unload_replacement_group", [](const CCommand &command) { g_CustomTextures->UnloadReplacementGroup(command); }, "unload a texture replacement group", FCVAR_NONE);
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

void CustomTextures::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		for (auto iterator = textureReplacements.begin(); iterator != textureReplacements.end(); ++iterator) {
			g_pMaterialSystem->AddTextureAlias(iterator->first.c_str(), iterator->second.replacement.c_str());
		}
	}
	else {
		for (auto iterator = textureReplacements.begin(); iterator != textureReplacements.end(); ++iterator) {
			g_pMaterialSystem->RemoveTextureAlias(iterator->first.c_str());
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