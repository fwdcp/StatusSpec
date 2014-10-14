/*
 *  gameconsoledialog.cpp
 *  StatusSpec project
 *  Derived from the Alien Swarm SDK
 *
 */

#pragma once

#include "vgui_controls/consoledialog.h"

class CGameConsoleDialog : public vgui::CConsoleDialog
{
	DECLARE_CLASS_SIMPLE(CGameConsoleDialog, vgui::CConsoleDialog);

public:
	CGameConsoleDialog();

private:
	MESSAGE_FUNC_CHARPTR(OnCommandSubmitted, "CommandSubmitted", command);
};
