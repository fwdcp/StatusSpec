"Resource/UI/MedigunPanel.res"
{
	"MedigunPanel"
	{
		"ControlName"	"Frame"
		"fieldName"		"MedigunPanel"
		"wide"			"164"
		"tall"			"18"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"
	}

    "MedigunPanelBackground"
    {
        "ControlName" 	"ScalableImagePanel"
        "fieldName" 	"MedigunPanelBackground"
        "xpos" 			"2"
        "ypos"			"1"
        "zpos" 			"1"
        "wide" 			"160"
        "tall" 			"16"
        "autoResize" 	"0"
        "pinCorner"		"0"
        "visible" 		"1"
        "enabled" 		"1"

        "team-red"
        {
            "drawcolor" "HUDRedTeam"
        }

        "team-blu"
        {
            "drawcolor" "HUDBlueTeam"
        }
    }



    "MedigunPanelChargeTypeIcon"
    {
        "ControlName"       "ImagePanel"
        "fieldName"         "MedigunPanelChargeTypeIcon"
        "xpos"              "3"
        "ypos"              "2"
        "zpos"              "2"
        "wide"              "14"
        "tall"              "14"
        "visible"           "1"
        "enabled"           "1"
        "scaleImage"        "1"

        "medigun-medigun"
        {
            "image"         "replay/thumbnails/ubercharge"
        }

        "medigun-kritzkrieg"
        {
            "image"         "replay/thumbnails/critboost"
        }

        "medigun-quickfix"
        {
            "team-red"
            {
                "image"     "replay/thumbnails/megaheal_red"
            }

            "team-blu"
            {
                "image"     "replay/thumbnails/megaheal_blue"
            }
        }

        "medigun-vaccinator"
        {
            "team-red"
            {
                "resist-bullet"
                {
                    "image" "replay/thumbnails/defense_buff_bullet_red"
                }

                "resist-explosive"
                {
                    "image" "replay/thumbnails/defense_buff_explosion_red"
                }

                "resist-fire"
                {
                    "image" "replay/thumbnails/defense_buff_fire_red"
                }
            }

            "team-blu"
            {
                "resist-bullet"
                {
                    "image" "replay/thumbnails/defense_buff_bullet_blue"
                }

                "resist-explosive"
                {
                    "image" "replay/thumbnails/defense_buff_explosion_blue"
                }

                "resist-fire"
                {
                    "image" "replay/thumbnails/defense_buff_fire_blue"
                }
            }
        }

        "player-dead"
        {
            "visible"       "0"
        }
    }

    "MedigunPanelChargeLabel"
    {
        "ControlName"	"VariableLabel"
        "fieldName"		"MedigunPanelChargeLabel"
        "xpos"			"147"
        "ypos"			"2"
        "zpos"			"2"
        "wide"			"14"
        "tall"			"14"
        "autoResize"	"1"
        "visible"		"1"
        "enabled"		"1"
        "labelText"		"%charge%%"
        "tabPosition"	"0"
        "textAlignment"	"center"
        "dulltext"		"0"
        "brighttext"	"0"
        "font"			"DefaultVerySmall"
        "fgcolor"		"HudWhite"

        "medigun-vaccinator"
        {
            "labelText" "%charges%"
        }

        "player-dead"
        {
            "visible"   "0"
        }
    }

    "MedigunPanelChargeMeter"
    {
        "ControlName"		"ImageProgressBar"
        "fieldName"			"MedigunPanelChargeMeter"
        "font"				"Default"
        "xpos"				"19"
        "ypos"				"2"
        "zpos"				"2"
        "wide"				"126"
        "tall"				"14"
        "autoResize"		"0"
        "pinCorner"			"0"
        "visible"			"1"
        "enabled"			"1"
        "textAlignment"		"Left"
        "dulltext"			"0"
        "brighttext"		"0"
        "bgcolor_override"	"ProgressBackground"
        "fgcolor_override" 	"ProgressOffWhite"
        "direction"			"east"
        "variable"			"charge"

        "medigun-vaccinator"
        {
            "visible"       "0"
        }

        "player-dead"
        {
            "visible"       "0"
        }
    }

    "MedigunPanelChargeMeter1"
    {
        "ControlName"		"ImageProgressBar"
        "fieldName"			"MedigunPanelChargeMeter1"
        "font"				"Default"
        "xpos"				"19"
        "ypos"				"2"
        "zpos"				"2"
        "wide"				"30"
        "tall"				"14"
        "autoResize"		"0"
        "pinCorner"			"0"
        "visible"			"0"
        "enabled"			"1"
        "textAlignment"		"Left"
        "dulltext"			"0"
        "brighttext"		"0"
        "bgcolor_override"	"ProgressBackground"
        "fgcolor_override" 	"ProgressOffWhite"
        "direction"			"east"
        "variable"			"charge1"

        "player-alive"
        {
            "medigun-vaccinator"
            {
                "visible"   "1"
            }
        }
    }

    "MedigunPanelChargeMeter2"
    {
        "ControlName"		"ImageProgressBar"
        "fieldName"			"MedigunPanelChargeMeter2"
        "font"				"Default"
        "xpos"				"51"
        "ypos"				"2"
        "zpos"				"2"
        "wide"				"30"
        "tall"				"14"
        "autoResize"		"0"
        "pinCorner"			"0"
        "visible"			"0"
        "enabled"			"1"
        "textAlignment"		"Left"
        "dulltext"			"0"
        "brighttext"		"0"
        "bgcolor_override"	"ProgressBackground"
        "fgcolor_override" 	"ProgressOffWhite"
        "direction"			"east"
        "variable"			"charge2"

        "player-alive"
        {
            "medigun-vaccinator"
            {
                "visible"   "1"
            }
        }
    }

    "MedigunPanelChargeMeter3"
    {
        "ControlName"		"ImageProgressBar"
        "fieldName"			"MedigunPanelChargeMeter3"
        "font"				"Default"
        "xpos"				"83"
        "ypos"				"2"
        "zpos"				"2"
        "wide"				"30"
        "tall"				"14"
        "autoResize"		"0"
        "pinCorner"			"0"
        "visible"			"0"
        "enabled"			"1"
        "textAlignment"		"Left"
        "dulltext"			"0"
        "brighttext"		"0"
        "bgcolor_override"	"ProgressBackground"
        "fgcolor_override" 	"ProgressOffWhite"
        "direction"			"east"
        "variable"			"charge3"

        "player-alive"
        {
            "medigun-vaccinator"
            {
                "visible"   "1"
            }
        }
    }

    "MedigunPanelChargeMeter4"
    {
        "ControlName"		"ImageProgressBar"
        "fieldName"			"MedigunPanelChargeMeter4"
        "font"				"Default"
        "xpos"				"115"
        "ypos"				"2"
        "zpos"				"2"
        "wide"				"30"
        "tall"				"14"
        "autoResize"		"0"
        "pinCorner"			"0"
        "visible"			"0"
        "enabled"			"1"
        "textAlignment"		"Left"
        "dulltext"			"0"
        "brighttext"		"0"
        "bgcolor_override"	"ProgressBackground"
        "fgcolor_override" 	"ProgressOffWhite"
        "direction"			"east"
        "variable"			"charge4"

        "player-alive"
        {
            "medigun-vaccinator"
            {
                "visible"   "1"
            }
        }
    }
}
