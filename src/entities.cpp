/*
 *  entities.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "entities.h"

std::map<ClassPropDefinition, int> Entities::classPropOffsets;

bool Entities::RetrieveClassPropOffset(std::string className, std::vector<std::string> propertyTree) {
	ClassPropDefinition classPropDefinition;
	classPropDefinition.className = className;
	classPropDefinition.propertyTree = propertyTree;

	if (classPropOffsets.find(classPropDefinition) != classPropOffsets.end()) {
		return true;
	}

	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();

	while (cc) {
		if (className.compare(cc->GetName()) == 0) {
			RecvTable *table = cc->m_pRecvTable;

			int offset = 0;
			RecvProp *prop = nullptr;

			if (table) {
				for (auto iterator = propertyTree.begin(); iterator != propertyTree.end(); ++iterator) {
					int subOffset = 0;

					if (prop && prop->GetType() == DPT_DataTable) {
						table = prop->GetDataTable();
					}

					if (!table) {
						return false;
					}

					if (GetSubProp(table, iterator->c_str(), prop, subOffset)) {
						offset += subOffset;
					}
					else {
						return false;
					}

					table = nullptr;
				}

				classPropOffsets[classPropDefinition] = offset;

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

	ClassPropDefinition classPropDefinition;
	classPropDefinition.className = className;
	classPropDefinition.propertyTree = propertyTree;

	return (void *)((unsigned long)(entity)+(unsigned long)(classPropOffsets[classPropDefinition]));
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
	std::string name = sTable->GetName();
	if (name.find(baseclass) != name.npos) {
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