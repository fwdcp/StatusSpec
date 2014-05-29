/*
 *  statusspec.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include "ifaces.h"

#include "KeyValues.h"
#include "filesystem.h"

#include <vector>
#include <string>
#include <sstream>

class ItemSchema {
	public:
		ItemSchema();
		~ItemSchema();
		KeyValues *GetItemKey(int itemDefinitionIndex, const char *keyName);
		KeyValues *GetItemKey(const char *itemDefinitionIndex, const char *keyName);
		KeyValues *GetPrefabKey(const char *prefabName, const char *keyName);
		const char *GetItemKeyData(int itemDefinitionIndex, const char *keyName);
		const char *GetItemKeyData(const char *itemDefinitionIndex, const char *keyName);
		const char *GetPrefabKeyData(const char *prefabName, const char *keyName);
		void ForEachItem(void (*f)(const char *));
	private:
		KeyValues *itemSchema;
		KeyValues *itemInfo;
		KeyValues *prefabInfo;
		KeyValues *GetPrefabKey(const char *prefabName, const char *keyName, int &level);
		const char *GetPrefabKeyData(const char *prefabName, const char *keyName, int &level);
};