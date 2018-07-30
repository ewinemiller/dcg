/*  Toon! Pro - plug-in for Carrara
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

#include "CellDef.h"
#include "SetCellLightingModelDef.h"
#include "../WirerendererCommon/WireRendererDef.h"
#include "../WirerendererCommon/ToonEnabledDef.h"

#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "ToonPro.rsr";

#define COPYRIGHT "Copyright � 2004 Digital Carvers Guild"
#define DCGOPGROUP "Toon! Pro"

resource 'COMP' (325) {
	kRID_ShaderFamilyID,
	'SPCE',
	"Toon! Pro Cel",
	"Toon! Pro Lighting Models # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (325) {
	{
		'SHAD','vec2',interpolate,"Shadow and Highlight","",
		'LVLS','in32',noflags,"Levels","",
		'SHBR','re32',interpolate,"Shadow Brightness","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 3 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (325) {
{
		R_IID_I3DExShader,
		R_CLSID_Cel
}
};

resource 'CPUI' (325) 
{
	325,					// Id of your main part
	1325,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (134)
{
	kRID_FinalRendererFamilyID,        // extension type =renderer
	'SPTO',        // Class ID
	"Toon! Pro",
	"Required",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (134)
{
	{
	R_IID_I3DExFinalRenderer,
	R_CLSID_ToonRenderer
	}
};

resource 'PMap' (134) 
{
	{
#if VERSIONNUMBER >= 0x070000
		'Shdw', 'bool', noflags, "", "",
		'Flec', 'bool', noflags, "", "",
		'Tran', 'bool', noflags, "", "",
		'LTTO', 'bool', noflags, "", "",
		'BUMP', 'bool', noflags, "", "",
		'FRAC', 'bool', noflags, "", "",
		'Radi', 'bool', noflags, "", "",
		'Caus', 'bool', noflags, "", "",
		'PHmp', 'bool', noflags, "", "",
		'uGam', 'bool', noflags, "", "",
		'SkyL', 'bool', noflags, "", "",
		'IEAc', 'bool', noflags, "", "",
		'irrC', 'bool', noflags, "", "",
		'sepS', 'bool', noflags, "", "",
		'Gamm', 're32', noflags, "", "",
		'MaxD', 'in32', noflags, "", "",
		'GItp', 'in32', noflags, "", "",
		'Qual', 'in32', noflags, "", "",
		'ILin', 're32', noflags, "", "",
		'SkyI', 're32', noflags, "", "",
		'CAin', 're32', noflags, "", "",
		'Intr', 're32', noflags, "", "",
		'AmbR', 're32', noflags, "", "",		
		'atmD', 'bool', noflags, "", "",
		'vlpD', 'bool', noflags, "", "",
		'vleD', 'bool', noflags, "", "",
		'OVER', 'in32', noflags, "", "",
		'Oacc', 'in32', noflags, "", "",
		'Lacc', 'in32', noflags, "", "",
		'Iacc', 'in32', noflags, "", "",
		'Dpht', 'in32', noflags, "", "",
		'Cpht', 'in32', noflags, "", "",
		'cfil', 're32', noflags, "", "",
		'FltB', 're32', noflags, "", "",
		'rayT', 'bool', noflags, "", "",
		'dofd', 'bool', noflags, "", "",
		'csfm', 'bool', noflags, "", "",
		'phtA', 're32', noflags, "", "",
		'IrrM', 'in32', noflags, "", "",
		'SavM', 'bool', noflags, "", "",
		'GIlt', 'bool', noflags, "", "",
		'iMap', 'Dstr', noflags, "", "",
		'sMap', 'Dstr', noflags, "", "",
#elif VERSIONNUMBER >= 0x050000
		'Shdw', 'bool', noflags, "", "",
		'Flec', 'bool', noflags, "", "",
		'Tran', 'bool', noflags, "", "",
		'LTTO', 'bool', noflags, "", "",
		'BUMP', 'bool', noflags, "", "",
		'FRAC', 'bool', noflags, "", "",
		'Radi', 'bool', noflags, "", "",
		'Caus', 'bool', noflags, "", "",
		'PHmp', 'bool', noflags, "", "",
		'uGam', 'bool', noflags, "", "",
		'SkyL', 'bool', noflags, "", "",
		'IEAc', 'bool', noflags, "", "",
		'irrC', 'bool', noflags, "", "",
		'Gamm', 're32', noflags, "", "",
		'MaxD', 'in32', noflags, "", "",
		'GItp', 'in32', noflags, "", "",
		'Qual', 'in32', noflags, "", "",
		'ILin', 're32', noflags, "", "",
		'SkyI', 're32', noflags, "", "",
		'CAin', 're32', noflags, "", "",
		'Intr', 're32', noflags, "", "",
		'AmbR', 're32', noflags, "", "",		
		'OVER', 'in32', noflags, "", "",
		'Oacc', 'in32', noflags, "", "",
		'Lacc', 'in32', noflags, "", "",
		'Iacc', 'in32', noflags, "", "",
		'Dpht', 'in32', noflags, "", "",
		'Cpht', 'in32', noflags, "", "",
		'cfil', 're32', noflags, "", "",
		'FltB', 're32', noflags, "", "",
		'rayT', 'bool', noflags, "", "",
		'dofd', 'bool', noflags, "", "",
		'phtA', 're32', noflags, "", "",
		'IrrM', 'in32', noflags, "", "",
		'SavM', 'bool', noflags, "", "",
		'GIlt', 'bool', noflags, "", "",
		'iMap', 'Dstr', noflags, "", "",
		'sMap', 'Dstr', noflags, "", "",
#else
		'Shdw', 'bool', noflags, "", "",
		'Flec', 'bool', noflags, "", "",
		'Tran', 'bool', noflags, "", "",
		'LTTO', 'bool', noflags, "", "",
		'BUMP', 'bool', noflags, "", "",
		'FRAC', 'bool', noflags, "", "",
		'Radi', 'bool', noflags, "", "",
		'Caus', 'bool', noflags, "", "",
		'PHmp', 'bool', noflags, "", "",
		'uGam', 'bool', noflags, "", "",
		'SkyL', 'bool', noflags, "", "",
		'IEAc', 'bool', noflags, "", "",
		'irrC', 'bool', noflags, "", "",
		'Gamm', 're32', noflags, "", "",
		'MaxD', 'in32', noflags, "", "",
		'Qual', 'in32', noflags, "", "",
		'ILin', 're32', noflags, "", "",
		'SkyI', 're32', noflags, "", "",
		'CAin', 're32', noflags, "", "",
		'Intr', 're32', noflags, "", "",
		'OVER', 'in32', noflags, "", "",
		'Oacc', 'in32', noflags, "", "",
		'Lacc', 'in32', noflags, "", "",
		'Iacc', 'in32', noflags, "", "",
		'Dpht', 'in32', noflags, "", "",
		'Cpht', 'in32', noflags, "", "",
		'cfil', 're32', noflags, "", "",
		'FltB', 're32', noflags, "", "",
		'rayT', 'bool', noflags, "", "",
		'dofd', 'bool', noflags, "", "",
		'phtA', 're32', noflags, "", "",
#endif
		'line', 're32', noflags, "", "",
		'OUTL', 'actn', noflags, "", "",
		'EDGT', 're32', noflags, "", "",
		'OVDR', 're32', noflags, "", "",
		'DEPT', 'bool', noflags, "", "",
		'DIST', 're32', noflags, "", "",
		'BKCL', 'bool', noflags, "", "",
		'BUCK', 'in32', noflags, "", "",
		'LICO', 'colo', noflags, "", "",
		'LIVL', 're32', noflags, "", "",
		'RHID', 'bool', noflags, "", "",
		'DRWB', 'bool', noflags, "", "",
		'SMQA', 'bool', noflags, "Smart Quads", "",
		'EDGE', 'bool', noflags, "Filter Edges", "",
		'WFOV', 'actn', noflags, "Oversampling", "",
		'VERT', 'bool', noflags, "Vertex Edges", "",
		'SILO', 'bool', noflags, "Silhouette Edges", "",
		'OBJC', 'bool', noflags, "Use Object Color", "",
		'BARE', 'bool', noflags, "Base Render", "",
		'DOLI', 'bool', noflags, "Domain Boundary Lines", "",
		'ALPA', 'actn', noflags, "Alpha Content", "",
	}
};

resource 'COMP' (127) {
	'data',
	'TPEN',
	"Toon! Pro Override",
	DCGOPGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (127) {
	{
	'OVER','bool',noflags,"Override","",
	'LINE','re32',interpolate,"Line Effect","",
	'SYNC','bool',noflags,"Synch All","",
	}
	};
	
resource 'GUID' (127) {
{
		R_IID_I3DExDataComponent,
		R_CLSID_ToonEnabled
}
};

resource 'data' (127) {
	{
//	'li  ',	// light
//	'ca  ', // camera
	'prim', // instanciable primitive
	'grou', // group
// 'sc  '  // scene settings
	}
};

resource 'PMap' (257) {
	{
	'RayN','in32',noflags,"RayN","",
	'FacN','in32',noflags,"FacN","",
	}
	};

#if (VERSIONNUMBER >= 0x050000)
resource 'COMP' (309)
{
	kRID_SceneCommandFamilyID,
	'TPSC',
	"Set Cel Lighting Model",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (309)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_SetCellLightingModel
	}
};

resource 'scmd' (309) 
{
	kThirdPartiesCmdIDBase + kCMDIdToonApplyCell, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};

resource 'PMap' (309) {
	{	
		'SHAD','vec2',interpolate,"Shadow and Highlight","",
		'LVLS','in32',noflags,"Levels","",
		'SHBR','re32',interpolate,"Shadow Brightness","",
	}
};
#endif

/*resource 'COMP' (327) {
	kRID_ShaderFamilyID,
	'TPLI',
	"Toon! Pro Lines",
	"Toon! Pro Lines",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (327) {
	{
		'SIZE', 're32', interpolate,"Line Size","",
		'SPCE', 'in32', noflags,"Space","",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		
		'DOED', 'bool', noflags, "Do Angle Edges", "",
		'EDGT', 're32', noflags, "Angle", "",
		'DOLI', 'bool', noflags, "Do Shading Domain Boundaries", "",
		'DOPR', 'bool', noflags, "Do Profile Edges", "",
		'DOPE', 'bool', noflags, "Do Profile Edges", "",
	}
};


resource 'GUID' (327) {
{
		R_IID_I3DExShader,
		R_CLSID_ProfileShader
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
*/