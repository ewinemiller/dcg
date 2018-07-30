/*  Toon! Pro - plug-in for Carrara
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
#include "Cell.h"
#include "ToonEnabled.h"
//#include "ProfileShader.h"

#include "IShSMP.h"
#include "COMUtilities.h"
#include "WireRenderer.h"
#if VERSIONNUMBER >= 0x050000 
#include "SetCellLightingModel.h"
#endif

#if VERSIONNUMBER >= 0x050000
#include "comsafeutilities.h"
#endif


void Extension3DInit(IMCUnknown* utilities)
{

}

void Extension3DCleanup()
{

}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_Cel) 
		res = new CelShader;

#if VERSIONNUMBER >= 0x050000 
	if (classId == CLSID_SetCellLightingModel) 
		res = new SetCellLightingModel;
#endif

	if (classId == CLSID_ToonEnabled) res = new ToonEnabled(rsToon);
	if (classId == CLSID_ToonRenderer) 
		res = new WireRenderer(rsToon);
	return res;
}
