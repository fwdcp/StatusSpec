StatusSpec
==========

a client plugin that shows certain player statuses via icons next to a tournament HUD

![screenshot](http://cloud-3.steampowered.com/ugc/3296944182337081425/381C3EAEAD980236654DF6199422E235C1F98891/)

Changelog
---------

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
This plugin is not signed, and therefore will not load unless TF2 is loaded with the "-insecure" flag.

### CVars
* `statusspec_force_refresh_specgui` - whether to force the spectator GUI to refresh
* `statusspec_status_icons_enabled` - enable status icons
* `statusspec_status_icons_max` - max number of status icons to be rendered

### UI Resource Files

#### Status Icons
To enable status icons, add something similar to the following section within the `playerpanels_kv` section of the `specgui` section in `Resource/UI/SpectatorTournament.res`. The icons will appear on the right of `xpos` set with height and width `tall`, and the player panel will also be expanded to accommodate these icons.
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

Thanks
------
**bluee** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**SourceMod Development Team** - for the information on how to get TF2 player conditions