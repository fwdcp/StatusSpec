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
	itemSchema->LoadFromFile(Interfaces::pFileSystem, "scripts/items/items_game.txt", "mod");
	
	itemInfo = itemSchema->FindKey("items");
	prefabInfo = itemSchema->FindKey("prefabs");
}

ItemSchema::~ItemSchema() {
	itemSchema->deleteThis();
	itemInfo->deleteThis();
	prefabInfo->deleteThis();
}

KeyValues *ItemSchema::GetItemKey(int itemDefinitionIndex, const char *keyName) {
	KeyValues *item = itemInfo->FindKey(std::to_string((long long) itemDefinitionIndex).c_str());
	
	if (!item) {
		return NULL;
	}
	
	KeyValues *value = item->FindKey(keyName, false);
	
	if (value) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	KeyValues *prefabValue;
	
	std::string prefabs = item->GetString("prefab", "");
	std::stringstream ss(prefabs);
	std::string prefabName;
	while (std::getline(ss, prefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKey(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

KeyValues *ItemSchema::GetItemKey(const char *itemDefinitionIndex, const char *keyName) {
	KeyValues *item = itemInfo->FindKey(itemDefinitionIndex);
	
	if (!item) {
		return NULL;
	}
	
	KeyValues *value = item->FindKey(keyName, false);
	
	if (value) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	KeyValues *prefabValue;
	
	std::string prefabs = item->GetString("prefab", "");
	std::stringstream ss(prefabs);
	std::string prefabName;
	while (std::getline(ss, prefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKey(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

KeyValues *ItemSchema::GetPrefabKey(const char *prefabName, const char *keyName) {
	KeyValues *prefab = prefabInfo->FindKey(prefabName);
	
	if (prefab == NULL) {
		return NULL;
	}
	
	KeyValues *value = prefab->FindKey(keyName, false);
	
	if (value != NULL) {
		return value;
	}
	
	int level = -1;
	int prefabLevel;
	KeyValues *prefabValue;
	
	std::string subprefabs = prefab->GetString("prefab", "");
	std::stringstream ss(subprefabs);
	std::string subprefabName;
	while (std::getline(ss, subprefabName, ' ')) {
		prefabLevel = 1;
		prefabValue = GetPrefabKey(subprefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue != NULL && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

KeyValues *ItemSchema::GetPrefabKey(const char *prefabName, const char *keyName, int &level) {
	KeyValues *prefab = prefabInfo->FindKey(prefabName);
	
	if (prefab == NULL) {
		return NULL;
	}
	
	KeyValues *value = prefab->FindKey(keyName, false);
	
	if (value != NULL) {
		return value;
	}
	
	int nextLevel = level + 1;
	int prefabLevel;
	level = -1;
	KeyValues *prefabValue;
	
	std::string subprefabs = prefab->GetString("prefab", "");
	std::stringstream ss(subprefabs);
	std::string subprefabName;
	while (std::getline(ss, subprefabName, ' ')) {
		prefabLevel = nextLevel;
		prefabValue = GetPrefabKey(subprefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue != NULL && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetItemKeyData(int itemDefinitionIndex, const char *keyName) {
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
		prefabValue = GetPrefabKeyData(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetItemKeyData(const char *itemDefinitionIndex, const char *keyName) {
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
		prefabValue = GetPrefabKeyData(prefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetPrefabKeyData(const char *prefabName, const char *keyName) {
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
		prefabValue = GetPrefabKeyData(subprefabName.c_str(), keyName, prefabLevel);
		
		if (prefabValue != NULL && (level == -1 || prefabLevel < level)) {
			value = prefabValue;
			level = prefabLevel;
		}
    }
	
	return value;
}

const char *ItemSchema::GetPrefabKeyData(const char *prefabName, const char *keyName, int &level) {
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
		prefabValue = GetPrefabKeyData(subprefabName.c_str(), keyName, prefabLevel);
		
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