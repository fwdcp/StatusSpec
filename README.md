StatusSpec
==========

a client plugin that shows certain player statuses via icons next to a tournament HUD

![screenshot](http://cloud-3.steampowered.com/ugc/3296944113103557748/0A2F0965D6085C0BA1CE1B85C44D673F0F78E2FC/)

Changelog
---------

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
* **statusspec_force_refresh_specgui** - whether to force the specgui to refresh
* **statusspec_icons_enabled** - enable status icons
* **statusspec_icons_bg_dynamic** - dynamically move the background color with the icons
* **statusspec_icons_size** - square size of status icons
* **statusspec_icons_max** - max number of icons to be rendered
* **statusspec_icons_blu_x_base** - x-coordinate of the first BLU player
* **statusspec_icons_blu_x_delta** - amount to move in x-direction for next BLU player
* **statusspec_icons_blu_y_base** - y-coordinate of the first BLU player
* **statusspec_icons_blu_y_delta** - amount to move in y-direction for next BLU player
* **statusspec_icons_blu_bg_red** - red value of the icon background for BLU players
* **statusspec_icons_blu_bg_green** - green value of the icon background for BLU players
* **statusspec_icons_blu_bg_blue** - blue value of the icon background for BLU players
* **statusspec_icons_blu_bg_alpha** - alpha value of the icon background for BLU players
* **statusspec_icons_red_x_base** - x-coordinate of the first RED player
* **statusspec_icons_red_x_delta** - amount to move in x-direction for next RED player
* **statusspec_icons_red_y_base** - y-coordinate of the first RED player
* **statusspec_icons_red_y_delta** - amount to move in y-direction for next RED player
* **statusspec_icons_red_bg_red** - red value of the icon background for RED players
* **statusspec_icons_red_bg_green** - green value of the icon background for RED players
* **statusspec_icons_red_bg_blue** - blue value of the icon background for RED players
* **statusspec_icons_red_bg_alpha** - alpha value of the icon background for RED players

Thanks
------
**bluee** - for the inspiration to do this project and for a lot of code reused from his [AdvSpec project](https://github.com/MattMcNam/advspec)

**SourceMod Development Team** - for the information on how to get TF2 player conditions