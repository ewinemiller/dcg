/*  Anything Grows - plug-in for Carrara
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
#include "AnythingGrowsDeformerDef.h"
#include "AnythingGrowsPrimitiveDef.h"
#include "LockMasterShaderDef.h"
#include "BillboardDef.h"
#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "AnythingGrows.rsr";

#define COPYRIGHT "Copyright © 2000 Digital Carvers Guild"
#define DCGGROUP "Digital Carvers Guild Functions"
#define DCGMODGROUP "Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (308) {
	kRID_ModifierFamilyID,
	'GROD',
	"Anything Grows",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (308) {
	{	
		'STAR','re32',1,"Width","",
		'STOP','re32',1,"Length","",
		'CHAN','re32',1,"Animate","",
		'SHAD','s255',0,"Length Shader","",
		'WSHA','s255',0,"Width Shader","",
		'WARN','s255',0,"Warnings","",
		'REFC','bool',0,"Refresh Flag", "",
		'ENAB','bool',0,"Enabled", "",
		'EMPT','bool',0,"Show Base", "",
		'SHST','bool',0,"Show Strands", "",
		'MINS','bool',0,"At least one strand", "",
		'FACM','bool',0,"Face Camera", "",
		'BONE','bool',0,"Bone Mode", "",
		'SPAC','in32',0,"Space","",
		'HAIR','in32',0,"Number of hairs","",
		'SEED','in32',1,"Seed","",
		'SEGM','in32',0,"Segments","",
		'STEF','in32',1,"Start Effects","",
		'SIDE','in32',1,"Sides","",
		'GRAV','re32',1,"Gravity","",
		'GRAX','re32',1,"Gravity X","",
		'GRAY','re32',1,"Gravity Y","",
		'GRAZ','re32',1,"Gravity Z","",
		'TAPR','re32',1,"Taper","",
		'WIGL','re32',1,"Wiggle","",
		'STIF','re32',1,"Stiffness","",
		'TSIS','re32',1,"Tip Scale","",
		'CUSS','re32',1,"Custom Vector","",
		'OBJT','s255',0,"Object","",
		'REFO','s255',0,"Reference Object","",
		'TOBJ','s255',0,"Tip Object","",
		'SSHA','s255',0,"Stiffness Shader","",
		'XSHA','s255',0,"X Wiggle Shader","",
		'YSHA','s255',0,"Y Wiggle Shader","",
		'ZSHA','s255',0,"Z Wiggle Shader","",
		'TSIZ','s255',0,"Tip Scale Shader","",
		'TTWS','s255',0,"Tip Twist Shader","",
		'XCUS','s255',0,"X Custom Vector Shader","",
		'YCUS','s255',0,"Y Custom Vector Shader","",
		'ZCUS','s255',0,"Z Custom Vector Shader","",
		'DOMN','s255',0,"Domain","",
		'TAPM','in32',0,"Taper Mode","",
		'PREV','in32',0,"Preview Mode","",
		'STDM','in32',0,"Strand Distribution Mode","",
		'UVMP','in32',0,"UV Mapping","",
		'VRUN','in32',0,"V Runs Along","",
		'LSHP','actn',0,"Length Shader Picker","",
		'WSHP','actn',0,"Width Shader Picker","",
		'OBJP','actn',0,"Object Picker","",
		'REFP','actn',0,"Reference Object Picker","",
		'SSHP','actn',0,"Stiffness Shader Picker","",
		'XSHP','actn',0,"X Wiggle Shader Picker","",
		'YSHP','actn',0,"Y Wiggle Shader Picker","",
		'ZSHP','actn',0,"Z Wiggle Shader Picker","",
		'TOBP','actn',0,"Tip Object Picker","",
		'TSIP','actn',0,"Tip Scale Picker","",
		'TTWP','actn',0,"Tip Twist Picker","",
		'XCUP','actn',0,"X Custom Vector Shader Picker","",
		'YCUP','actn',0,"Y Custom Vector  Shader Picker","",
		'ZCUP','actn',0,"Z Custom Vector  Shader Picker","",
		'DOMP','actn',0,"Domain Picker","",
	}
};


resource 'GUID' (308) {
{
		R_IID_I3DExModifier,
		R_CLSID_AnythingGrowsDeformer
}
};


resource 'GUID' (310)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_AnythingGrowsPrimitive
	}
};

resource 'COMP' (310)
{
	kRID_GeometricPrimitiveFamilyID,
	'GROP',
	"Anything Grows",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (310) {
	{	
		'STAR','re32',1,"Width","",
		'STOP','re32',1,"Length","",
		'CHAN','re32',1,"Animate","",
		'SHAD','s255',0,"Length Shader","",
		'WSHA','s255',0,"Width Shader","",
		'WARN','s255',0,"Warnings","",
		'REFC','bool',0,"Refresh Flag", "",
		'ENAB','bool',0,"Enabled", "",
		'EMPT','bool',0,"Show Base", "",
		'SHST','bool',0,"Show Strands", "",
		'MINS','bool',0,"At least one strand", "",
		'FACM','bool',0,"Face Camera", "",
		'BONE','bool',0,"Bone Mode", "",
		'SPAC','in32',0,"Space","",
		'HAIR','in32',0,"Number of hairs","",
		'SEED','in32',1,"Seed","",
		'SEGM','in32',0,"Segments","",
		'STEF','in32',1,"Start Effects","",
		'SIDE','in32',1,"Sides","",
		'GRAV','re32',1,"Gravity","",
		'GRAX','re32',1,"Gravity X","",
		'GRAY','re32',1,"Gravity Y","",
		'GRAZ','re32',1,"Gravity Z","",
		'TAPR','re32',1,"Taper","",
		'WIGL','re32',1,"Wiggle","",
		'STIF','re32',1,"Stiffness","",
		'TSIS','re32',1,"Tip Scale","",
		'CUSS','re32',1,"Custom Vector","",
		'OBJT','s255',0,"Object","",
		'REFO','s255',0,"Reference Object","",
		'TOBJ','s255',0,"Tip Object","",
		'SSHA','s255',0,"Stiffness Shader","",
		'XSHA','s255',0,"X Wiggle Shader","",
		'YSHA','s255',0,"Y Wiggle Shader","",
		'ZSHA','s255',0,"Z Wiggle Shader","",
		'TSIZ','s255',0,"Tip Scale Shader","",
		'TTWS','s255',0,"Tip Twist Shader","",
		'XCUS','s255',0,"X Custom Vector Shader","",
		'YCUS','s255',0,"Y Custom Vector Shader","",
		'ZCUS','s255',0,"Z Custom Vector Shader","",
		'DOMN','s255',0,"Domain","",
		'TAPM','in32',0,"Taper Mode","",
		'PREV','in32',0,"Preview Mode","",
		'STDM','in32',0,"Strand Distribution Mode","",
		'UVMP','in32',0,"UV Mapping","",
		'VRUN','in32',0,"V Runs Along","",
		'LSHP','actn',0,"Length Shader Picker","",
		'WSHP','actn',0,"Width Shader Picker","",
		'OBJP','actn',0,"Object Picker","",
		'REFP','actn',0,"Reference Object Picker","",
		'SSHP','actn',0,"Stiffness Shader Picker","",
		'XSHP','actn',0,"X Wiggle Shader Picker","",
		'YSHP','actn',0,"Y Wiggle Shader Picker","",
		'ZSHP','actn',0,"Z Wiggle Shader Picker","",
		'TOBP','actn',0,"Tip Object Picker","",
		'TSIP','actn',0,"Tip Scale Picker","",
		'TTWP','actn',0,"Tip Twist Picker","",
		'XCUP','actn',0,"X Custom Vector Shader Picker","",
		'YCUP','actn',0,"Y Custom Vector  Shader Picker","",
		'ZCUP','actn',0,"Z Custom Vector  Shader Picker","",
		'DOMP','actn',0,"Domain Picker","",
	}
};

resource 'COMP' (309)
{
	kRID_SceneCommandFamilyID,
	'LAGO',
	"Lock Anything Grows Master Shaders",
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
	kThirdPartiesCmdIDBase + kCMDIdLockAnythingGrowsMasterShaders, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (309) 
{
	-1, -1,{'3Dvw'}
};
#endif

//for modifier
resource 'TABS' (1000)
{
	{
		402, "Basic", 402,
		403, "Gravity", 403,
		400, "Bend", 400,
		401, "Tip", 401,
		404, "UV", 404,
	}
};
//for primitive
resource 'TABS' (1001)
{
	{
		402, "Basic", 402,
		403, "Gravity", 403,
		400, "Bend", 400,
		401, "Tip", 401,
		404, "Mapping", 404,
	}
};

resource 'GUID' (311)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_Billboard
	}
};
resource 'COMP' (311)
{
	kRID_GeometricPrimitiveFamilyID,
	'BILL',
	"Billboard",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (311) 
{
	{	
		'WDTH','re32',1,"Width","",
		'HGHT','re32',1,"Height","",
		'SIDE','in32',1,"Sides","",
		'ZSTR','re32',1,"Z Start","",
	}
};