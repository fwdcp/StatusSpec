StatusSpec
==========

a Team Fortress 2 client plugin that augments game spectating

Changelog
---------

### 0.25.1
* general
  * fixed crash related to custom variable label panel
* filesystem tools
  * new module
* medigun info
  * added support for Gun Mettle skins

### 0.25.0
* general
  * increased reliability
  * optimized performance and size
* antifreeze
  * optimized module
  * removed timer functionality
  * moved freeze info to separate module
* camera auto switch
  * created new module
  * added ability to automatically switch to killer on death (with configurable delay)
* camera smooths
  * created new module
  * added ability to automatically smooth transitions between camera views
* camera state
  * created new module
  * optimized camera state updates
  * fixed issues with switching camera state
* camera tools
  * added ability to force camera mode and target
  * moved camera state into separate module
* console tools
  * created new module
  * added ability to change CVar flags
  * added ability to filter console by regular expressions
* custom materials
  * added reload settings command
* custom models
  * added reload settings command
* custom textures
  * added reload settings command
* FOV override
  * optimized and generalized module
* freeze info
  * created new module
* killstreaks
  * optimized module
* loadout icons
  * removed module
* local player
  * optimized module
* medigun info
  * optimized performance
  * add support for multiple mediguns per team
  * add direct support for dynamically switching HUD appearance
  * revamped required HUD files
  * removed redundant CVars
  * removed advantage calculations
  * removed custom animations
* player aliases
  * removed API-based (ESEA/ETF2L/Twitch) aliases
* player models
  * documented previously added reload settings command
* player outlines
  * removed module
* projectile outlines
  * optimized module
  * fix color commands
  * fix grenades and stickybombs not being highlighted
* spec GUI order
  * removed module
* status icons
  * removed module
* Steam tools
  * created new module
  * added ability to set rich presence
* team health comparison
  * optimized performance
* team overrides
  * removed module

### 0.24.0
* general
  * fix issues with invalid models
  * fix compatibility with Source SDK
  * increase performance with exceptions
* camera tools
  * add command to spectate a player by index
  * add exposure of SourceTV camera information

### 0.23.1
* player aliases
  * fix infinite recursion

### 0.23.0
* general
  * major modularization - modules check dependencies and fail separately
  * revamp support structures
  * fix incompatibilities between modules
* killstreaks
  * fixed changes from recent updates
* loadout icons
  * fixed recent issues

### 0.22.3
* general
  * fix issues with multiple detours causing crashes

### 0.22.2
* player models
  * add functionality to replace ragdolls

### 0.22.1
* antifreeze
  * added feature to fix timer freezes

### 0.22.0
* general
  * fixed issues with outlines (hopefully)
* custom models
  * new module

### 0.21.0
* general
  * optimized plugin
* camera tools
  * player selection now based on spec GUI position (independent of spec GUI order module)
* custom materials
  * readded module
* custom textures
  * fixed commands not working
* loadout icons
  * revamped to use VGUI system
* status icons
  * revamped to use VGUI system
* team health comparison
  * new module

### 0.20.1
* general
  * fix crashing due to invalid entities
* camera tools
  * add command to prevent speccing dead players
* spec GUI order
  * fix gaps in between players

### 0.20.0
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

### 0.19.2
* general
  * fixed crashes

### 0.19.1
* custom materials
  * removed module
* custom textures
  * new module
* spec GUI order
  * new module

### 0.19.0
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

### 0.18.0
* general
  * added plugin glow system
  * fix issues caused by the 2014-09-10 update
* player outlines
  * migrated to plugin glow system
  * removed useless commands
* projectile outlines
  * new module

### 0.17.4
* general
  * adjust paint method hooking so that we can draw at the correct location

### 0.17.3
* general
  * update offsets for 2014-07-30 update

### 0.17.2
* general
  * updated version of MinHook and changed build procedures

### 0.17.1
* medigun info
  * fix issues with charge advantage info (thanks Bluee/MattMcNam)
