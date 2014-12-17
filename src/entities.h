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

#include <cstdarg>
#include <map>
#include <vector>

#include "cdll_client_int.h"
#include "client_class.h"
#include "ehandle.h"
#include "icliententity.h"
#include "shareddefs.h"

#include "exceptions.h"
#include "ifaces.h"

typedef struct ClassPropDefinition {
	bool operator<(const ClassPropDefinition &other) const {
		if (className.compare(other.className) != 0) {
			return className.compare(other.className) < 0;
		}

		return propertyTree < other.propertyTree;
	}

	std::string className;
	std::vector<std::string> propertyTree;
} ClassPropDefinition;

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

	static std::map<ClassPropDefinition, int> classPropOffsets;
};