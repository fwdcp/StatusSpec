/*
 *  consolemanager.h
 *  StatusSpec project
 *
 *  Copyright (c) 2015 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include <set>

#include "../common.h"
#include "../modules.h"

class CCommand;
class ConCommand;
class ConVar;
class IConVar;

class ConsoleManager : public Module {
public:
	ConsoleManager(std::string name);

	static bool CheckDependencies(std::string name);
private:
	void ConsoleColorPrintfHook(const Color &clr, const char *message);
	void ConsoleColorPrintfOverride(const Color &clr, const char *message);
	void ConsoleDPrintfHook(const char *message);
	void ConsoleDPrintfOverride(const char *message);
	void ConsolePrintfHook(const char *message);
	void ConsolePrintfOverride(const char *message);

	bool CheckFilters(std::string message);
	void LogMessage(std::string message);

	int consoleColorPrintfPostHook;
	int consoleColorPrintfPreHook;
	int consoleDPrintfPostHook;
	int consoleDPrintfPreHook;
	int consolePrintfPostHook;
	int consolePrintfPreHook;
	std::string currentLogFile;
	std::set<std::string> filters;

	ConCommand *filter_add;
	ConVar *filter_enabled;
	ConCommand *filter_remove;
	ConCommand *flags_add;
	ConCommand *flags_remove;
	ConVar *logging_enabled;
	void AddFilter(const CCommand &command);
	void AddFlags(const CCommand &command);
	void RemoveFilter(const CCommand &command);
	void RemoveFlags(const CCommand &command);
	void ToggleFilterEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleLoggingEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};