* player aliases
  * add Twitch API support

### 0.17.0
* player aliases
  * add ETF2L API support

### 0.16.0
* loadout icons
  * add ability to only show the active weapon
* local player
  * add command to track spectator target
  * fix warning being emitted on successful changes
* medigun info
  * change how progress bars are set (requires changes to MedigunInfo.res)
  * add ability to have custom meter settings based on medigun type

### 0.15.0
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

### 0.14.1
* fix glitches with killstreaks on death
* add command to help stop flickering issues with player outlines

### 0.14.0
* add killstreaks module

### 0.13.1
* fix bleed icons not showing up

### 0.13.0
* add support for health-based team colored outlines

### 0.12.1
* fix medigun info not disappearing properly
* make unloading/pausing/unpausing cleaner and hopefully less error-prone

### 0.12.0
* add support for HUD features required for casting HUDs
* add ability to set progress direction via resource file and removed command
* add setting to allow Vaccinator to have only one charge meter
* add ability to view medigun charge advantage
* fix issues with medigun finding

### 0.11.3
* fix issues with player outlines not changing color after reloads

### 0.11.2
* fix player outline issues with players not being hooked/unhooked properly

### 0.11.1
* update offsets for TF2 update on 2014-06-11

### 0.11.0
* reorganize entire plugin into modules (changing many console variables and commands)
* add player outlines module

### 0.10.0
* fix player alias incompatibility with Spec Tools
* add utility command for setting the direction of a progress bar

### 0.9.0
* implement medigun info box

### 0.8.0
* implement player aliasing

### 0.7.0
* implement loadout icons

### 0.6.0
* implement status icons as part of spectator GUI

### 0.5.0
* added option to force refreshing of spectator GUI (partially to fully fixes post-pause issues, YMMV)

### 0.4.0
* added option (enabled by default) to have icon backgrounds adjust with icons rather than be static

### 0.3.0
* scale HUD positioning values automatically

### 0.2.0
* added Vaccinator resistances/Ubers, Soldier banners, Jarate, Mad Milk, bleeding, and on fire icons to display

### 0.1.0
* initial version
* Uber, Kritz, marked for death status icons
* many CVars for customizing appearance

Usage
-----
To install, place the `StatusSpec` folder within the `custom` folder in the `tf` directory. Because this plugin is not signed by Valve, you will not be able to use this plugin on VAC-secured servers and must launch TF2 with the `-insecure` option in order for this plugin to be active.

### AntiFreeze
*forces spectator GUI to refresh constantly, eliminating many HUD issues after game unpauses*

#### Console Variables
* `statusspec_antifreeze_enabled` - enable antifreeze (forces the spectator GUI to refresh)

### Camera Auto Switch
*automatically switches the camera based on game events*

#### Console Variables
* `statusspec_cameraautoswitch_enabled` - enable automatic switching of camera
* `statusspec_cameraautoswitch_killer` - switch to killer upon spectated player death
* `statusspec_cameraautoswitch_killer_delay` - delay before switching to killer

### Camera Smooths
*smooth transitions between camera positions*

#### Console Variables
* `statusspec_camerasmooths_enabled` - enable smooth transition between camera positions
* `statusspec_camerasmooths_max_angle_difference` - max angle difference at which smoothing will be performed
* `statusspec_camerasmooths_max_distance` - max distance at which smoothing will be performed
* `statusspec_camerasmooths_move_speed` - speed to move view per second

### Camera State
*exposes the current SourceTV camera state*

#### Console Variables
* `statusspec_camerastate_change` - JSON to change camera state to
* `statusspec_camerastate_current` - JSON of current camera state
* `statusspec_camerastate_enabled` - enable exposure of camera state

### Camera Tools
*allows more options for managing the camera*

#### Console Commands
* `statusspec_cameratools_spec_player <index>` - spec a certain player by player index
* `statusspec_cameratools_spec_player <team> <position>` - spec a certain player by position on spectator HUD
* `statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>` - spec a certain camera position

