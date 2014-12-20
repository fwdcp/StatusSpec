/*
 *  entities.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include <functional>
#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>

#include "cdll_client_int.h"
#include "client_class.h"
#include "ehandle.h"
#include "icliententity.h"
#include "shareddefs.h"

#include "exceptions.h"
#include "ifaces.h"

class Entities {
public:
	static bool RetrieveClassPropOffset(std::string className, std::vector<std::string> propertyTree);
	template <typename T> static T GetEntityProp(IClientEntity *entity, std::vector<std::string> propertyTree) {
		return reinterpret_cast<T>(GetEntityProp(entity, propertyTree));
	};

	static bool CheckEntityBaseclass(IClientEntity *entity, std::string baseclass);

private:
	static bool GetSubProp(RecvTable *table, const char *propName, RecvProp *&prop, int &offset);
	static void *GetEntityProp(IClientEntity *entity, std::vector<std::string> propertyTree);

	static bool CheckClassBaseclass(ClientClass *clientClass, std::string baseclass);
	static bool CheckTableBaseclass(RecvTable *sTable, std::string baseclass);

	static std::unordered_map<std::string, std::unordered_map<std::string, int>> classPropOffsets;
};