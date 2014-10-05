/*
 *  statusicons.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusicons.h"

StatusIcons::StatusIcons() {
	enabled = new ConVar("statusspec_statusicons_enabled", "0", FCVAR_NONE, "enable status icons");
	max_icons = new ConVar("statusspec_statusicons_max_icons", "5", FCVAR_NONE, "maximum number of icons to show");

	Paint::InitializeTexture(TEXTURE_NULL);
	Paint::InitializeTexture(TEXTURE_UBERCHARGE);
	Paint::InitializeTexture(TEXTURE_CRITBOOST);
	Paint::InitializeTexture(TEXTURE_MEGAHEALRED);
	Paint::InitializeTexture(TEXTURE_MEGAHEALBLU);
	Paint::InitializeTexture(TEXTURE_RESISTSHIELDRED);
	Paint::InitializeTexture(TEXTURE_RESISTSHIELDBLU);
	Paint::InitializeTexture(TEXTURE_BULLETRESISTRED);
	Paint::InitializeTexture(TEXTURE_BLASTRESISTRED);
	Paint::InitializeTexture(TEXTURE_FIRERESISTRED);
	Paint::InitializeTexture(TEXTURE_BULLETRESISTBLU);
	Paint::InitializeTexture(TEXTURE_BLASTRESISTBLU);
	Paint::InitializeTexture(TEXTURE_FIRERESISTBLU);
	Paint::InitializeTexture(TEXTURE_BUFFBANNERRED);
	Paint::InitializeTexture(TEXTURE_BUFFBANNERBLU);
	Paint::InitializeTexture(TEXTURE_BATTALIONSBACKUPRED);
	Paint::InitializeTexture(TEXTURE_BATTALIONSBACKUPBLU);
	Paint::InitializeTexture(TEXTURE_CONCHERORRED);
	Paint::InitializeTexture(TEXTURE_CONCHERORBLU);
	Paint::InitializeTexture(TEXTURE_JARATE);
	Paint::InitializeTexture(TEXTURE_MADMILK);
	Paint::InitializeTexture(TEXTURE_MARKFORDEATH);
	Paint::InitializeTexture(TEXTURE_BLEEDING);
	Paint::InitializeTexture(TEXTURE_FIRE);
}

bool StatusIcons::IsEnabled() {
	return enabled->GetBool();
}

void StatusIcons::InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	std::string originPanelName = g_pVGuiPanel->GetName(ifromPanel);

	if (originPanelName.substr(0, 11).compare("playerpanel") == 0 && strcmp(params->GetName(), "DialogVariables") == 0) {
		const char *playerName = params->GetString("playername", NULL);
		
		if (playerName) {
			for (int i = 0; i <= MAX_PLAYERS; i++) {
				Player player = i;
			
				if (!player) {
					continue;
				}
			
				if (strcmp(playerName, player.GetName()) == 0) {
					playerPanels[originPanelName] = player;

					break;
				}
			}
		}
	}
}

void StatusIcons::NoPaint(vgui::VPANEL vguiPanel) {
	const char *panelName = g_pVGuiPanel->GetName(vguiPanel);
	
	if (strcmp(panelName, "statusicons") == 0) {
		vgui::VPANEL playerPanel = g_pVGuiPanel->GetParent(vguiPanel);
		int iconsWide, iconsTall, playerWide, playerTall;
		
		g_pVGuiPanel->GetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->GetSize(playerPanel, playerWide, playerTall);
		
		playerWide -= iconsWide;
		iconsWide -= iconsWide;
		
		g_pVGuiPanel->SetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->SetSize(playerPanel, playerWide, playerTall);
	}
}

void StatusIcons::Paint(vgui::VPANEL vguiPanel) {
	const char *panelName = g_pVGuiPanel->GetName(vguiPanel);
	
	if (strcmp(panelName, "statusicons") == 0) {
		vgui::VPANEL playerPanel = g_pVGuiPanel->GetParent(vguiPanel);
		int iconsWide, iconsTall, playerWide, playerTall;
		
		g_pVGuiPanel->GetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->GetSize(playerPanel, playerWide, playerTall);
		
		playerWide -= iconsWide;
		iconsWide -= iconsWide;
		
		int iconSize = iconsTall;
		int icons = 0;
		int maxIcons = max_icons->GetInt();
		
		g_pVGuiPanel->SetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->SetSize(playerPanel, playerWide, playerTall);
		
		const char *playerPanelName = g_pVGuiPanel->GetName(playerPanel);
		
		if (playerPanels.find(playerPanelName) == playerPanels.end()) {
			return;
		}
		
		Player player = playerPanels[playerPanelName];

		if (!player) {
			return;
		}

		TFTeam team = player.GetTeam();
		
		if (player.CheckCondition(TFCond_Ubercharged)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_UBERCHARGE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_Kritzkrieged)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_CRITBOOST, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_MegaHeal)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_MEGAHEALRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_MEGAHEALBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_UberBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BULLETRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BULLETRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (player.CheckCondition(TFCond_SmallBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BULLETRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BULLETRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_UberBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BLASTRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BLASTRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (player.CheckCondition(TFCond_SmallBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BLASTRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BLASTRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_UberFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_FIRERESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_FIRERESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (player.CheckCondition(TFCond_SmallFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_FIRERESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_FIRERESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_Buffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BUFFBANNERRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BUFFBANNERBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_DefenseBuffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BATTALIONSBACKUPRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BATTALIONSBACKUPBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_RegenBuffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_CONCHERORRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_CONCHERORBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_Jarated)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_JARATE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_Milked)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_MADMILK, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_MarkedForDeath) || player.CheckCondition(TFCond_MarkedForDeathSilent)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_MARKFORDEATH, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_Bleeding)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_BLEEDING, iconsWide, 0, iconSize, iconSize, Color(255, 0, 0, 255));
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (player.CheckCondition(TFCond_OnFire)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_FIRE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
	}
}