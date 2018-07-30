/*  Cognito - plug-in for Carrara
    Copyright (C) 2001 Michael Clarke

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
#include "Copyright.h"
#include "CognitoDef.h"

#undef COPYRIGHT
#define COPYRIGHT "(C) Michael Clarke, 2001"

#include "External3DAPI.r"

#include "InterfaceIDs.h"

include "Cognito.rsr";

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

// Cognito

resource 'COMP' (203)
{
	kRID_ModifierFamilyID,
	'COGN',
	"Cognito",
	"Digital Carvers Guild# 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (203)
{
	{
		R_IID_I3DExModifier,
		R_CLSID_Cognito
	}
};

resource 'PMap' (203)
{
	{
		'VECX','re32', interpolate, "Vector X","", 
		'VECY','re32', interpolate, "Vector Y","", 
		'VECZ','re32', interpolate, "Vector Z","", 
		'RADS','re32', interpolate, "Radius","", 
		'ARNX','bool', noFlags, "Around X","", 
		'ARNY','bool', noFlags, "Around Y","", 
		'ARNZ','bool', noFlags, "Around Z","", 
		'HNTV','s255', noFlags, "Hint for Vector","", 
		'HNTP','s255', noFlags, "Hint for Parameter","", 
		'HNTH','s255', noFlags, "Hint for Hot Point","", 
		'HNTR','s255', noFlags, "Hint for Parameter2","", 
		'RADT','re32', interpolate, "2ND Radius","", 
	}
};


// Cognito Data

resource 'data' (200) 
{
	{
		'li  ', // light
		'ca  ', // camera
		'prim', // instanciable primitive
		'grou', // group
	}
};

resource 'COMP' (200)
{
	'data',
	'COGD',
	"Cognito Data",
	"Miks",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (200)
{
	{
		'ROTA','re32', interpolate, "Rotation","", 
		'PTCH','re32', interpolate, "Worm Pitch","", 
		'OFFS','re32', interpolate, "Offset","", 
		'LINK','in32', noFlags, "Linkage","", 
		'TETH','in32', interpolate, "Teeth","", 
		'MOTR','bool', noFlags, "Is Motor","", 
		'SLVE','bool', noFlags, "Is Slave","", 
		'DRVR','s255', noFlags, "Drivers Name","", 
	}
};

resource 'GUID' (200)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_CognitoData
	}
};

