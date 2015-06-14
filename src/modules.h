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

#include "exceptions.h"

class Module {
public:
	virtual ~Module() = default;

	static bool CheckDependencies(std::string name) { return true; };
protected:
	Module(std::string name) : name(name) {};

	std::string name;
};

class ModuleManager {
public:
	template <typename ModuleType> bool LoadModule(std::string moduleName);
	template <typename ModuleType> ModuleType *GetModule(std::string moduleName);
	void UnloadAllModules();
private:
	std::map<std::string, Module *> modules;
};

template <typename ModuleType> inline bool ModuleManager::LoadModule(std::string moduleName) {
	if (ModuleType::CheckDependencies(moduleName)) {
		modules[moduleName] = new ModuleType(moduleName);

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

template <typename ModuleType> inline ModuleType *ModuleManager::GetModule(std::string moduleName) {
	if (modules.find(moduleName) != modules.end()) {
		return dynamic_cast<ModuleType *>(modules.find(moduleName)->second);
	}
	else {
		throw module_not_loaded(moduleName.c_str());
	}
}

extern ModuleManager *g_ModuleManager;