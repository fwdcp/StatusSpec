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

#include "convar.h"
#include "icvar.h"

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
	flags_add = new ConCommand("statusspec_consolemanager_flags_add", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->AddFlags(command); }, "add a flag to a console command or variable", FCVAR_NONE);
	flags_remove = new ConCommand("statusspec_consolemanager_flags_remove", [](const CCommand &command) { g_ModuleManager->GetModule<ConsoleManager>("Console Manager")->RemoveFlags(command); }, "remove a flag from a console command or variable", FCVAR_NONE);
}

bool ConsoleManager::CheckDependencies(std::string name) {
	bool ready = true;

	if (!g_pCVar) {
		PRINT_TAG();
		Warning("Required interface ICVar for module %s not available!\n", name.c_str());

		ready = false;
	}

	return ready;
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
		Warning("Usage: statusspec_consolemanager_flags_add <name> <flag1> [flag2 ...]\n", command.Arg(0));
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
		Warning("Usage: statusspec_consolemanager_flags_remove <name> <flag1> [flag2 ...]\n", command.Arg(0));
	}
}