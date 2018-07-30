/*  Shoestring Shaders - plug-in for Carrara
    Copyright (C) 2003  Mark DesMarais

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

/****************************************************************************************

		Shoestring_shaders.r
		
weave 120
gray 138
super_mixer 128
iri	136
ripples 140
location 134
fresnel 130
fresnel2 170
fresnel menus 128, 150
proximity 124
translucent 144
lit	154
corners 148
curvature 166
drip 160
super_mixer_top 122
shadow_pass  126
multipasscontrol 132
multipassmanager 142
spoke 146
coordinate_shift 152
key 156
gray_top 158
*****************************************************************************************/
 
#include "Copyright.h"
#include "interfaceids.h"

#include "External3DAPI.r"

#include "WeaveDef.h"
#include "GrayDef.h"
#include "FresnelDef.h"
#include "Super_mixerDef.h"
#include "Super_mixer_topDef.h"
#include "IridescentDef.h"
#include "LocationDef.h"
#include "ProximityDef.h"
#include "TranslucentDef.h"
#include "LitDef.h"
#include "DripDef.h"
#include "MultiPassControlDef.h"
#include "MultiPassManagerDef.h"
#include "SpokeDef.h"
#include "Coordinate_ShiftDef.h"
#include "KeyDef.h"


include "Weave.rsr";
include "Gray.rsr";
include "Gray_top.rsr";
include "Super_mixer.rsr";
include "Super_mixer_top.rsr";
include "Iridescent.rsr";
include "Location.rsr";
include "Fresnel.rsr";
include "Fresnel2.rsr";
include "Proximity.rsr";
include "Translucent.rsr";
include "Lit.rsr";
include "Drip.rsr";
include "Spoke.rsr";
include "MultiPassControl.rsr";
include "MultiPassManager.rsr";
include "Coordinate_Shift.rsr";
include "Key.rsr";

#include "Weave.r"
#include "Gray.r"
#include "Super_mixer.r"
#include "Super_mixer_top.r"
#include "Iridescent.r"
#include "Location.r"
#include "Fresnel.r"
#include "Proximity.r"
#include "Translucent.r";
#include "Lit.r";
#include "Drip.r";
#include "Spoke.r";
#include "MultiPassControl.r";
#include "MultiPassManager.r";
#include "Coordinate_Shift.r";
#include "Key.r";

