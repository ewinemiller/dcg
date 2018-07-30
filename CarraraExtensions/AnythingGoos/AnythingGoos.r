/*  Anything Goos - plug-in for Carrara
    Copyright (C) 2004 Eric Winemiller

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

#include "AnythingGoosDef.h"

include "AnythingGoos.rsr";

#define COPYRIGHT "Copyright © 2004 Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif


resource 'COMP' (327) {
	kRID_ShaderFamilyID,
	'SPAG',
	"Anything Goos",
	"Anything Goos",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (327) {
	{
		'SIZE', 're32', interpolate,"Goo Size","",
		'SPCE', 'in32', noflags,"Space","",
		'EDGT', 're32', noflags, "Angle", "",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'EPER', 'bool', noflags, "Perimeter", "",
		'APER', 'bool', noflags, "Advanced Perimeter", "",
		'EINN', 'bool', noflags, "Inner", "",
		'EOUT', 'bool', noflags, "Outer", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'SHDO', 'bool', noflags, "Shading Domains", "",
		'FANG', 're32', noflags, "Face Away Limit", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (327) {
{
		R_IID_I3DExShader,
		R_CLSID_AnythingGoos
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

#if (VERSIONNUMBER >= 0x040000)

resource 'CPUI' (329) 
{
	327,					// Id of your main part
	1327,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (329) {
	kRID_ShaderFamilyID,
	'SPAT',
	"Anything Goos",
	"Terrain Distribution # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

 resource 'GUID' (329) {
{
		R_IID_I3DExShader,
		R_CLSID_AnythingGoosTerrain
}
};

resource 'PMap' (329) {
	{
		'SIZE', 're32', interpolate,"Goo Size","",
		'SPCE', 'in32', noflags,"Space","",
		'EDGT', 're32', noflags, "Angle", "",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'EPER', 'bool', noflags, "Perimeter", "",
		'APER', 'bool', noflags, "Advanced Perimeter", "",
		'EINN', 'bool', noflags, "Inner", "",
		'EOUT', 'bool', noflags, "Outer", "",
		'MIMA', 'vec2', interpolate, "Min/Max", "",
		'SHDO', 'bool', noflags, "Shading Domains", "",
		'FANG', 're32', noflags, "Face Away Limit", "",
		'sh00','comp',interpolate, "Noise","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};
#endif