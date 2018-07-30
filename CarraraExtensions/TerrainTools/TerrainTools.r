/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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

#include "BeachDef.h"
#include "SurfDef.h"  
#include "CutoutDef.h"  
#include "IntersectDef.h"  
#include "NormalToTerrainDef.h"
#include "TerrainToNormalDef.h"

include "TerrainTools.rsr";

#define COPYRIGHT "Copyright © 2005 Digital Carvers Guild"
#define DCGMODGROUP "Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif


resource 'COMP' (327) {
	kRID_ShaderFamilyID,
	'TTBE',
	"Beach",
	"Terrain Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (327) {
	{
		'SIZE', 're32', interpolate,"Distance","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'INPL','s255', noflags, "Infinite Plane","",
	}
};


resource 'GUID' (327) {
{
		R_IID_I3DExShader,
		R_CLSID_Beach
}
};

resource 'CPUI' (327) 
{
	327,					// Id of your main part
	1327,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (328) {
	kRID_ShaderFamilyID,
	'TTBT',
	"Terrain Tools Beach",
	"Terrain Distribution # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (328) {
	{
		'SIZE', 're32', interpolate,"Distance","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'INPL','s255', noflags, "Infinite Plane","",
	}
};


resource 'GUID' (328) {
{
		R_IID_I3DExShader,
		R_CLSID_BeachTerrain
}
};

resource 'CPUI' (328) 
{
	327,					// Id of your main part
	1327,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};


resource 'COMP' (340) {
	kRID_ShaderFamilyID,
	'TTSU',
	"Surf",
	"Terrain Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (340) {
	{
		'SIZE', 're32', interpolate,"Distance","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'EFFE','in32',noflags,"Effect","",
		'LIST','Dstr',noflags,"List","",
		'WIND', 're32', interpolate,"Wind Effect","",
		'WIDI', 're32', interpolate,"Wind Direction","",
		'SULE', 're32', interpolate,"Surf Length","",
	}
};


resource 'GUID' (340) {
{
		R_IID_I3DExShader,
		R_CLSID_Surf
}
};

resource 'CPUI' (340) 
{
	340,					// Id of your main part
	1340,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (341) {
	kRID_ShaderFamilyID,
	'TTSD',
	"Terrain Tools Intersect",
	"Terrain Distribution # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (341) {
	{
		'SIZE', 're32', interpolate,"Distance","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'EFFE','in32',noflags,"Effect","",
		'LIST','Dstr',noflags,"List","",
	}
};


resource 'GUID' (341) {
{
		R_IID_I3DExShader,
		R_CLSID_Intersect
}
};

resource 'CPUI' (341) 
{
	341,					// Id of your main part
	1341,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (342) {
	kRID_ShaderFamilyID,
	'TTST',
	"Intersect",
	"Terrain Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (342) {
	{
		'SIZE', 're32', interpolate,"Distance","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'EFFE','in32',noflags,"Effect","",
		'LIST','Dstr',noflags,"List","",
	}
};


resource 'GUID' (342) {
{
		R_IID_I3DExShader,
		R_CLSID_Intersect
}
};

resource 'CPUI' (342) 
{
	341,					// Id of your main part
	1341,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (350) {
	kRID_ShaderFamilyID,
	'TTCO',
	"Cutout",
	"Terrain Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (350) {
	{
		'INVT', 'bool', noflags, "Invert", "",
		'EFFE','in32',noflags,"Effect","",
		'LIST','Dstr',noflags,"List","",

	}
};


resource 'GUID' (350) {
{
		R_IID_I3DExShader,
		R_CLSID_Cutout
}
};

resource 'CPUI' (350) 
{
	350,					// Id of your main part
	1350,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};


resource 'COMP' (313) {
	kRID_ShaderFamilyID,
	'TTNS',
	"Terrain Tools Normal Shader",
	"Terrain Distribution # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (313) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (313) {
{
		R_IID_I3DExShader,
		R_CLSID_NormalToTerrain
}
};

resource 'COMP' (314) {
	kRID_ShaderFamilyID,
	'TTTD',
	"Terrain Distributions",
	"Terrain Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (314) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 32 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (314) {
{
		R_IID_I3DExShader,
		R_CLSID_TerrainToNormal
}
};
#if (VERSIONNUMBER >= 0x060000)

/*resource 'COMP' (400) {
	kRID_ModifierFamilyID,
	'TTWA',
	"Terrain Tools Wave",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (400) {
	{	
		'ENAB','bool',0,"Enabled", "",
	}
};


resource 'GUID' (400) {
{
		R_IID_I3DExModifier,
		R_CLSID_TTWave
}
};*/
#endif