#### Console Variables
* `statusspec_cameratools_force_mode` - if a valid mode, force the camera mode to this
* `statusspec_cameratools_force_target` - if a valid target, force the camera target to this
* `statusspec_cameratools_force_valid_target` - forces the camera to only have valid targets
* `statusspec_cameratools_spec_player_alive` - prevent speccing dead players

### Console Tools
*allows more options for managing the console*

#### Console Commands
* `statusspec_consoletools_filter_add <filter>` - add a console filter
* `statusspec_consoletools_filter_remove <filter>` - remove a console filter
* `statusspec_consoletools_flags_add <name> <flag1> [flag2 ...]` - add a flag to a console command or variable
* `statusspec_consoletools_flags_remove <name> <flag1> [flag2 ...]` - remove a flag from a console command or variable

#### Console Variables
* `statusspec_consoletools_filter_enabled` - enable console filtering

### Custom Materials
*allows materials to be swapped out*

#### Console Commands
* `statusspec_custommaterials_load_replacement_group <group>` - load a material replacement group
* `statusspec_custommaterials_reload_settings` - reload settings for the custom materials from the resource file
* `statusspec_custommaterials_unload_replacement_group <group>` - unload a material replacement group

#### Console Variables
* `statusspec_custommaterials_enabled` - enable custom materials

#### Resource Files
Custom material configuration is loaded from the `Resource/CustomMaterials.res` file. Replacement groups should be configured as sections (whose names are used in the commands for this module). Material replacements should be specified by entries with the path of the old material from the `materials` folder as the key and the path of the new material from the `materials` folder as the value. An example of a configured file is given below:
```
"materials"
{
	"blank"
	{
		"signs/team_blue"	"signs/team_blue_blank"
		"signs/team_red"	"signs/team_red_blank"
	}
}
```

### Custom Models
*allows models to be swapped out*

#### Console Commands
* `statusspec_custommodels_load_replacement_group <group>` - load a model replacement group
* `statusspec_custommodels_reload_settings` - reload settings for the custom models from the resource file
* `statusspec_custommodels_unload_replacement_group <group>` - unload a model replacement group

#### Console Variables
* `statusspec_custommodels_enabled` - enable custom models

#### Resource Files
Custom model configuration is loaded from the `Resource/CustomModels.res` file. Replacement groups should be configured as sections (whose names are used in the commands for this module). Material replacements should be specified by entries with the path of the old model as the key and the path of the new model as the value. An example of a configured file is given below:

```
"models"
{
	"demo"
	{
		"models/player/scout.mdl"		"models/player/demo.mdl"
		"models/player/soldier.mdl"		"models/player/demo.mdl"
		"models/player/pyro.mdl"		"models/player/demo.mdl"
		"models/player/demo.mdl"		"models/player/demo.mdl"
		"models/player/heavy.mdl"		"models/player/demo.mdl"
		"models/player/engineer.mdl"	"models/player/demo.mdl"
		"models/player/medic.mdl"		"models/player/demo.mdl"
		"models/player/sniper.mdl"		"models/player/demo.mdl"
		"models/player/spy.mdl"			"models/player/demo.mdl"
	}
}
```

### Custom Textures
*allows textures to be swapped out*

#### Console Commands
* `statusspec_customtextures_load_replacement_group <group>` - load a texture replacement group
* `statusspec_customtextures_reload_settings` - reload settings for the custom textures from the resource file
* `statusspec_customtextures_unload_replacement_group <group>` - unload a texture replacement group

#### Console Variables
* `statusspec_customtextures_enabled` - enable custom textures

#### Resource Files
Custom texture configuration is loaded from the `Resource/CustomTextures.res` file. Replacement groups should be configured as sections (whose names are used in the commands for this module). Texture replacements should be specified by entries with the path of the old texture from the `materials` folder as the key and the path of the new texture from the `materials` folder as the value. An example of a configured file is given below:
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

### Filesystem Tools
*tools to manipulate the virtual filesystem*

