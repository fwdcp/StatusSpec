/*
 *  module.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <map>
#include <string>
#include <typeindex>

#include "common.h"
#include "exceptions.h"

class Module {
public:
	virtual ~Module() = default;

	static bool CheckDependencies() { return true; };
};

class ModuleManager {
public:
	template <typename ModuleType> ModuleType *GetModule();
	template <typename ModuleType> std::string GetModuleName();
	template <typename ModuleType> bool RegisterAndLoadModule(std::string moduleName);
	void UnloadAllModules();
private:
	std::map<std::type_index, Module *> modules;
	std::map<std::type_index, std::string> moduleNames;
};

template <typename ModuleType> inline ModuleType *ModuleManager::GetModule() {
	if (modules.find(typeid(ModuleType)) != modules.end()) {
		return dynamic_cast<ModuleType *>(modules[typeid(ModuleType)]);
	}
	else {
		throw module_not_loaded(GetModuleName<ModuleType>().c_str());
	}
}

template <typename ModuleType> inline std::string ModuleManager::GetModuleName() {
	if (moduleNames.find(typeid(ModuleType)) != moduleNames.end()) {
		return moduleNames[typeid(ModuleType)];
	}
	else {
		return "[Unknown]";
	}
}

template <typename ModuleType> inline bool ModuleManager::RegisterAndLoadModule(std::string moduleName) {
	moduleNames[typeid(ModuleType)] = moduleName;

	if (ModuleType::CheckDependencies()) {
		modules[typeid(ModuleType)] = new ModuleType();

		PRINT_TAG();
		ConColorMsg(Color(0, 255, 0, 255), "Module %s loaded successfully!\n", moduleName.c_str());

		return true;
	}
	else {
		PRINT_TAG();
		ConColorMsg(Color(255, 0, 0, 255), "Module %s failed to load!\n", moduleName.c_str());

		return false;
	}
}

inline void ModuleManager::UnloadAllModules() {
	for (auto iterator : modules) {
		PRINT_TAG();
		ConColorMsg(Color(0, 255, 0, 255), "Module %s unloaded!\n", moduleNames[iterator.first].c_str());

		delete iterator.second;
	}

	modules.clear();
}

extern ModuleManager *g_ModuleManager;