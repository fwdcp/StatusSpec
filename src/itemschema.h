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

#include <vector>
#include <string>
#include <sstream>

class ItemSchema {
	public:
		ItemSchema();
		~ItemSchema();
		const char *GetItemKeyValue(int itemDefinitionIndex, const char *keyName);
		const char *GetItemKeyValue(const char *itemDefinitionIndex, const char *keyName);
		const char *GetPrefabKeyValue(const char *prefabName, const char *keyName);
		void ForEachItem(void (*f)(const char *));
	private:
		KeyValues *itemSchema;
		KeyValues *itemInfo;
		KeyValues *prefabInfo;
		const char *GetPrefabKeyValue(const char *prefabName, const char *keyName, int &level);
};