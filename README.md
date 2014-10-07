StatusSpec
==========

a Team Fortress 2 client plugin that augments game spectating

![screenshot](http://i.imgur.com/HliUKLk.png)

Changelog
---------

**0.20.0**
* camera tools
  * new module
* FOV override
  * new module
* killstreaks
  * added functionality to track total kills in killfeed
  * prevent double tracking of kills in local servers
* player outlines
  * fix health-based team-colored outlines
* spec GUI order
  * added reverse ordering of players

**0.19.2**
* general
  * fixed crashes

**0.19.1**
* custom materials
  * removed module
* custom textures
  * new module
* spec GUI order
  * new module

**0.19.0**
* custom materials
  * new module
* player aliases
  * add support for ESEA aliases
  * add support for formatting names with prefixes and suffixes
  * remove support for STEAM_ format (only 64-bit IDs accepted now)
* player outlines
  * added fading based on distance
* projectile outlines
  * added fading based on distance
* player models
  * new module
* team overrides
  * new module

**0.18.0**
* general
  * added plugin glow system
  * fix issues caused by the 2014-09-10 update
* player outlines
  * migrated to plugin glow system
  * removed useless commands
* projectile outlines
  * new module

**0.17.4**
* general
  * adjust paint method hooking so that we can draw at the correct location

**0.17.3**
* general
  * update offsets for 2014-07-30 update

**0.17.2**
* general
  * updated version of MinHook and changed build procedures

**0.17.1**
* medigun info
  * fix issues with charge advantage info (thanks Bluee/MattMcNam)
* player aliases
  * add Twitch API support

**0.17.0**
* player aliases
  * add ETF2L API support

**0.16.0**
* loadout icons
  * add ability to only show the active weapon
* local player
  * add command to track spectator target
  * fix warning being emitted on successful changes
* medigun info
  * change how progress bars are set (requires changes to MedigunInfo.res)
  * add ability to have custom meter settings based on medigun type

**0.15.0**
* antifreeze
  * now supports displaying a message when a freeze is occurring
* killstreaks
  * expand killstreak display to the scoreboard
  * fix killstreaks causing post-game crashes
* local player
  * new module - allows a certain player's actions to be highlighted by the killfeed
* multipanel
  * new module - allows the console and scoreboard to be opened on top of the game
* player outlines
  * make player outline calculations more efficient
  * add option to disable health-based calculations

**0.14.1**
* fix glitches with killstreaks on death
* add command to help stop flickering issues with player outlines

**0.14.0**
* add killstreaks module

**0.13.1**
* fix bleed icons not showing up

**0.13.0**
* add support for health-based team colored outlines

**0.12.1**
* fix medigun info not disappearing properly
* make unloading/pausing/unpausing cleaner and hopefully less error-prone

**0.12.0**
* add support for HUD features required for casting HUDs
* add ability to set progress direction via resource file and removed command
* add setting to allow Vaccinator to have only one charge meter
* add ability to view medigun charge advantage
* fix issues with medigun finding

**0.11.3**
* fix issues with player outlines not changing color after reloads

**0.11.2**
* fix player outline issues with players not being hooked/unhooked properly

**0.11.1**
* update offsets for TF2 update on 2014-06-11

**0.11.0**
* reorganize entire plugin into modules (changing many console variables and commands)
* add player outlines module

**0.10.0**
* fix player alias incompatibility with Spec Tools
* add utility command for setting the direction of a progress bar

**0.9.0**
* implement medigun info box

**0.8.0**
* implement player aliasing

**0.7.0**
* implement loadout icons

**0.6.0**
* implement status icons as part of spectator GUI

**0.5.0**
* added option to force refreshing of spectator GUI (partially to fully fixes post-pause issues, YMMV)

**0.4.0**
* added option (enabled by default) to have icon backgrounds adjust with icons rather than be static

**0.3.0**
* scale HUD positioning values automatically

**0.2.0**
* added Vaccinator resistances/Ubers, Soldier banners, Jarate, Mad Milk, bleeding, and on fire icons to display

**0.1.0**
* initial version
* Uber, Kritz, marked for death status icons
* many CVars for customizing appearance

Usage
-----
To install, place the `StatusSpec` folder within the `custom` folder in the `tf` directory. Because this plugin is not signed by Valve, you must launch TF2 with the `-insecure` option in order for this plugin to be active and will not be able to use this plugin on VAC-secured servers.

### AntiFreeze
*forces spectator GUI to refresh constantly, eliminating many HUD issues after game unpauses*

#### Console Variables
* `statusspec_antifreeze_display` - enables display of an info panel when a freeze is detected
* `statusspec_antifreeze_display_threshold` - the time of a freeze (in seconds) before the info panel is displayed
* `statusspec_antifreeze_enabled` - enable antifreeze (forces the spectator GUI to refresh)

#### Console Commands
* `statusspec_antifreeze_display_reload_settings` - reload settings for the freeze info panel from the resource file

#### UI Resource Files
An example file for the freeze info box is included under `Resource/UI/FreezeInfo.res`. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_antifreeze_display_reload_settings` is provided as a replacement.

### Camera Tools
*allows more options for managing the camera*

#### Console Commands
* `statusspec_cameratools_spec_player <team> <position>` - spec a certain player
* `statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>` - spec a certain camera position

### Custom Textures
*allows textures to be swapped out*

#### Console Variables
* `statusspec_customtextures_enabled` - enable custom textures

#### Console Commands
* `statusspec_customtextures_load_replacement_group <group>` - load a texture replacement group
* `statusspec_customtextures_unload_replacement_group <group>` - unload a texture replacement group

#### Resource Files
Player model configuration is loaded from the `Resource/CustomTextures.res` file. Replacement groups should be configured as sections (whose names are used in the commands for this module). Material replacements should be specified by entries with the path of the old texture from the `materials` folder as the key and the path of the new texture from the `materials` folder as the value. An example of a configured file is given below:
```
"textures"
{
	"blank"
	{
		"signs/team_blue"	"signs/team_blue_blank"
		"signs/team_red"	"signs/team_red_blank"
	}
}
```

### FOV Override
*forces the FOV used to be consistent even if players have different FOVs*

#### Console Variables
* `statusspec_fovoverride_enabled` - enable FOV override
* `statusspec_fovoverride_fov` - the FOV value used
* `statusspec_fovoverride_zoomed` - enable FOV override even when sniper rifle is zoomed

### Killstreaks
*enables killstreak tracking for all weapons, all players*

#### Console Variables
* `statusspec_killstreaks_enabled` - enable killstreaks display
* `statusspec_killstreaks_total_killfeed` - display total kills for player in killfeed instead of only kills with single weapon

### Loadout Icons
*displays icons to represent weapons in a player's loadout*

#### Console Variables
* `statusspec_loadouticons_enabled` - enable loadout icons
* `statusspec_loadouticons_nonloadout` - enable loadout icons for nonloadout items
* `statusspec_loadouticons_only_active` - only display loadout icons for the active weapon

#### Console Commands
* `statusspec_loadouticons_filter_active <red> <green> <blue> <alpha>` - the RGBA filter applied to the icon for an active item
* `statusspec_loadouticons_filter_nonactive <red> <green> <blue> <alpha>` - the RGBA filter applied to the icon for a nonactive item

#### UI Resource Files
To properly support loadout icons, adjust `Resource/UI/SpectatorTournament.res` so that it includes something similar to the `loadouticons` section below contained within the `playerpanels_kv` section of the `specgui` section, as demonstrated below. You will also need to adjust the rest of the file as necessary, since this will act as a normal GUI panel. The icons will be entirely contained within this element as defined in this configuration, and each icon will be a square size of `tall`. The rendering of this panel only displays icons and will not adjust anything else, no matter whether this feature is enabled or disabled - thus, you will be required to manually adjust your HUD as appropriate when enabling or disabling this feature.
```
"Resource/UI/SpectatorTournament.res"
{
	"specgui"
	{
		...
	       
		"playerpanels_kv"
		{
			...
			
			"loadouticons"
			{
				"ControlName"	"Panel"
				"fieldName"		"loadouticons"
				"xpos"			"16"
				"ypos"			"0"
				"zpos"			"2"
				"wide"			"60"
				"tall"			"15"
				"visible"		"1"
				"enabled"		"1"
			}
		}
	}
	
	...	
}
```

### Local Player
*changes the local player locally to affect certain HUD elements like the killfeed*

#### Console Variables
* `statusspec_localplayer_enabled` - enable local player override
* `statusspec_localplayer_player` - player to set as the local player

#### Console Commands
* `statusspec_localplayer_set_current_target` - set the local player to the current spectator target

### Medigun Info
*displays info about mediguns for both teams*

#### Console Variables
* `statusspec_mediguninfo_dynamic_meters` - enable charge meters to change based on medigun
* `statusspec_mediguninfo_enabled` - enable medigun info
* `statusspec_mediguninfo_individual_charge_meters` - enable individual charge meters (for Vaccinator)

#### Console Commands
* `statusspec_mediguninfo_reload_settings` - reload settings for the medigun info HUD from the resource file

#### UI Resource Files
An example file for the medigun info box is included under `Resource/UI/MedigunInfo.res`. In addition, the `Resource/UI/MedigunInfoDynamicMeters.res` file is used for the dynamic meters feature, and contains empty sections for each medigun within which you can add progress bar options that will be applied when the specified medigun is being used. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_mediguninfo_reload_settings` is provided as a replacement.

In addition, the following HUD animations are triggered by this plugin and may be used to show events on the HUD:
* `MedigunInfoBluChargeReady` - triggered when a BLU medigun charge is ready for use
* `MedigunInfoBluChargeReleased` - triggered when a BLU medigun charge has been released/popped
* `MedigunInfoBluChargeStop` - triggered when the BLU medigun charge is no longer ready/released
* `MedigunInfoRedChargeReady` - triggered when a RED medigun charge is ready for use
* `MedigunInfoRedChargeReleased` - triggered when a RED medigun charge has been released/popped
* `MedigunInfoRedChargeStop` - triggered when the RED medigun charge is no longer ready/released (after a charge is completed/dropped)

### Multipanel
*displays panels on top of the main game*

#### Console Variables
* `statusspec_multipanel_console` - displays a console in the HUD
* `statusspec_multipanel_scoreboard` - displays the scoreboard

### Player Aliases
*substitutes in user-defined aliases for player names*

#### Console Variables
* `statusspec_playeraliases_enabled` - enable player aliases
* `statusspec_playeraliases_esea` - enable player aliases from the ESEA API
* `statusspec_playeraliases_etf2l` - enable player aliases from the ETF2L API
* `statusspec_playeraliases_format_blu` - the name format for BLU players
* `statusspec_playeraliases_format_red` - the name format for RED players
* `statusspec_playeraliases_twitch` - enable player aliases from the Twitch API

#### Console Commands
* `statusspec_playeraliases_get <steamid>` - get an alias for a player
* `statusspec_playeraliases_remove <steamid>` - remove an alias for a player
* `statusspec_playeraliases_set <steamid> <alias>` - set an alias for a player
* `statusspec_playeraliases_switch_teams` - switch name formats for both teams

### Player Models
*changes the model used based on the player*

#### Console Variables
* `statusspec_playermodels_enabled` - enable custom player models

#### Resource Files
Player model configuration is loaded from the `Resource/PlayerModels.res` file. Within the `players` section, players should be configured with an entry consisting of their Steam ID as the key and a group name from the `groups` section as the value. Within the `groups` section, each group should have a section (named with the group name specified in the `players` section above), and model replacements should be specified by entries with the path of the old model from the `models` folder as the key and the path of the new model from the `models` folder as the value. An example of a configured file is given below:
```
"models"
{
	"players"
	{
		"76561197970669109"	"demo-at-heart"
	}
	"groups"
	{
		"demo-at-heart"
		{
			"player/scout.mdl"		"player/demo.mdl"
			"player/soldier.mdl"	"player/demo.mdl"
			"player/pyro.mdl"		"player/demo.mdl"
			"player/demo.mdl"		"player/demo.mdl"
			"player/heavy.mdl"		"player/demo.mdl"
			"player/engineer.mdl"	"player/demo.mdl"
			"player/medic.mdl"		"player/demo.mdl"
			"player/sniper.mdl"		"player/demo.mdl"
			"player/spy.mdl"		"player/demo.mdl"
		}
	}
}
```

### Player Outlines
*displays bright outlines around players that can be seen through walls*

#### Console Variables
* `statusspec_playeroutlines_enabled` - enable player outlines
* `statusspec_playeroutlines_fade` - make outlines fade with distance
* `statusspec_playeroutlines_fade_distance` - the distance (in Hammer units) at which outlines will fade
* `statusspec_playeroutlines_health_adjusted_team_colors` - adjusts team colors depending on health of players
* `statusspec_playeroutlines_team_colors` - override default health-based outline colors with team colors

#### Console Commands
* `statusspec_playeroutlines_color_blu_buff <red> <green> <blue> [alpha]` - the color used for outlines for BLU team players at max buffed health
* `statusspec_playeroutlines_color_blu_full <red> <green> <blue> [alpha]` - the color used for outlines for BLU team players at full health
* `statusspec_playeroutlines_color_blu_low <red> <green> <blue> [alpha]` - the color used for outlines for BLU team players at low health
* `statusspec_playeroutlines_color_blu_medium <red> <green> <blue> [alpha]` - the color used for outlines for BLU team players at medium health
* `statusspec_playeroutlines_color_red_buff <red> <green> <blue> [alpha]` - the color used for outlines for RED team players at max buffed health
* `statusspec_playeroutlines_color_red_full <red> <green> <blue> [alpha]` - the color used for outlines for RED team players at full health
* `statusspec_playeroutlines_color_red_low <red> <green> <blue> [alpha]` - the color used for outlines for RED team players at low health
* `statusspec_playeroutlines_color_red_medium <red> <green> <blue> [alpha]` - the color used for outlines for RED team players at medium health

### Projectile Outlines
*displays bright outlines around projectiles that can be seen through walls*

#### Console Variables
* `statusspec_projectileoutlines_enabled` - enable projectile outlines
* `statusspec_projectileoutlines_fade` - make outlines fade with distance
* `statusspec_projectileoutlines_fade_distance` - the distance (in Hammer units) at which outlines will fade
* `statusspec_projectileoutlines_grenades` - enable outlines for grenades
* `statusspec_projectileoutlines_rockets` - enable outlines for rockets
* `statusspec_projectileoutlines_stickybombs` - enable outlines for stickybombs

#### Console Commands
* `statusspec_projectileoutlines_color_blu <red> <green> <blue> [alpha]` - the color used for outlines for BLU projectiles
* `statusspec_projectileoutlines_color_red <red> <green> <blue> [alpha]` - the color used for outlines for RED projectiles

### Spec GUI Order
*orders the spec GUI more logically than player indices*

#### Console Variables
* `statusspec_specguiorder_enabled` - enable ordering of spec GUI
* `statusspec_specguiorder_reverse_blu` - reverse order for BLU players
* `statusspec_specguiorder_reverse_red` - reverse order for RED players

### Status Icons
*displays icons representing special statuses of a player*

#### Console Variables
* `statusspec_statusicons_enabled` - enable status icons
* `statusspec_statusicons_max_icons` - maximum number of icons to show

#### UI Resource Files
To properly support status icons, adjust `Resource/UI/SpectatorTournament.res` so that it includes something similar to the `statusicons` section below contained within the `playerpanels_kv` section of the `specgui` section, as demonstrated below. The icons will appear on the right of `xpos` with a square size of `tall`, and the player panel will also be expanded to accommodate these icons. The rendering of this panel assumes that this element is on the right edge of the player panel and will not appear if status icons are disabled.
```
"Resource/UI/SpectatorTournament.res"
{
	"specgui"
	{
		...
	       
		"playerpanels_kv"
		{
			...
			
			"statusicons"
			{
				"ControlName"	"Panel"
				"fieldName"		"statusicons"
				"xpos"			"160"
				"ypos"			"0"
				"zpos"			"8"
				"wide"			"0"
				"tall"			"15"
				"visible"		"1"
				"enabled"		"1"
			}
		}
	}
	
	...	
}
```

### Team Overrides
*override team info in HUD*

#### Console Variables
* `statusspec_teamoverrides_enabled` - enable team overrides
* `statusspec_teamoverrides_name_blu` - BLU team name
* `statusspec_teamoverrides_name_red` - RED team name
* `statusspec_teamoverrides_names` - enable overrides for team names
* `statusspec_teamoverrides_score_blu` - BLU team score
* `statusspec_teamoverrides_score_red` - RED team score
* `statusspec_teamoverrides_scores` - enable overrides for team scores

#### Console Commands
* `statusspec_teamoverrides_switch_teams` - switch names and scores for both teams

Thanks
------
**bluee (MattMcNam)** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**AlliedModders** - for in-depth information about the internals of the Source Engine and for [SourceHook](https://github.com/alliedmodders/metamod-source/tree/master/core/sourcehook)

**Tsuda Kageyu and other contributors** - for [MinHook](https://github.com/TsudaKageyu/minhook)