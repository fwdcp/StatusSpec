/*
 *  consolemanager.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2015 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "consolemanager.h"

#include <regex>

#include "convar.h"
#include "filesystem.h"
#include "icvar.h"
#include "utlbuffer.h"

#include "../funcs.h"
#include "../ifaces.h"

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

ConsoleManager::ConsoleManager(std::string name) : Module(name) {
	consoleColorPrintfPostHook = 0;
	consoleColorPrintfPreHook = 0;
	consoleDPrintfPostHook = 0;
	consoleDPrintfPreHook = 0;
	consolePrintfPostHook = 0;
	consolePrintfPreHook = 0;

	filter_add = new ConCommand("statusspec_consolemanager_filter_add", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->AddFilter(command); }, "add a console filter", FCVAR_NONE);
	filter_enabled = new ConVar("statusspec_consolemanager_filter_enabled", "0", FCVAR_NONE, "enable console filtering", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->ToggleFilterEnabled(var, pOldValue, flOldValue); });
	filter_remove = new ConCommand("statusspec_consolemanager_filter_remove", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->RemoveFilter(command); }, "remove a console filter", FCVAR_NONE);
	flags_add = new ConCommand("statusspec_consolemanager_flags_add", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->AddFlags(command); }, "add a flag to a console command or variable", FCVAR_NONE);
	flags_remove = new ConCommand("statusspec_consolemanager_flags_remove", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->RemoveFlags(command); }, "remove a flag from a console command or variable", FCVAR_NONE);
	logging_enabled = new ConVar("statusspec_consolemanager_logging_enabled", "0", FCVAR_NONE, "enable console logging to file", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->ToggleLoggingEnabled(var, pOldValue, flOldValue); });
}

bool ConsoleManager::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pCVar) {
		PRINT_TAG();
		Warning("Required interface ICVar for module %s not available!\n", name.c_str());

		ready = false;
	}

	return ready;
}

void ConsoleManager::ConsoleColorPrintfHook(const Color &clr, const char *message) {
	LogMessage(message);

	RETURN_META(MRES_IGNORED);
}

void ConsoleManager::ConsoleColorPrintfOverride(const Color &clr, const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void ConsoleManager::ConsoleDPrintfHook(const char *message) {
	LogMessage(message);

	RETURN_META(MRES_IGNORED);
}

void ConsoleManager::ConsoleDPrintfOverride(const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void ConsoleManager::ConsolePrintfHook(const char *message) {
	LogMessage(message);

	RETURN_META(MRES_IGNORED);
}

void ConsoleManager::ConsolePrintfOverride(const char *message) {
	if (CheckFilters(message)) {
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

bool ConsoleManager::CheckFilters(std::string message) {
	for (std::string filter : filters) {
		if (std::regex_search(message, std::regex(filter))) {
			return true;
		}
	}

	return false;
}

void ConsoleManager::LogMessage(std::string message) {
	CUtlBuffer buffer;
	buffer.PutString(message.c_str());
	size_t length = buffer.TellPut();

	void *memory = malloc(length);
	buffer = CUtlBuffer(memory, length);
	buffer.PutString(message.c_str());

	FSAsyncStatus_t status = Interfaces::pFileSystem->AsyncAppend(currentLogFile.c_str(), buffer.Base(), buffer.TellPut(), true);
}

void ConsoleManager::AddFlags(const CCommand &command) {
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
		Warning("Usage: statusspec_consolemanager_flags_add <name> <flag1> [flag2 ...]\n");
	}
}

void ConsoleManager::AddFilter(const CCommand &command) {
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
		Warning("Usage: statusspec_consolemanager_filter_add <filter>\n");
	}
}

void ConsoleManager::RemoveFlags(const CCommand &command) {
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
		Warning("Usage: statusspec_consolemanager_flags_remove <name> <flag1> [flag2 ...]\n");
	}
}

void ConsoleManager::RemoveFilter(const CCommand &command) {
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
		Warning("Usage: statusspec_consolemanager_filter_remove <filter>\n");
	}
}

void ConsoleManager::ToggleFilterEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (filter_enabled->GetBool()) {
		if (!consoleColorPrintfPreHook) {
			consoleColorPrintfPreHook = Funcs::AddHook_ICvar_ConsoleColorPrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsoleColorPrintfOverride), false);
		}

		if (!consoleDPrintfPreHook) {
			consoleDPrintfPreHook = Funcs::AddHook_ICvar_ConsoleDPrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsoleDPrintfOverride), false);
		}
		
		if (!consolePrintfPreHook) {
			consolePrintfPreHook = Funcs::AddHook_ICvar_ConsolePrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsolePrintfOverride), false);
		}
	}
	else {
		if (consoleColorPrintfPreHook) {
			if (Funcs::RemoveHook(consoleColorPrintfPreHook)) {
				consoleColorPrintfPreHook = 0;
			}
		}

		if (consoleDPrintfPreHook) {
			if (Funcs::RemoveHook(consoleDPrintfPreHook)) {
				consoleDPrintfPreHook = 0;
			}
		}

		if (consolePrintfPreHook) {
			if (Funcs::RemoveHook(consolePrintfPreHook)) {
				consolePrintfPreHook = 0;
			}
		}
	}
}

void ConsoleManager::ToggleLoggingEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (logging_enabled->GetBool()) {
		if (!consoleColorPrintfPostHook) {
			consoleColorPrintfPostHook = Funcs::AddHook_ICvar_ConsoleColorPrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsoleColorPrintfHook), true);
		}

		if (!consoleDPrintfPostHook) {
			consoleDPrintfPostHook = Funcs::AddHook_ICvar_ConsoleDPrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsoleDPrintfHook), true);
		}

		if (!consolePrintfPostHook) {
			consolePrintfPostHook = Funcs::AddHook_ICvar_ConsolePrintf(g_pCVar, SH_MEMBER(this, &ConsoleManager::ConsolePrintfHook), true);
		}

		Interfaces::pFileSystem->CreateDirHierarchy("logs", "MOD");

		char logName[64];
		time_t currentTime = time(nullptr);
		struct tm *currentLocalTime = localtime(&currentTime);
		strftime(logName, sizeof(logName), "logs/clientconsole-%Y-%m-%d-%H-%M-%S.log", currentLocalTime);

		currentLogFile = std::string(logName);
	}
	else {
		if (consoleColorPrintfPostHook) {
			if (Funcs::RemoveHook(consoleColorPrintfPostHook)) {
				consoleColorPrintfPostHook = 0;
			}
		}

		if (consoleDPrintfPostHook) {
			if (Funcs::RemoveHook(consoleDPrintfPostHook)) {
				consoleDPrintfPostHook = 0;
			}
		}

		if (consolePrintfPostHook) {
			if (Funcs::RemoveHook(consolePrintfPostHook)) {
				consolePrintfPostHook = 0;
			}
		}
	}
}