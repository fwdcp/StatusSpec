/*
 *  consoletools.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "consoletools.h"

#include <regex>

#include "convar.h"
#include "icvar.h"

#include "../funcs.h"

// this is a total hijack of a friend class declared but never defined in the public SDK
class CCvar {
public:
	static int GetFlags(ConCommandBase *base);
	static void SetFlags(ConCommandBase *base, int flags);
};

int CCvar::GetFlags(ConCommandBase *base) {
	return base->m_nFlags;
}

void CCvar::SetFlags(ConCommandBase *base, int flags) {
	base->m_nFlags = flags;
}

ConsoleTools::ConsoleTools() {
	consoleColorPrintfHook = 0;
	consoleDPrintfHook = 0;
	consolePrintfHook = 0;

	filter_add = new ConCommand("statusspec_consoletools_filter_add", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleTools>()->AddFilter(command); }, "add a console filter", FCVAR_NONE);
	filter_enabled = new ConVar("statusspec_consoletools_filter_enabled", "0", FCVAR_NONE, "enable console filtering", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<ConsoleTools>()->ToggleFilterEnabled(var, pOldValue, flOldValue); });
	filter_remove = new ConCommand("statusspec_consoletools_filter_remove", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleTools>()->RemoveFilter(command); }, "remove a console filter", FCVAR_NONE);
	flags_add = new ConCommand("statusspec_consoletools_flags_add", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleTools>()->AddFlags(command); }, "add a flag to a console command or variable", FCVAR_NONE);
	flags_remove = new ConCommand("statusspec_consoletools_flags_remove", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleTools>()->RemoveFlags(command); }, "remove a flag from a console command or variable", FCVAR_NONE);
}

bool ConsoleTools::CheckDependencies() {
	bool ready = true;

	if (!g_pCVar) {
		PRINT_TAG();
		Warning("Required interface ICVar for module %s not available!\n", g_ModuleManager->GetModuleName<ConsoleTools>().c_str());

		ready = false;
	}

	return ready;
}

void ConsoleTools::ConsoleColorPrintfHook(const Color &clr, const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void ConsoleTools::ConsoleDPrintfHook(const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void ConsoleTools::ConsolePrintfHook(const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

bool ConsoleTools::CheckFilters(std::string message) {
	for (std::string filter : filters) {
		if (std::regex_search(message, std::regex(filter))) {
			return true;
		}
	}

	return false;
}

void ConsoleTools::AddFilter(const CCommand &command) {
	if (command.ArgC() >= 2) {
		std::string filter = command.Arg(1);

		if (!filters.count(filter)) {
			filters.insert(filter);
		}
		else {
			Warning("Filter %s is already present.\n", command.Arg(1));
		}
	}
	else {
		Warning("Usage: statusspec_consoletools_filter_add <filter>\n");
	}
}

void ConsoleTools::AddFlags(const CCommand &command) {
	if (command.ArgC() >= 3) {
		const char *name = command.Arg(1);

		ConCommandBase *base = g_pCVar->FindCommandBase(name);

		if (base) {
			for (int i = 2; i < command.ArgC(); i++) {
				std::string flag = command.Arg(i);

				if (flag.compare("game") == 0) {
					base->AddFlags(FCVAR_GAMEDLL);
				}
				else if (flag.compare("client") == 0) {
					base->AddFlags(FCVAR_CLIENTDLL);
				}
				else if (flag.compare("archive") == 0) {
					base->AddFlags(FCVAR_ARCHIVE);
				}
				else if (flag.compare("notify") == 0) {
					base->AddFlags(FCVAR_NOTIFY);
				}
				else if (flag.compare("singleplayer") == 0) {
					base->AddFlags(FCVAR_SPONLY);
				}
				else if (flag.compare("notconnected") == 0) {
					base->AddFlags(FCVAR_NOT_CONNECTED);
				}
				else if (flag.compare("cheat") == 0) {
					base->AddFlags(FCVAR_CHEAT);
				}
				else if (flag.compare("replicated") == 0) {
					base->AddFlags(FCVAR_REPLICATED);
				}
				else if (flag.compare("server_can_execute") == 0) {
					base->AddFlags(FCVAR_SERVER_CAN_EXECUTE);
				}
				else if (flag.compare("clientcmd_can_execute") == 0) {
					base->AddFlags(FCVAR_CLIENTCMD_CAN_EXECUTE);
				}
				else if (IsInteger(flag)) {
					base->AddFlags(1 << atoi(flag.c_str()));
				}
				else {
					Warning("Unrecognized flag %s!\n", command.Arg(i));
				}
			}
		}
		else {
			Warning("%s is not a valid command or variable!\n", command.Arg(1));
		}
	}
	else {
		Warning("Usage: statusspec_consoletools_flags_add <name> <flag1> [flag2 ...]\n");
	}
}

void ConsoleTools::RemoveFilter(const CCommand &command) {
	if (command.ArgC() >= 2) {
		std::string filter = command.Arg(1);

		if (filters.count(filter)) {
			filters.erase(filter);
		}
		else {
			Warning("Filter %s is not already present.\n", command.Arg(1));
		}
	}
	else {
		Warning("Usage: statusspec_consoletools_filter_remove <filter>\n");
	}
}

void ConsoleTools::RemoveFlags(const CCommand &command) {
	if (command.ArgC() >= 3) {
		const char *name = command.Arg(1);

		ConCommandBase *base = g_pCVar->FindCommandBase(name);

		if (base) {
			int flags = CCvar::GetFlags(base);

			for (int i = 2; i < command.ArgC(); i++) {
				std::string flag = command.Arg(i);

				if (flag.compare("game") == 0) {
					flags &= ~(FCVAR_GAMEDLL);
				}
				else if (flag.compare("client") == 0) {
					flags &= ~(FCVAR_CLIENTDLL);
				}
				else if (flag.compare("archive") == 0) {
					flags &= ~(FCVAR_ARCHIVE);
				}
				else if (flag.compare("notify") == 0) {
					flags &= ~(FCVAR_NOTIFY);
				}
				else if (flag.compare("singleplayer") == 0) {
					flags &= ~(FCVAR_SPONLY);
				}
				else if (flag.compare("notconnected") == 0) {
					flags &= ~(FCVAR_NOT_CONNECTED);
				}
				else if (flag.compare("cheat") == 0) {
					flags &= ~(FCVAR_CHEAT);
				}
				else if (flag.compare("replicated") == 0) {
					flags &= ~(FCVAR_REPLICATED);
				}
				else if (flag.compare("server_can_execute") == 0) {
					flags &= ~(FCVAR_SERVER_CAN_EXECUTE);
				}
				else if (flag.compare("clientcmd_can_execute") == 0) {
					flags &= ~(FCVAR_CLIENTCMD_CAN_EXECUTE);
				}
				else if (IsInteger(flag)) {
					flags &= ~(1 << atoi(flag.c_str()));
				}
				else {
					Warning("Unrecognized flag %s!\n", command.Arg(i));
				}
			}

			CCvar::SetFlags(base, flags);
		}
		else {
			Warning("%s is not a valid command or variable!\n", command.Arg(1));
		}
	}
	else {
		Warning("Usage: statusspec_consoletools_flags_remove <name> <flag1> [flag2 ...]\n");
	}
}

void ConsoleTools::ToggleFilterEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (filter_enabled->GetBool()) {
		if (!consoleColorPrintfHook) {
			consoleColorPrintfHook = Funcs::AddHook_ICvar_ConsoleColorPrintf(g_pCVar, SH_MEMBER(this, &ConsoleTools::ConsoleColorPrintfHook), false);
		}

		if (!consoleDPrintfHook) {
			consoleDPrintfHook = Funcs::AddHook_ICvar_ConsoleDPrintf(g_pCVar, SH_MEMBER(this, &ConsoleTools::ConsoleDPrintfHook), false);
		}
		
		if (!consolePrintfHook) {
			consolePrintfHook = Funcs::AddHook_ICvar_ConsolePrintf(g_pCVar, SH_MEMBER(this, &ConsoleTools::ConsolePrintfHook), false);
		}
	}
	else {
		if (consoleColorPrintfHook) {
			if (Funcs::RemoveHook(consoleColorPrintfHook)) {
				consoleColorPrintfHook = 0;
			}
		}

		if (consoleDPrintfHook) {
			if (Funcs::RemoveHook(consoleDPrintfHook)) {
				consoleDPrintfHook = 0;
			}
		}

		if (consolePrintfHook) {
			if (Funcs::RemoveHook(consolePrintfHook)) {
				consolePrintfHook = 0;
			}
		}
	}
}