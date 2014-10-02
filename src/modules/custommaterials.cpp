/*
*  custommaterials.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "custommaterials.h"

CustomMaterials::CustomMaterials() {
	materialConfig = new KeyValues("materials");
	materialConfig->LoadFromFile(Interfaces::pFileSystem, "resource/custommaterials.res", "mod");

	enabled = new ConVar("statusspec_custommaterials_enabled", "0", FCVAR_NONE, "enable custom materials");
	load_replacement_group = new ConCommand("statusspec_custommaterials_load_replacement_group", CustomMaterials::LoadReplacementGroup, "load a material replacement group", FCVAR_NONE);
	unload_replacement_group = new ConCommand("statusspec_custommaterials_unload_replacement_group", CustomMaterials::UnloadReplacementGroup, "unload a material replacement group", FCVAR_NONE);
}

bool CustomMaterials::IsEnabled() {
	return enabled->GetBool();
}

char const *CustomMaterials::LoadMaterialOverride(char const *material) {
	if (materialReplacements.find(material) != materialReplacements.end()) {
		return materialReplacements[material].replacement.c_str();
	}

	return material;
}

void CustomMaterials::LoadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Usage: statusspec_custommaterials_load_replacement_group <group>\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = g_CustomMaterials->materialConfig->FindKey(group);

	if (replacementsConfig) {
		FOR_EACH_VALUE(replacementsConfig, materialReplacement) {
			std::string original = materialReplacement->GetName();

			g_CustomMaterials->materialReplacements[original].group = group;
			g_CustomMaterials->materialReplacements[original].replacement = materialReplacement->GetString();
		}
	}
	else {
		Warning("Must specify a valid replacement group to load!\n");
	}
}

void CustomMaterials::UnloadReplacementGroup(const CCommand &command) {
	if (command.ArgC() < 2) {
		Warning("Usage: statusspec_custommaterials_unload_replacement_group <group>\n");
		return;
	}

	const char *group = command.Arg(1);

	KeyValues *replacementsConfig = g_CustomMaterials->materialConfig->FindKey(group);

	if (replacementsConfig) {
		auto iterator = g_CustomMaterials->materialReplacements.begin();

		while (iterator != g_CustomMaterials->materialReplacements.end()) {
			if (iterator->second.group.compare(group) == 0) {
				g_CustomMaterials->materialReplacements.erase(iterator++);
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