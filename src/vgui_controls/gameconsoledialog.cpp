/*
 *  gameconsoledialog.cpp
 *  StatusSpec project
 *  Derived from the Alien Swarm SDK
 *
 */

#include "GameConsoleDialog.h"

#include "cdll_int.h"

#include "../ifaces.h"

using namespace vgui;

CGameConsoleDialog::CGameConsoleDialog() : BaseClass(NULL, "GameConsole", false)
{
	SetScheme("SourceScheme");

	// because animations do NOT work, we have to disable them lest the panel completely disappears for us
	SetFadeEffectDisableOverride(true);

	AddActionSignalTarget(this);
}

void CGameConsoleDialog::OnCommandSubmitted(const char *pCommand)
{
	Interfaces::pEngineClient->ClientCmd_Unrestricted(pCommand);
}