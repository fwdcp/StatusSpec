StatusSpec
==========

a client plugin that:
* almost fixes HUD freeze issues
* displays loadout icons
* displays medigun info
* displays status icons

![screenshot](http://i.imgur.com/HliUKLk.png)

Changelog
---------

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
This plugin is not signed by Valve, and therefore will not load unless TF2 is loaded with the "-insecure" flag.

### Console Variables
* `statusspec_force_refresh_specgui` - whether to force the spectator GUI to refresh
* `statusspec_loadout_icons_enabled` - enable loadout icons
* `statusspec_loadout_icons_nonloadout` - enable loadout icons for nonloadout items
* `statusspec_medigun_info_charge_label_text` - text for charge label in medigun info ('%charge%' is replaced with the current charge percentage number)
* `statusspec_medigun_info_enabled` - enable medigun info
* `statusspec_medigun_info_individual_charges_label_text` - text for individual charges label (for Vaccinator) in medigun info ('%charges%' is replaced with the current number of charges)
* `statusspec_player_alias_enabled` - enable player aliases
* `statusspec_status_icons_enabled` - enable status icons
* `statusspec_status_icons_max` - max number of status icons to be rendered

### Console Commands
* `statusspec_loadout_filter_active <red> <green> <blue> <alpha>` - the RGBA filter applied to the icon when the item is active
* `statusspec_loadout_filter_nonactive <red> <green> <blue> <alpha>` - the RGBA filter applied to the icon when the item is not active
* `statusspec_medigun_info_reload_settings` - reload settings for the medigun info HUD from the resource file
* `statusspec_player_alias_get <steamid>` - get an alias for a player
* `statusspec_player_alias_remove <steamid>` - remove an alias for a player
* `statusspec_player_alias_set <steamid> <alias>` - set an alias for a player

### UI Resource Files

#### Loadout Icons
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

#### Medigun Info Box
An example file for the medigun info box is included under `Resource/UI/MedigunInfo.res`. This HUD cannot be refreshed using the normal `hud_reloadscheme` because it isn't natively implemented into TF2, and thus the command `statusspec_medigun_info_reload_settings` is provided as a replacement.

In addition, the following HUD animations are triggered by this plugin and may be used to show events on the HUD:
* `MedigunInfoBluChargeReady` - triggered when a BLU medigun charge is ready for use
* `MedigunInfoBluChargeReleased` - triggered when a BLU medigun charge has been released/popped
* `MedigunInfoBluChargeStop` - triggered when the BLU medigun charge is no longer ready/released
* `MedigunInfoRedChargeReady` - triggered when a RED medigun charge is ready for use
* `MedigunInfoRedChargeReleased` - triggered when a RED medigun charge has been released/popped
* `MedigunInfoRedChargeStop` - triggered when the RED medigun charge is no longer ready/released (after a charge is completed/dropped)

#### Status Icons
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

Thanks
------
**bluee** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**AlliedModders** - for in-depth information about the internals of the Source Engine and for SourceHook