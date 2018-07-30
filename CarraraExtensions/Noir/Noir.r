/*  Noir - plug-in for Carrara
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
#include "interfaceids.h"

#include "NoirDef.h"


#if (VERSIONNUMBER >= 0x030000)
include "NoirC3.rsr";
#else
include "Noir.rsr";
#endif
#define DCGMODGROUP "Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'GUID' (2000)
{
	{
		R_IID_I3DExPostRenderer,
		R_CLSID_Noir
	}
};

resource 'COMP' (2000)
{
	kRID_PostRendererFamilyID,
    'NOIR',
	"Noir",
	"Miks",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource 'PMap' (2000)
{
	{
	'EXPR','re32', interpolate, "Exposure Strength","",
	'EXPM','re32', interpolate, "Exposure Master","",
	'EXP0','re32', interpolate, "Exposure   0","",
	'EXP1','re32', interpolate, "Exposure  10","",
	'EXP2','re32', interpolate, "Exposure  20","",
	'EXP3','re32', interpolate, "Exposure  30","",
	'EXP4','re32', interpolate, "Exposure  40","",
	'EXP5','re32', interpolate, "Exposure  50","",
	'EXP6','re32', interpolate, "Exposure  60","",
	'EXP7','re32', interpolate, "Exposure  70","",
	'EXP8','re32', interpolate, "Exposure  80","",
	'EXP9','re32', interpolate, "Exposure  90","",
	'EXPA','re32', interpolate, "Exposure 100","",
	
	'REDR','re32', interpolate, "Red Strength","",
	'REDM','re32', interpolate, "Red Master","",
	'RED0','re32', interpolate, "Red   0","",
	'RED1','re32', interpolate, "Red  10","",
	'RED2','re32', interpolate, "Red  20","",
	'RED3','re32', interpolate, "Red  30","",
	'RED4','re32', interpolate, "Red  40","",
	'RED5','re32', interpolate, "Red  50","",
	'RED6','re32', interpolate, "Red  60","",
	'RED7','re32', interpolate, "Red  70","",
	'RED8','re32', interpolate, "Red  80","",
	'RED9','re32', interpolate, "Red  90","",
	'REDA','re32', interpolate, "Red 100","",
	
	'GRNR','re32', interpolate, "Green Strength","",
	'GRNM','re32', interpolate, "Green Master","",
	'GRN0','re32', interpolate, "Green   0","",
	'GRN1','re32', interpolate, "Green  10","",
	'GRN2','re32', interpolate, "Green  20","",
	'GRN3','re32', interpolate, "Green  30","",
	'GRN4','re32', interpolate, "Green  40","",
	'GRN5','re32', interpolate, "Green  50","",
	'GRN6','re32', interpolate, "Green  60","",
	'GRN7','re32', interpolate, "Green  70","",
	'GRN8','re32', interpolate, "Green  80","",
	'GRN9','re32', interpolate, "Green  90","",
	'GRNA','re32', interpolate, "Green 100","",
	
	'BLUR','re32', interpolate, "Blue Strength","",
	'BLUM','re32', interpolate, "Blue Master","",
	'BLU0','re32', interpolate, "Blue   0","",
	'BLU1','re32', interpolate, "Blue  10","",
	'BLU2','re32', interpolate, "Blue  20","",
	'BLU3','re32', interpolate, "Blue  30","",
	'BLU4','re32', interpolate, "Blue  40","",
	'BLU5','re32', interpolate, "Blue  50","",
	'BLU6','re32', interpolate, "Blue  60","",
	'BLU7','re32', interpolate, "Blue  70","",
	'BLU8','re32', interpolate, "Blue  80","",
	'BLU9','re32', interpolate, "Blue  90","",
	'BLUA','re32', interpolate, "Blue 100","",
	
	'RGRY','re32', interpolate, "Pre GrayScale","",
	'RNEG','re32', interpolate, "Pre Negative","",
	'RSOL','re32', interpolate, "Pre Solar","",
	'RSOT','re32', interpolate, "Pre Solar Threshold","",
	'RQNT','re32', interpolate, "Pre Quantize","",
	'RQNL','in32', interpolate, "Pre Quantize Levels","",
	'RTNT','re32', interpolate, "Pre Tint","",
	'RTNC','colo', interpolate, "Pre Tint Colour","",
	
	'SGRY','re32', interpolate, "Post GrayScale","",
	'SNEG','re32', interpolate, "Post Negative","",
	'SSOL','re32', interpolate, "Post Solar","",
	'SSOT','re32', interpolate, "Post Solar Threshold","",
	'SQNT','re32', interpolate, "Post Quantize","",
	'SQNL','in32', interpolate, "Post Quantize Levels","",
	'STNT','re32', interpolate, "Post Tint","",
	'STNC','colo', interpolate, "Post Tint Colour","",
	
	'GRAN','re32', interpolate, "Gradient Angle","",
	'GRLD','re32', interpolate, "Gradient Lead","",
	'GRWD','re32', interpolate, "Gradient Width","",
	'GRCX','re32', interpolate, "Gradient Center X","",
	'GRCY','re32', interpolate, "Gradient Center Y","",

	'DIFD','re32', interpolate, "Fade Distance","",
	'DIFL','re32', interpolate, "Fade Lead Distance","",
	
	'DIFX','in32', noFlags, "Distance Effect","",
	'DITG','in32', noFlags, "Distance Target","",

	'PRST','in32', noFlags, "Preset","",
	'OPRS','in32', noFlags, "Old Preset","",

	'PREP','bool', noFlags, "Enable PreProcessing","",
	'EQAL','bool', noFlags, "Enable Equalizer","",
	'POST','bool', noFlags, "Enable PostProcessing","",
	
	'GRTC','bool', noFlags, "Gradient True Circle","",
	'GRIN','bool', noFlags, "Gradient Invert","",
	'GRMR','bool', noFlags, "Gradient Mirror","",
	
	'ASHI','bool', noFlags, "Invert Mask","",
	'ASHM','s255', noFlags, "Apply Shader Mask","",
	}
};