#### Console Commands
* `statusspec_filesystemtools_searchpath_add <path> <id> [location]` - add a filesystem search path
* `statusspec_filesystemtools_searchpath_remove <path> <id>` - remove a filesystem search path

### FOV Override
*forces the FOV used to be consistent even if players have different FOVs*

#### Console Variables
* `statusspec_fovoverride_enabled` - enable FOV override
* `statusspec_fovoverride_fov` - the FOV value used
* `statusspec_fovoverride_zoomed` - enable FOV override even when sniper rifle is zoomed

### Freeze Info
*displays info when the game is frozen*

#### Console Commands
* `statusspec_freezeinfo_reload_settings` - reload settings for the freeze info panel from the resource file

#### Console Variables
* `statusspec_freezeinfo_enabled` - enables display of an info panel when a freeze is detected
* `statusspec_freezeinfo_threshold` - the time of a freeze (in seconds) before the info panel is displayed

#### UI Resource Files
The configuration file for the freeze info HUD is `Resource/UI/FreezeInfo.res`. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_freezeinfo_reload_settings` is provided as a replacement.

### Killstreaks
*enables killstreak tracking for all players*

#### Console Variables
* `statusspec_killstreaks_enabled` - enable killstreaks display

### Local Player
*changes the local player locally to affect certain HUD elements like the killfeed*

#### Console Commands
* `statusspec_localplayer_set_current_target` - set the local player to the current spectator target

#### Console Variables
* `statusspec_localplayer_enabled` - enable local player override
* `statusspec_localplayer_player` - player index to set as the local player
* `statusspec_localplayer_trac_spec_target` - have the local player value track the spectator target

### Medigun Info
*displays info about mediguns for both teams*

#### Console Commands
* `statusspec_mediguninfo_reload_settings` - reload settings for the medigun info HUD from the resource file

#### Console Variables
* `statusspec_mediguninfo_enabled` - enable medigun info

#### UI Resource Files
The configuration files for the medigun info HUD are `Resource/UI/MedigunInfo.res`, which controls the overall layout of the panel, and `Resource/UI/MedigunPanel.res`, which controls the display of each medigun. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_mediguninfo_reload_settings` is provided as a replacement.

The medigun panel located in `Resource/UI/MedigunPanel.res` supports conditional sections. This allows the HUD to change based on certain conditions, such as the medigun type or player team, by putting relevant keys within a section named with the condition. Conditions can be stacked by placing one conditional section within another, which will only trigger the contained keys if both conditions are triggered. The following conditions are supported:
* `charges-0` - no charges are ready
* `charges-1` - one charge is ready
* `charges-2` - two charges are ready
* `charges-3` - three charges are ready
* `charges-4` - four charges are ready
* `resist-bullet` - current medigun resistance is bullet
* `resist-explosive` - current medigun resistance is explosive
* `resist-fire` - current medigun resistance is fire
* `medigun-kritzkrieg` - medigun is the Kritzkrieg
* `medigun-medigun` - medigun is the stock Medi Gun
* `medigun-quickfix` - medigun is the Quick-Fix
* `medigun-vaccinator` - medigun is the Vaccinator
* `player-alive` - player using medigun is currently alive
* `player-dead` - player using medigun is currently dead
* `status-building` - charge is being built
* `status-released` - charge is being used
* `team-blu` - player using medigun is on the BLU team
* `team-red` - player using medigun is on the RED team

In addition, certain dialog variables will be set for each medigun panel. They are:
* `charge` - total percentage of uber filled (out of 100)
* `charges` - number of Vaccinator charges available
* `charge1` - percentage of first Vaccinator charge filled (out of 100)
* `charge2` - percentage of second Vaccinator charge filled (out of 100)
* `charge3` - percentage of third Vaccinator charge filled (out of 100)
* `charge4` - percentage of fourth Vaccinator charge filled (out of 100)

### Multipanel
*displays panels on top of the main game*

