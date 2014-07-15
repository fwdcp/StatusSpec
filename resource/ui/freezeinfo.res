"Resource/UI/FreezeInfo.res"
{
	"FreezeInfo"
	{
		"ControlName"	"Frame"
		"fieldName"		"FreezeInfo"
		"xpos"			"c-80"
		"ypos"			"c-16"
		"wide"			"160"
		"tall"			"32"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"
	}
	
	"FreezeInfoBackground"
	{
		"ControlName" 	"ImagePanel"
		"fieldName" 	"FreezeInfoBackground"
		"xpos" 			"0"
		"ypos"			"0"
		"zpos" 			"0"
		"wide"			"160"
		"tall"			"32"
		"autoResize" 	"0"
		"pinCorner"		"0"
		"visible" 		"1"
		"enabled" 		"1"
		"fillcolor" 	"NotoDark"
	}
	
	"FreezeInfoIcon"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"FreezeInfoIcon"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"32"
		"tall"			"32"
		"visible"		"1"
		"enabled"		"1"
		"scaleImage"	"1"
		"image"			"animated/tf2_logo_hourglass"
	}
	
	"FreezeInfoLabel"
	{
		"ControlName"	"VariableLabel"
		"fieldName"		"MedigunInfoRedNameLabel"
		"xpos"			"32"
		"ypos"			"0"
		"wide"			"128"
		"tall"			"32"
		"autoResize"	"1"
		"visible"		"1"
		"enabled"		"1"
		"labelText"		"FROZEN FOR %time%"
		"tabPosition"	"0"
		"textAlignment"	"center"
		"dulltext"		"0"
		"brighttext"	"0"
		"font"			"Noto16"
		"fgcolor"		"NotoWhite"
	}
}