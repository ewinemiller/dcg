/*  Parchment - plug-in for Carrara
    Copyright (C) 2002 Michael Clarke

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
#include "ParchmentDef.h"
#include "interfaceids.h"


#if (VERSIONNUMBER >= 0x030000)
include "ParchmentC3.rsr";
#else
include "Parchment.rsr";
#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'GUID' (2000)
{
	{
		R_IID_I3DExPostRenderer,
		R_CLSID_Parchment
	}
};

resource 'COMP' (2000)
{
	kRID_PostRendererFamilyID,
	'PRCH',
	"Parchment",
	"Miks",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource 'PMap' (2000)
{
	{
	'TXST','re32', interpolate, "Texture Strength","",
	'TXOT','re32', interpolate, "Texture Overlay Threshold","",
	
	'GRAN','re32', interpolate, "Gradient Angle","",
	'GRLD','re32', interpolate, "Gradient Lead","",
	'GRWD','re32', interpolate, "Gradient Width","",
	'GRCX','re32', interpolate, "Gradient Center X","",
	'GRCY','re32', interpolate, "Gradient Center Y","",

	'DIFD','re32', interpolate, "Fade Distance","",
	'DIFL','re32', interpolate, "Fade Lead Distance","",
	
	'DIFX','in32', noFlags, "Distance Effect","",
	'DITG','in32', noFlags, "Distance Target","",

	'TXSC','in32', noFlags, "Texture Source","",
	'TXMT','in32', noFlags, "Texture Method","",
	'TXCH','in32', noFlags, "Texture Channel","",

	'TXCR','colo', interpolate, "Chromakey colour","",


	'GRTC','bool', noFlags, "Gradient True Circle","",
	'GRIN','bool', noFlags, "Gradient Invert","",
	'GRMR','bool', noFlags, "Gradient Mirror","",

	'ASHI','bool', noFlags, "Invert Mask","",
	'ASHM','s255', noFlags, "Apply Shader Mask","",

	'SHIN','bool', noFlags, "Invert Shader Texture","",
	'SHNM','s255', noFlags, "Shader Texture Name","",
	
	'SSED','in32', noFlags, "PRN Seed","",
	'SLEN','re32', noFlags, "SCP Length","",
	
	'SFIV','re32', noFlags, "Flicker Interval","",
	'SFDR','re32', noFlags, "Flicker Duration","",
	'SFIN','re32', noFlags, "Flicker Intensity","",
	'SFCO','colo', interpolate, "Flicker Colour","",
	'TEX1','TMap', noFlags,	"Texture Map","",
	}
};
