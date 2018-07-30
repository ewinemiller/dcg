/*  Shader Plus - plug-in for Carrara
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

#include "AnisotropicDef.h"
#include "FauxAnisotropicDef.h"
#include "FlatDef.h"
#include "ShadowGlowDef.h"
//#include "FakeSSDef.h"
#include "MaskDef.h"
#include "SelectiveLightingDef.h"
#include "GIShadowCatcherDef.h"
#include "GIControlDef.h"
#include "ProceduralLockDef.h"
#include "BevelDef.h"
//#include "IlluminationCatcherDef.h"
//#include "IlluminationLevelDef.h"
#include "SpecularDiffuseControlDef.h"


include "ShadersPlus.rsr";

#define COPYRIGHT "Copyright � 2004 Digital Carvers Guild"
#define DCGOPGROUP "Shaders Plus"


resource 'COMP' (323) {
	kRID_ShaderFamilyID,
	'SPAN',
	"Anisotropic",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (323) {
	{
		'XROU','re32',interpolate,"X Roughness","",
		'YROU','re32',interpolate,"Y Roughness","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
		
	}
};


resource 'GUID' (323) {
{
		R_IID_I3DExShader,
		R_CLSID_Anisotropic
}
};

resource 'COMP' (324) {
	kRID_ShaderFamilyID,
	'SPFL',
	"Flat",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (324) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (324) {
{
		R_IID_I3DExShader,
		R_CLSID_Flat
}
};

resource 'COMP' (326) {
	kRID_ShaderFamilyID,
	'SPSG',
	"Shadow Glow",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (326) {
	{
		'EFFE','re32',interpolate,"Effect","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (326) {
{
		R_IID_I3DExShader,
		R_CLSID_ShadowGlow
}
};

/*
resource 'COMP' (327) {
	kRID_ShaderFamilyID,
	'SPSS',
	"Fake Subsurface scattering",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (327) {
	{
		'EFFE','re32',noflags,"Effect","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (327) {
{
		R_IID_I3DExShader,
		R_CLSID_FakeSS
}
};
*/

resource 'COMP' (328) {
	kRID_ShaderFamilyID,
	'SPMA',
	"Mask",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (328) {
	{
		'EFFE','in32',noflags,"Effect","",
		'COLO','colo',interpolate,"Color","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (328) {
{
		R_IID_I3DExShader,
		R_CLSID_Mask
}
};

resource 'COMP' (329) {
	kRID_ShaderFamilyID,
	'SPSL',
	"Selective Lighting",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (329) {
	{
		'EFFE','in32',noflags,"Effect","",
		'LIST','Dstr',noflags,"List","",
		'ALAM','bool',noflags,"Allow Ambient","",
		'ALGI','bool',noflags,"Allow GI","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (329) {
{
		R_IID_I3DExShader,
		R_CLSID_SelectiveLighting
}
};

resource 'COMP' (330) {
	kRID_ShaderFamilyID,
	'SPGI',
	"GI Brightness",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (330) {
	{
		'EFFE','re32',interpolate,"Effect","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (330) {
{
		R_IID_I3DExShader,
		R_CLSID_GIControl
}
};

resource 'COMP' (331) {
	kRID_ShaderFamilyID,
	'SPGS',
	"GI Shadow Catcher",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (331) {
	{
		'EFFE','re32',interpolate,"Brightness","",
		'SUBS','in32',noflags,"Sub Shader Use","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (331) {
{
		R_IID_I3DExShader,
		R_CLSID_GIShadowCatcher
}
};

resource 'COMP' (332) {
	kRID_ShaderFamilyID,
	'SPFA',
	"Alternative Luster",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (332) {
	{
		'XROU','re32',interpolate,"X Roughness","",
		'YROU','re32',interpolate,"Y Roughness","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
		
	}
};


resource 'GUID' (332) {
{
		R_IID_I3DExShader,
		R_CLSID_FauxAnisotropic
}
};

resource 'COMP' (333) {
	kRID_ShaderFamilyID,
	'SPPL',
	"Procedural Lock",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (333) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (333) {
{
		R_IID_I3DExShader,
		R_CLSID_ProceduralLock
}
};

resource 'COMP' (334) {
	kRID_ShaderFamilyID,
	'SPBV',
	"Bevel",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (334) {
	{
		'SIZE', 're32', interpolate,"Bevel Size","",
		'SPCE', 'in32', noflags,"Space","",
		'EDGT', 're32', noflags, "Angle", "",
		'FALL', 'actn', noflags, "Bevel", "",
		'EINN', 'bool', noflags, "Inner", "",
		'EOUT', 'bool', noflags, "Outer", "",
		'GRSA', 'bool', noflags, "Anything Grows safe mode", "",
		'SOFT', 'bool', noflags, "Soften Points", "",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (334) {
{
		R_IID_I3DExShader,
		R_CLSID_Bevel
}
};
/*
resource 'COMP' (335) {
	kRID_ShaderFamilyID,
	'SPIC',
	"Illumination Catcher",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (335) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (335) {
{
		R_IID_I3DExShader,
		R_CLSID_IlluminationCatcher
}
};

resource 'COMP' (336) {
	kRID_ShaderFamilyID,
	'SPIL',
	"Illumination Level",
	"Shaders Plus",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


//resource 'PMap' (336) {
//	{
//	}
//};


resource 'GUID' (336) {
{
		R_IID_I3DExShader,
		R_CLSID_IlluminationLevel
}
};
*/
resource 'COMP' (337) {
	kRID_ShaderFamilyID,
	'SPSD',
	"Lighting Control",
	"Shaders Plus Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (337) {
	{
		'AMBI', 're32', noflags, "Ambient", "",
		'GILI', 're32', noflags, "GI", "",
		'SYNC', 'bool', noflags,"Synch all lights","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (337) {
{
		R_IID_I3DExShader,
		R_CLSID_SpecularDiffuseControl
}
};
