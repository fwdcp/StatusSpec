/*
 *  entities.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "entities.h"

#include "cdll_int.h"
#include "client_class.h"
#include "icliententity.h"

#include "common.h"
#include "exceptions.h"
#include "ifaces.h"

std::unordered_map<std::string, std::unordered_map<std::string, int>> Entities::classPropOffsets;

bool Entities::RetrieveClassPropOffset(std::string className, std::vector<std::string> propertyTree) {
	std::string propertyString = ConvertTreeToString(propertyTree);

	if (classPropOffsets[className].find(propertyString) != classPropOffsets[className].end()) {
		return true;
	}

	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();

	while (cc) {
		if (className.compare(cc->GetName()) == 0) {
			RecvTable *table = cc->m_pRecvTable;

			int offset = 0;
			RecvProp *prop = nullptr;

			if (table) {
				for (std::string propertyName : propertyTree) {
					int subOffset = 0;

					if (prop && prop->GetType() == DPT_DataTable) {
						table = prop->GetDataTable();
					}

					if (!table) {
						return false;
					}

					if (GetSubProp(table, propertyName.c_str(), prop, subOffset)) {
						offset += subOffset;
					}
					else {
						return false;
					}

					table = nullptr;
				}

				classPropOffsets[className][propertyString] = offset;

				return true;
			}
		}
		cc = cc->m_pNext;
	}

	return false;
}

void *Entities::GetEntityProp(IClientEntity *entity, std::vector<std::string> propertyTree) {
	std::string className = entity->GetClientClass()->GetName();

	if (!RetrieveClassPropOffset(className, propertyTree)) {
		throw invalid_class_prop(className.c_str());
	}

	std::string propertyString = ConvertTreeToString(propertyTree);

	return (void *)((unsigned long)(entity)+(unsigned long)(classPropOffsets[className][propertyString]));
}

bool Entities::GetSubProp(RecvTable *table, const char *propName, RecvProp *&prop, int &offset) {
	for (int i = 0; i < table->GetNumProps(); i++) {
		offset = 0;

		RecvProp *currentProp = table->GetProp(i);

		if (strcmp(currentProp->GetName(), propName) == 0) {
			prop = currentProp;
			offset = currentProp->GetOffset();
			return true;
		}

		if (currentProp->GetType() == DPT_DataTable) {
			if (GetSubProp(currentProp->GetDataTable(), propName, prop, offset)) {
				offset += currentProp->GetOffset();
				return true;
			}
		}
	}

	return false;
}

bool Entities::CheckEntityBaseclass(IClientEntity *entity, std::string baseclass) {
	ClientClass *clientClass = entity->GetClientClass();

	if (clientClass) {
		return CheckClassBaseclass(clientClass, baseclass);
	}

	return false;
}

bool Entities::CheckClassBaseclass(ClientClass *clientClass, std::string baseclass) {
	RecvTable *sTable = clientClass->m_pRecvTable;

	if (sTable) {
		return CheckTableBaseclass(sTable, baseclass);
	}

	return false;
}

bool Entities::CheckTableBaseclass(RecvTable *sTable, std::string baseclass) {
	if (std::string(sTable->GetName()).compare("DT_" + baseclass) == 0) {
		return true;
	}

	for (int i = 0; i < sTable->GetNumProps(); i++) {
		RecvProp *sProp = sTable->GetProp(i);

		if (strcmp(sProp->GetName(), "baseclass") != 0) {
			continue;
		}

		RecvTable *sChildTable = sProp->GetDataTable();
		if (sChildTable) {
			return CheckTableBaseclass(sChildTable, baseclass);
		}
	}

	return false;
}