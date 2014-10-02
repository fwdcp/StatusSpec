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

	enabled = new ConVar("statusspec_customtextures_enabled", "0", FCVAR_NONE, "enable custom materials", CustomTextures::ToggleEnabled);
	load_replacement_group = new ConCommand("statusspec_customtextures_load_replacement_group", CustomTextures::LoadReplacementGroup, "load a texture replacement group", FCVAR_NONE);
	unload_replacement_group = new ConCommand("statusspec_customtextures_unload_replacement_group", CustomTextures::UnloadReplacementGroup, "unload a texture replacement group", FCVAR_NONE);
}

bool CustomTextures::IsEnabled() {
	return enabled->GetBool();
}

void CustomTextures::LoadReplacementGroup(const CCommand &command) {
	if (command.ArgC() >= 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = g_CustomTextures->textureConfig->FindKey(group);

	if (replacementsConfig) {
		FOR_EACH_VALUE(replacementsConfig, textureReplacement) {
			std::string original = textureReplacement->GetName();

			if (g_CustomTextures->textureReplacements.find(original) != g_CustomTextures->textureReplacements.end()) {
				if (g_CustomTextures->IsEnabled()) {
					g_pMaterialSystem->RemoveTextureAlias(original.c_str());
				}

				g_CustomTextures->textureReplacements.erase(original);
			}
				
			g_CustomTextures->textureReplacements[original].group = group;
			g_CustomTextures->textureReplacements[original].replacement = textureReplacement->GetString();

			if (g_CustomTextures->IsEnabled()) {
				g_pMaterialSystem->AddTextureAlias(textureReplacement->GetName(), textureReplacement->GetString());
			}
		}
	}
	else {
		Warning("Must specify a valid replacement group to load!\n");
	}
}

void CustomTextures::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (g_CustomTextures->IsEnabled()) {
		for (auto iterator = g_CustomTextures->textureReplacements.begin(); iterator != g_CustomTextures->textureReplacements.end(); ++iterator) {
			g_pMaterialSystem->AddTextureAlias(iterator->first.c_str(), iterator->second.replacement.c_str());
		}
	}
	else {
		for (auto iterator = g_CustomTextures->textureReplacements.begin(); iterator != g_CustomTextures->textureReplacements.end(); ++iterator) {
			g_pMaterialSystem->RemoveTextureAlias(iterator->first.c_str());
		}
	}
}

void CustomTextures::UnloadReplacementGroup(const CCommand &command) {
	if (command.ArgC() >= 2) {
		Warning("Must specify a replacement group to load!\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = g_CustomTextures->textureConfig->FindKey(group);

	if (replacementsConfig) {
		auto iterator = g_CustomTextures->textureReplacements.begin();

		while (iterator != g_CustomTextures->textureReplacements.end()) {
			if (iterator->second.group.compare(group) == 0) {
				if (g_CustomTextures->IsEnabled()) {
					g_pMaterialSystem->RemoveTextureAlias(iterator->first.c_str());
				}

				g_CustomTextures->textureReplacements.erase(iterator++);
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