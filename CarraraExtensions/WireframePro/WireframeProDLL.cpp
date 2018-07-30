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
#include "copyright.h"
#include "Wireframe.h"
//#include "WireframeAccelerator.h"

#include "IShSMP.h"
#include "COMUtilities.h"
#include "WireRenderer.h"
#include "ToonEnabled.h"


#if VERSIONNUMBER >= 0x050000
#include "comsafeutilities.h"
#endif

uint32 lResetValue = 0;
TMCCriticalSection* gBackgroundCS = NULL;



void Extension3DInit(IMCUnknown* utilities)
{

	// Perform your dll initialization here
	wireframeCache.init();
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
	wireframeCache.cleanup();
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	
{														
	TBasicUnknown* res = NULL;
	if (classId == CLSID_WireRenderer)
	{
		res = new WireRenderer(rsWireFrame);
	}
	if (classId == CLSID_Wireframe)
	{
		res = new Wireframe;
	}
	if (classId == CLSID_WireframeEnabled)
	{
		res = new ToonEnabled(rsWireFrame);
	}

	return res;
}

