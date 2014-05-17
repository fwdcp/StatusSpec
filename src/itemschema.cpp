/*
 *  itemschema.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "itemschema.h"

ItemSchema::ItemSchema() {
	itemSchema = new KeyValues("items_game");
	itemSchema->LoadFromFile(g_pFullFileSystem, "tf/scripts/items/items_game.txt");
	
	itemInfo = itemSchema->FindKey("items");
	prefabInfo = itemSchema->FindKey("prefabs");
}

ItemSchema::~ItemSchema() {
	itemSchema->deleteThis();
	itemInfo->deleteThis();
	prefabInfo->deleteThis();
}

const char *ItemSchema::GetItemKeyValue(int itemDefinitionIndex, const char *keyName) {
	KeyValues *item = itemInfo->FindKey(std::to_string((long long) itemDefinitionIndex).c_str());
	
	if (!item) {
		return NULL;
	}
	
	const char *value = item->GetString(keyName, NULL);
	
	if (value) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	const char *prefabValue;
	
	std::string prefabs = item->GetString("prefab", "");
	std::stringstream ss(prefabs);
	std::string prefabName;
	while (std::getline(ss, prefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKeyValue(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetItemKeyValue(const char *itemDefinitionIndex, const char *keyName) {
	KeyValues *item = itemInfo->FindKey(itemDefinitionIndex);
	
	if (!item) {
		return NULL;
	}
	
	const char *value = item->GetString(keyName, NULL);
	
	if (value) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	const char *prefabValue;
	
	std::string prefabs = item->GetString("prefab", "");
	std::stringstream ss(prefabs);
	std::string prefabName;
	while (std::getline(ss, prefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKeyValue(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetPrefabKeyValue(const char *prefabName, const char *keyName) {
	KeyValues *prefab = prefabInfo->FindKey(prefabName);
	
	if (prefab == NULL) {
		return NULL;
	}
	
	const char *value = prefab->GetString(keyName, NULL);
	
	if (value != NULL) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	const char *prefabValue;
	
	std::string subprefabs = prefab->GetString("prefab", "");
	std::stringstream ss(subprefabs);
	std::string subprefabName;
	while (std::getline(ss, subprefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKeyValue(subprefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue != NULL && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetPrefabKeyValue(const char *prefabName, const char *keyName, int &level) {
	KeyValues *prefab = prefabInfo->FindKey(prefabName);
	
	if (prefab == NULL) {
		return NULL;
	}
	
	const char *value = prefab->GetString(keyName, NULL);
	
	if (value != NULL) {
		return value;
	}
	
	int nextLevel = level + 1;
	int prefabLevel;
	level = -1;
	const char *prefabValue;
	
	std::string subprefabs = prefab->GetString("prefab", "");
	std::stringstream ss(subprefabs);
	std::string subprefabName;
	while (std::getline(ss, subprefabName, ' ')) {
		prefabLevel = nextLevel;
		prefabValue = GetPrefabKeyValue(subprefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue != NULL && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

void ItemSchema::ForEachItem(void (*f)(const char *)) {
	for (KeyValues *item = itemInfo->GetFirstTrueSubKey(); item; item = item->GetNextTrueSubKey()) {
		f(item->GetName());
	}
}