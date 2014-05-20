StatusSpec
==========

a client plugin that shows certain player statuses via icons next to a tournament HUD

![screenshot](http://i.imgur.com/XS9VQaV.png)

Changelog
---------

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
* `statusspec_status_icons_enabled` - enable status icons
* `statusspec_status_icons_max` - max number of status icons to be rendered

### Console Commands
* `statusspec_loadout_filter_active` - the RGBA filter applied to the icon when the item is active (usage: statusspec_loadout_filter_active <red> <green> <blue> <alpha>)
* `statusspec_loadout_filter_nonactive` - the RGBA filter applied to the icon when the item is not active (usage: statusspec_loadout_filter_nonactive <red> <green> <blue> <alpha>)

### UI Resource Files

#### Status Icons
To properly support status icons, add something similar to the following section within the `playerpanels_kv` section of the `specgui` section in `Resource/UI/SpectatorTournament.res`. The icons will appear on the right of `xpos` with a square size of `tall`, and the player panel will also be expanded to accommodate these icons. The rendering of this panel assumes that this element is on the right edge of the player panel and will not appear if status icons are disabled.
```
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
```

#### Loadout Icons
To properly support loadout icons, add something similar to the following section within the `playerpanels_kv` section of the `specgui` section in `Resource/UI/SpectatorTournament.res` and adjust the rest of the file as if this were a normal GUI panel. The icons will be entirely contained within this element as defined in this configuration, and each icon will be a square size of `tall`. The rendering of this panel only displays icons and will not adjust anything else, no matter whether this feature is enabled or disabled - thus, you will be required to manually adjust your HUD as appropriate when enabling or disabling this feature.
```
"loadouticons"
{
	"ControlName"   "Panel"
	"fieldName"     "loadouticons"
	"xpos"          "16"
	"ypos"          "0"
	"zpos"          "2"
	"wide"          "60"
	"tall"          "15"
	"visible"       "1"
	"enabled"       "1"
}
```

Thanks
------
**bluee** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**SourceMod Development Team** - for the information on how to get TF2 player and weapon information