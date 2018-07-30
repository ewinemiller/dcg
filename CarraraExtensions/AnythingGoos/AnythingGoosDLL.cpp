/*  Anything Goos - plug-in for Carrara
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
#include "AnythingGoos.h"

#include "IShSMP.h"
#include "COMUtilities.h"

#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif

void Extension3DInit(IMCUnknown* utilities)
{

	agoosCache.init();

}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
	agoosCache.cleanup();
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	
{														
	TBasicUnknown* res = NULL;
	if (classId == CLSID_AnythingGoos)
	{
		res = new AnythingGoos();
	}
	if (classId == CLSID_AnythingGoosTerrain)
	{
		res = new AnythingGoos();
	}
	return res;
}

