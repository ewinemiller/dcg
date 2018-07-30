/*  Shader Ops - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    http://digitalcarversguild.com/
    email: ewinemiller @ digitalcarversguild.com (remove space)
*/
#include "ExternalAPI.r"
#include "External3DAPI.r"
#include "Copyright.h"
#include "interfaceids.h"
#include "imageiids.h"
#include "RealUVDef.h"
#include "ClipDef.h"
#include "CompressDef.h"
#include "StretchDef.h"
#include "UVTileDef.h"
#include "ThresholdDef.h"
#include "InvertDef.h"
#include "BrightnessDef.h"
#include "MaxDef.h"
#include "MinDef.h"
#include "MaxThresholdDef.h"
#include "MinThresholdDef.h"
#include "NoteDef.h"
#include "TranslateDef.h"
#include "ScaleDef.h"
#include "FresnelDef.h"
#include "UVShifterDef.h"
#include "RadialGradientDef.h"
#include "ProgScaleDef.h"
#include "GroupShaderDef.h"
#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "ShaderOps.rsr";

#define COPYRIGHT "Copyright © 2003 Digital Carvers Guild"
#define DCGOPGROUP "Shader Ops"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (304) {
	kRID_ShaderFamilyID,
	'REUV',
	"Real UV",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (304) {
	{	
		'DIRE','in32',0,"Direction","",
		'SPCE','in32',0,"Space","",
		'SIZU','re32',1,"Size U","",
		'SIZV','re32',1,"Size V","",
		'ROTA','in32',0,"Rotation","",
		'OFFU','re32',1,"Offset U","",
		'OFFV','re32',1,"Offset V","",
		'ODDL','re32',1,"Odd Lap","",
		'EVNL','re32',1,"Even Lap","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (304) {
{
		R_IID_I3DExShader,
		R_CLSID_RealUV
}
};

resource 'CPUI' (304) 
{
	304,					// Id of your main part
	1304,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (305) {
	kRID_ShaderFamilyID,
	'SOTR',
	"Translate",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (305) {
	{	
		'SPCE','in32',0,"Space","",
		'OFFX','re32',1,"Offset X","",
		'OFFY','re32',1,"Offset Y","",
		'OFFZ','re32',1,"Offset Z","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (305) {
{
		R_IID_I3DExShader,
		R_CLSID_Translate
}
};

resource 'CPUI' (305) 
{
	305,					// Id of your main part
	1305,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (306) {
	kRID_ShaderFamilyID,
	'SOSC',
	"Scale",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (306) {
	{	
		'SPCE','in32',0,"Space","",
		'LOCX','re32',1,"Scale X","",
		'LOCY','re32',1,"Scale Y","",
		'LOCZ','re32',1,"Scale Z","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (306) {
{
		R_IID_I3DExShader,
		R_CLSID_Scale
}
};

resource 'CPUI' (306) 
{
	306,					// Id of your main part
	1306,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (308) {
	kRID_ShaderFamilyID,
	'CLIP',
	"Clip",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (308) {
	{
		'CLIP','vec2',interpolate,"Clip","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (308) {
{
		R_IID_I3DExShader,
		R_CLSID_Clip
}
};

resource 'CPUI' (308) 
{
	308,					// Id of your main part
	1308,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (309) {
	kRID_ShaderFamilyID,
	'COMP',
	"Compress",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (309) {
	{
		'COMP','vec2',interpolate,"Compress","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (309) {
{
		R_IID_I3DExShader,
		R_CLSID_Compress
}
};

resource 'CPUI' (309) 
{
	309,					// Id of your main part
	1309,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
resource 'COMP' (310) {
	kRID_ShaderFamilyID,
	'STRC',
	"Stretch",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (310) {
	{
		'STRC','vec2',interpolate,"Stretch","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (310) {
{
		R_IID_I3DExShader,
		R_CLSID_Stretch
}
};

resource 'CPUI' (310) 
{
	310,					// Id of your main part
	1310,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
resource 'COMP' (311) {
	kRID_ShaderFamilyID,
	'UREP',
	"UV Repeater",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (311) {
	{
		'REPU','re32',interpolate,"Repeat U","",
		'REPV','re32',interpolate,"Repeat V","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (311) {
{
		R_IID_I3DExShader,
		R_CLSID_UVTile
}
};

resource 'CPUI' (311) 
{
	311,					// Id of your main part
	1311,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
resource 'COMP' (312) {
	kRID_ShaderFamilyID,
	'THRS',
	"Threshold",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (312) {
	{
		'THRS','re32',interpolate,"Threshold","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (312) {
{
		R_IID_I3DExShader,
		R_CLSID_Threshold
}
};

resource 'CPUI' (312) 
{
	312,					// Id of your main part
	1312,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
resource 'COMP' (313) {
	kRID_ShaderFamilyID,
	'INVR',
	"Invert",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (313) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (313) {
{
		R_IID_I3DExShader,
		R_CLSID_Invert
}
};

resource 'CPUI' (313) 
{
	313,					// Id of your main part
	1313,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (314) {
	kRID_ShaderFamilyID,
	'BRIT',
	"Brightness",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (314) {
	{
		'BRIT','re32',interpolate,"Brightness","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (314) {
{
		R_IID_I3DExShader,
		R_CLSID_Brightness
}
};

resource 'CPUI' (314) 
{
	314,					// Id of your main part
	1314,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (315) {
	kRID_ShaderFamilyID,
	'SOMX',
	"Max",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (315) {
	{
		'sh00','comp',interpolate, "Source 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Source 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (315) {
{
		R_IID_I3DExShader,
		R_CLSID_Max
}
};

resource 'CPUI' (315) 
{
	315,					// Id of your main part
	1315,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (316) {
	kRID_ShaderFamilyID,
	'SOMN',
	"Min",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (316) {
	{
		'sh00','comp',interpolate, "Source 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Source 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (316) {
{
		R_IID_I3DExShader,
		R_CLSID_Min
}
};

resource 'CPUI' (316) 
{
	316,					// Id of your main part
	1316,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (317) {
	kRID_ShaderFamilyID,
	'SONT',
	"Note",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (317) {
	{
		'NOTE','s255',0,"Note","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (317) {
{
		R_IID_I3DExShader,
		R_CLSID_Note
}
};

resource 'CPUI' (317) 
{
	317,					// Id of your main part
	1317,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (318) {
	kRID_ShaderFamilyID,
	'SOFR',
	"Fake Fresnel",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (318) {
 {
		'ROLL','re32',interpolate,"Rolloff","",
	}
};


resource 'GUID' (318) {
{
		R_IID_I3DExShader,
		R_CLSID_Fresnel
}
};

resource 'CPUI' (318) 
{
	318,					// Id of your main part
	1318,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (319) {
	kRID_ShaderFamilyID,
	'SOXT',
	"Max Threshold",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (319) {
	{
		'sh00','comp',interpolate, "Source 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Source 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (319) {
{
		R_IID_I3DExShader,
		R_CLSID_MaxThreshold
}
};

resource 'CPUI' (319) 
{
	319,					// Id of your main part
	1319,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (320) {
	kRID_ShaderFamilyID,
	'SOMT',
	"Min Threshold",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (320) {
	{
		'sh00','comp',interpolate, "Source 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Source 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (320) {
{
		R_IID_I3DExShader,
		R_CLSID_MinThreshold
}
};

resource 'CPUI' (320) 
{
	320,					// Id of your main part
	1320,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (321) {
	kRID_ShaderFamilyID,
	'SOUV',
	"UV Shifter",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (321) {
	{
		'REPU','re32',interpolate,"Offset U","",
		'REPV','re32',interpolate,"Offset V","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (321) {
{
		R_IID_I3DExShader,
		R_CLSID_UVShifter
}
};

resource 'CPUI' (321) 
{
	321,					// Id of your main part
	1321,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (322) {
	kRID_ShaderFamilyID,
	'RGRD',
	"Radial Gradient",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (322) {
	{
		'RANG','vec2',interpolate,"Range","",
		'USTR','re32',interpolate,"U Start","",
		'VSTR','re32',interpolate,"V Start","",
	
	}
};


resource 'GUID' (322) {
{
		R_IID_I3DExShader,
		R_CLSID_RadialGradient
}
};

resource 'CPUI' (322) 
{
	322,					// Id of your main part
	1322,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (204)
{
	kRID_SceneCommandFamilyID,
	'SHGS',
	"Apply Shader to Selected Items",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (204)
{
	{
		'MSHD','actn',interpolate,"Master Shader","",
	}
};

resource 'GUID' (204)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_GroupShader
	}
};
#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (204) 
{
	kThirdPartiesCmdIDBase + kCMDIdApplyShaderToSelectedObjects, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (204) 
{
	-1, -1,{'3Dvw'}
};
#endif

resource 'COMP' (323) {
	kRID_ShaderFamilyID,
	'SOPS',
	"Programmable Scale",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (323) {
	{
		'sh00','comp',interpolate, "Source","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Scale","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (323) {
{
		R_IID_I3DExShader,
		R_CLSID_ProgScale
}
};

resource 'CPUI' (323) 
{
	315,					// Id of your main part
	1315,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