#### Console Variables
* `statusspec_multipanel_console` - displays a console in the HUD
* `statusspec_multipanel_scoreboard` - displays the scoreboard

### Player Aliases
*substitutes in user-defined aliases for player names*

#### Console Commands
* `statusspec_playeraliases_get <steamid>` - get an alias for a player
* `statusspec_playeraliases_remove <steamid>` - remove an alias for a player
* `statusspec_playeraliases_set <steamid> <alias>` - set an alias for a player
* `statusspec_playeraliases_switch_teams` - switch name formats for both teams

#### Console Variables
* `statusspec_playeraliases_enabled` - enable player aliases
* `statusspec_playeraliases_format_blu` - the name format for BLU players
* `statusspec_playeraliases_format_red` - the name format for RED players

### Player Models
*changes the model used based on the player*

#### Console Commands
* `statusspec_playermodels_reload_settings` - reload settings for the player models from the resource file

#### Console Variables
* `statusspec_playermodels_enabled` - enable custom player models

#### Resource Files
Player model configuration is loaded from the `Resource/PlayerModels.res` file. Within the `players` section, players should be configured with an entry consisting of their Steam ID as the key and a group name from the `groups` section as the value. Within the `groups` section, each group should have a section (named with the group name specified in the `players` section above), and model replacements should be specified by entries with the path of the old model as the key and the path of the new model as the value. An example of a configured file is given below:
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
			"models/player/scout.mdl"		"models/player/demo.mdl"
			"models/player/soldier.mdl"		"models/player/demo.mdl"
			"models/player/pyro.mdl"		"models/player/demo.mdl"
			"models/player/demo.mdl"		"models/player/demo.mdl"
			"models/player/heavy.mdl"		"models/player/demo.mdl"
			"models/player/engineer.mdl"	"models/player/demo.mdl"
			"models/player/medic.mdl"		"models/player/demo.mdl"
			"models/player/sniper.mdl"		"models/player/demo.mdl"
			"models/player/spy.mdl"			"models/player/demo.mdl"
		}
	}
}
```

### Projectile Outlines
*displays bright outlines around projectiles that can be seen through walls*

#### Console Commands
* `statusspec_projectileoutlines_color_blu <red> <green> <blue> [alpha]` - the color used for outlines for BLU projectiles
* `statusspec_projectileoutlines_color_red <red> <green> <blue> [alpha]` - the color used for outlines for RED projectiles

#### Console Variables
* `statusspec_projectileoutlines_enabled` - enable projectile outlines
* `statusspec_projectileoutlines_fade` - make outlines fade with distance
* `statusspec_projectileoutlines_fade_distance` - the distance (in Hammer units) at which outlines will fade
* `statusspec_projectileoutlines_grenades` - enable outlines for grenades
* `statusspec_projectileoutlines_rockets` - enable outlines for rockets
* `statusspec_projectileoutlines_stickybombs` - enable outlines for stickybombs

### Steam Tools
*allows more options for interacting with Steam*

#### Console Variables
* `statusspec_steamtools_rich_presence_status` - the rich presence status displayed to Steam

### Team Health Comparison
*displays a comparison of total health between the two teams*

#### Console Commands
* `statusspec_teamhealthcomparison_reload_settings` - reload settings for the team health comparison HUD from the resource file

#### Console Variables
* `statusspec_teamhealthcomparison_enabled` - enable team health comparison

#### UI Resource Files
The configuration file for the team health comparison HUD is `Resource/UI/TeamHealthComparison.res`. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_teamhealthcomparison_reload_settings` is provided as a replacement.

Thanks
------
**Matt McNamara** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**AlliedModders** - for in-depth information about the internals of the Source Engine and for [SourceHook](https://github.com/alliedmodders/metamod-source/tree/master/core/sourcehook)

**Tsuda Kageyu and other contributors** - for [MinHook](https://github.com/TsudaKageyu/minhook)

**Milo Yip and other contributors** - for [RapidJSON](https://github.com/miloyip/rapidjson)
