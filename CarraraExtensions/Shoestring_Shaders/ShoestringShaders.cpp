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
/* Common Files */

#define  _USE_MATH_DEFINES
//#include "math.h"
#include "vector3.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "IMFDialogPart.h"
#include "IMFPart.h"
#include "MiscComUtilsImpl.h"

#include "Weave.h"
#include "Gray.h"
#include "Super_mixer.h"
#include "Super_mixer_top.h"
#include "Iridescent.h"
#include "Location.h"
#include "Fresnel.h"
#include "Proximity.h"
#include "Translucent.h"
#include "Lit.h"
#include "Drip.h"
#include "MultiPassControl.h"
#include "MultiPassManager.h"
#include "Coordinate_Shift.h"
#include "Key.h"
#include "Spoke.h"


#include "COMSafeUtilities.h"
#include "IShPartUtilities.h"
#include "I3DShUtilities.h"

#if CP_PRAGMA_ONCE
#pragma once
#endif


const MCGUID CLSID_Weave(R_CLSID_Weave);
const MCGUID CLSID_Gray(R_CLSID_Gray);
const MCGUID CLSID_Gray_top(R_CLSID_Gray_top);
const MCGUID CLSID_Super_mixer(R_CLSID_Super_mixer);
const MCGUID CLSID_Super_mixer_top(R_CLSID_Super_mixer_top);
const MCGUID CLSID_Iridescent(R_CLSID_Iridescent);
const MCGUID CLSID_Location(R_CLSID_Location);
const MCGUID CLSID_Fresnel(R_CLSID_Fresnel);
const MCGUID CLSID_Proximity(R_CLSID_Proximity);
const MCGUID CLSID_Translucent(R_CLSID_Translucent);
const MCGUID CLSID_Lit(R_CLSID_Lit);
const MCGUID CLSID_Drip(R_CLSID_Drip);
const MCGUID CLSID_Fresnel2 (R_CLSID_Fresnel2);
const MCGUID CLSID_MultiPassControl (R_CLSID_MultiPassControl);
const MCGUID CLSID_MultiPassManager (R_CLSID_MultiPassManager);
const MCGUID CLSID_Coordinate_Shift (R_CLSID_Coordinate_Shift);
const MCGUID CLSID_Key (R_CLSID_Key);
const MCGUID CLSID_Spoke (R_CLSID_Spoke);


float quarter = 0.5;

void Extension3DInit(IMCUnknown* utilities)
{
}


void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{
	TBasicUnknown* res = NULL;

	if (classId == CLSID_Weave) res = new Weave;
	else if (classId == CLSID_Gray) res = new Gray;
	else if (classId == CLSID_Gray_top) res = new Gray;
	else if (classId == CLSID_Super_mixer) res = new Super_mixer;
	else if (classId == CLSID_Super_mixer_top) res = new Super_mixer_top;
	else if (classId == CLSID_Iridescent) res = new Iridescent;
	else if (classId == CLSID_Location) res = new Location;
	else if (classId == CLSID_Fresnel) res = new Fresnel;
	else if (classId == CLSID_Fresnel2) res = new Fresnel;
	else if (classId == CLSID_Proximity) res = new Proximity;
	else if (classId == CLSID_Translucent) res = new Translucent;
	else if (classId == CLSID_Lit) res = new Lit;
	else if (classId == CLSID_Drip) res = new Drip;

	else if (classId == CLSID_MultiPassControl) res = new MultiPassControl;
	else if (classId == CLSID_MultiPassManager) res = new MultiPassManager;
	else if (classId == CLSID_Coordinate_Shift) res = new Coordinate_Shift;
	else if (classId == CLSID_Key) res = new Key;
	else if (classId == CLSID_Spoke) res = new Spoke;

	return res;
}


