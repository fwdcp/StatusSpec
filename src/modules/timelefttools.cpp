/*
*  timelefttools.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "timelefttools.h"

#include <iomanip>
#include <sstream>

#include "cbase.h"
#include "convar.h"
#include "KeyValues.h"
#include "steam/steamclientpublic.h"
#include "teamplayroundbased_gamerules.h"
#include "tier3/tier3.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class TimeleftTools::TeamplayRoundBasedRulesOverride : public C_TeamplayRoundBasedRules {
public:
	using C_TeamplayRoundBasedRules::m_flMapResetTime;
};

TimeleftTools::TimeleftTools(std::string name) : Module(name) {
	sendMessageHook = 0;

	count_up = new ConVar("statusspec_timelefttools_count_up", "0", FCVAR_NONE, "count up instead of down");
	enabled = new ConVar("statusspec_timelefttools_enabled", "0", FCVAR_NONE, "enable timeleft tools", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<TimeleftTools>("Timeleft Tools")->ToggleEnabled(var, pOldValue, flOldValue); });
	timelimit = new ConVar("statusspec_timelefttools_timelimit", "0", FCVAR_NONE, "timelimit to use");
}

bool TimeleftTools::CheckDependencies(std::string name) {
	bool ready = true;

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetGlobalVars();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Required interface CGlobalVarsBase for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetTeamplayRoundBasedRules();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Module %s requires C_TeamplayRoundBasedRules, which cannot be verified at this time!\n", name.c_str());
	}

	return ready;
}

void TimeleftTools::SendMessageOverride(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	if (strcmp(params->GetName(), "DialogVariables") == 0) {
		try {
			float time;

			if (count_up->GetBool()) {
				time = Interfaces::GetGlobalVars()->curtime - ((TeamplayRoundBasedRulesOverride *)Interfaces::GetTeamplayRoundBasedRules())->m_flMapResetTime;
			}
			else if (timelimit->GetInt() == 0) {
				params->SetString("servertimeleft", "");

				RETURN_META(MRES_HANDLED);
			}
			else {
				time = ((TeamplayRoundBasedRulesOverride *)Interfaces::GetTeamplayRoundBasedRules())->m_flMapResetTime + (timelimit->GetInt() * 60) - Interfaces::GetGlobalVars()->curtime;

				if (time < 0) {
					time = 0;
				}
			}

			char *formattedTime = new char[16];

			if (int(time) / 3600 > 0) {
				V_snprintf(formattedTime, 15, "%i:%02i:%02i", int(time) / 3600, (int(time) / 60) % 60, int(time) % 60);
			}
			else {
				V_snprintf(formattedTime, 15, "%i:%02i", int(time) / 60, int(time) % 60);
			}

			params->SetString("servertimeleft", formattedTime);

			RETURN_META(MRES_HANDLED);
		}
		catch (bad_pointer &e) {
			params->SetString("servertimeleft", "");

			RETURN_META(MRES_HANDLED);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void TimeleftTools::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!sendMessageHook) {
			sendMessageHook = Funcs::AddHook_IPanel_SendMessage(g_pVGuiPanel, SH_MEMBER(this, &TimeleftTools::SendMessageOverride), false);
		}
	}
	else {
		if (sendMessageHook) {
			if (Funcs::RemoveHook(sendMessageHook)) {
				sendMessageHook = 0;
			}
		}
	}
}