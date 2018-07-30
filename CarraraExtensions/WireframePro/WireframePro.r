/*  Wireframe Pro - plug-in for Carrara
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

#include "WireframeDef.h"
#include "../WirerendererCommon/WireRendererDef.h"
#include "../WirerendererCommon/ToonEnabledDef.h"

include "WireFramePro.rsr";

#define COPYRIGHT "Copyright © 2004 Digital Carvers Guild"
#define DCGOPGROUP "Shaders Plus"

resource 'COMP' (135)
{
	kRID_FinalRendererFamilyID,        // extension type =renderer
	'SPWR',        // Class ID
	"Wireframe Pro",
	"Required",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (135)
{
	{
	R_IID_I3DExFinalRenderer,
	R_CLSID_WireRenderer
	}
};

resource 'PMap' (135) 
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

resource 'COMP' (327) {
	kRID_ShaderFamilyID,
	'SPWF',
	"Wireframe Pro",
	"Wireframe Pro",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (327) {
	{
		'SIZE', 're32', interpolate,"Wire Size","",
		'SPCE', 'in32', noflags,"Space","",
		'EDGT', 're32', noflags, "Angle", "",
		'SMQA', 'bool', noflags, "Smart Quads", "",
		'EDGE', 'bool', noflags, "Filter Edges", "",
		'FALL', 'actn', noflags, "Falloff", "",
		'INVT', 'bool', noflags, "Invert", "",
		'VERT', 'bool', noflags, "Vertex Edges", "",
	}
};


resource 'GUID' (327) {
{
		R_IID_I3DExShader,
		R_CLSID_Wireframe
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

resource 'COMP' (127) {
	'data',
	'WFEN',
	"Wireframe Pro Override",
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
		R_CLSID_WireframeEnabled
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
/*
resource 'COMP' (331) {
	kRID_ShaderFamilyID,
	'WPAC',
	"Wireframe Pro Helper",
	"Wireframe Pro # 16",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (331) {
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 18 bnon 0 Subm 1 Sort 1 Mini 1 }",
	}
};


resource 'GUID' (331) {
{
		R_IID_I3DExShader,
		R_CLSID_WireframeAccelerator
}
};
*/

resource 'PMap' (257) {
	{
	'RayN','in32',noflags,"RayN","",
	'FacN','in32',noflags,"FacN","",
	}
	};
