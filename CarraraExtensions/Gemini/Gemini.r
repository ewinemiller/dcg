/*  Project Gemini - plug-in for Carrara
    Copyright (C) 2002 Eric Winemiller

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
#include "GeminiDeformerDef.h"
#include "GeminiCommandDef.h"
#include "EraserDef.h"

#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "Gemini.rsr";

#define COPYRIGHT "Copyright © 2002 Digital Carvers Guild"
#define DCGGROUP "Digital Carvers Guild Functions"
#define DCGMODGROUP "Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (308) {
	kRID_ModifierFamilyID,
	'GEMI',
	"Gemini",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (308) {
	{	
		'ENAB','bool',0,"Enabled", "",
		'KEEP','in32',0,"Keep","",
		'MOER','re32',1,"Margin of Error","",
	}
};


resource 'GUID' (308) {
{
		R_IID_I3DExModifier,
		R_CLSID_GeminiDeformer
}
};

resource 'COMP' (309) {
	kRID_ModifierFamilyID,
	'ERAS',
	"Eraser",
	DCGMODGROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (309) {
	{	
		'ENAB','bool',0,"Enabled", "",
		'KEEP','in32',0,"Keep","",
		'ERAS','re32',1,"Erase","",
	}
};


resource 'GUID' (309) {
{
		R_IID_I3DExModifier,
		R_CLSID_Eraser
}
};

resource 'COMP' (320)
{
	kRID_SceneCommandFamilyID,
	'COGE',
	"Gemini",
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
		R_CLSID_GeminiCommand
	}
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (320) 
{
	kThirdPartiesCmdIDBase + kCMDIdGemini, -1,{'3Dvw'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (320) 
{
	-1, -1,{'3Dvw'}
};
#endif

resource 'PMap' (320) {
	{	
		'KEEP','in32',0,"Keep","",
		'MOER','re32',1,"Margin of Error","",
		'ORIG','bool',0,"Keep Original Object", "",
	}
};

/*resource 'COMP' (321)
{
	kRID_SceneCommandFamilyID,
	'GEVM',
	"Gemini VM",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (321)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_GeminiVM
	}
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (321) 
{
	kThirdPartiesCmdIDBase + kCMDIdGeminiVM, -1,{'modm'}, kNoStrings, kDefaultName, kNoGroup, {kDefaultID, kDefaultName, kNoChar, kNoShift, kNoCtrl, kNoAlt, kAnyPlatform}, kNoContext
};
#else
resource 'scmd' (321) 
{
	-1, -1,{'modm'}
};
#endif

resource 'PMap' (321) {
	{	
		'KEEP','in32',noflags,"Keep","",
	}
};*/
