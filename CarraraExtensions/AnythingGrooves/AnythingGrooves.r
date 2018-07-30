/*  Anything Grooves - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#include "DefMapDef.h"
#include "LockMasterShaderDef.h"
#include "AGRPrimDef.h"
#include "AGRConvertToMeshDef.h"
#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "AnythingGrooves.rsr";

#define COPYRIGHT "Copyright © 2000 Digital Carvers Guild"
#define DCGGROUP "Digital Carvers Guild Functions"
#define DCGMODGROUP "Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (308) {
	kRID_ModifierFamilyID,
	'GIBB',
	"Anything Grooves",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (308) {
	{	
		'STAR','re32',interpolate,"Start","",
		'STOP','re32',interpolate,"Stop","",
		'SMTH','re32',interpolate,"Smooth Angle","",
		'CHAN','re32',interpolate,"Animate","",
		'WIDT','in32',interpolate,"U Samples","",
		'HEIG','in32',interpolate,"V Samples","",
		'PRES','bool',noflags,"Presmooth", "",
		'SHAD','s255',noflags,"Master Shader","",
		'WARN','s255',noflags,"Warnings","",
		'REFC','bool',noflags,"Refresh Flag", "",
		'ADAP','bool',noflags,"Adaptive Mesh", "",
		'ENAB','bool',noflags,"Enabled", "",
		'QUAD','in32',noflags, "Quad Split","",
		'EMPT','bool',noflags,"Empty zero facets", "",
		'OPIC','actn',noflags,"Object Picker","",
	}
};


resource 'GUID' (308) {
{
		R_IID_I3DExModifier,
		R_CLSID_DefMap
}
};

resource 'COMP' (309)
{
	kRID_SceneCommandFamilyID,
	'LOAG',
	"Lock Anything Grooves Master Shaders",
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
		R_CLSID_LockMasterShader
	}
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (309) 
{
	kThirdPartiesCmdIDBase + kCMDIdLockAnythingGroovesMasterShaders, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (309) 
{
	-1, -1,{'3Dvw'}
};
#endif

resource 'GUID' (310)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_AGrPrim
	}
};

resource 'COMP' (310)
{
	kRID_GeometricPrimitiveFamilyID,
	'AGRP',
	"Anything Grooves",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (310) {
	{	
		'SHAP','in32',noflags, "Shape","",
		'STAR','re32',interpolate,"Start","",
		'STOP','re32',interpolate,"Stop","",
		'SMTH','re32',interpolate,"Smooth Angle","",
		'CHAN','re32',interpolate,"Animate","",
		'WIDT','in32',interpolate,"U Samples","",
		'HEIG','in32',interpolate,"V Samples","",
		'PRES','bool',noflags,"Presmooth", "",
		'SHAD','s255',noflags,"Master Shader","",
		'WARN','s255',noflags,"Warnings","",
		'REFC','bool',noflags,"Refresh Flag", "",
		'ADAP','bool',noflags,"Adaptive Mesh", "",
		'ENAB','bool',noflags,"Enabled", "",
		'QUAD','in32',noflags, "Quad Split","",
		'SIZX','re32',interpolate,"X","",
		'SIZY','re32',interpolate,"Y","",
		'SIZZ','re32',interpolate,"Z","",
		'EMPT','bool',noflags,"Empty zero facets", "",
		'PREV','in32',noflags,"Preview Size","",
		'OPIC','actn',noflags,"Object Picker","",
	}
};

resource 'COMP' (320)
{
	kRID_SceneCommandFamilyID,
	'COAG',
	"Convert Deformed Object to Mesh",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (320)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_AGrConvertToMesh
	}
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (320) 
{
	kThirdPartiesCmdIDBase + kCMDIdConvertDeformedObjectToMesh, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (320) 
{
	-1, -1,{'3Dvw'}
};
#endif
