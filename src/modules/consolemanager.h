/*
 *  consolemanager.h
 *  StatusSpec project
 *
 *  Copyright (c) 2015 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "../common.h"
#include "../modules.h"

class CCommand;
class ConCommand;

class ConsoleManager : public Module {
public:
	ConsoleManager(std::string name);

	static bool CheckDependencies(std::string name);
private:
	ConCommand *flags_add;
	ConCommand *flags_remove;
	void AddFlags(const CCommand &command);
	void RemoveFlags(const CCommand &command);
};