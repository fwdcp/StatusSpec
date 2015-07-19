/*
 *  consoletools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
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

class ConsoleTools : public Module {
public:
	ConsoleTools();

	static bool CheckDependencies();
private:
	void ConsoleColorPrintfHook(const Color &clr, const char *message);
	void ConsoleDPrintfHook(const char *message);
	void ConsolePrintfHook(const char *message);

	bool CheckFilters(std::string message);

	int consoleColorPrintfHook;
	int consoleDPrintfHook;
	int consolePrintfHook;
	std::set<std::string> filters;

	ConCommand *filter_add;
	ConVar *filter_enabled;
	ConCommand *filter_remove;
	ConCommand *flags_add;
	ConCommand *flags_remove;
	void AddFilter(const CCommand &command);
	void AddFlags(const CCommand &command);
	void RemoveFilter(const CCommand &command);
	void RemoveFlags(const CCommand &command);
	void ToggleFilterEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};