/*  Ground Control - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#include "DEM.h"
#include "DEMPrim.h"
#include "GCWater.h"
#include "DEMDll.h"
#if (VERSIONNUMBER >= 0x040000)
#include "MCFileType.h"
#include "GCShapefile.h"
#include "GCGenerator.h"
#include "ShaderFilter.h"
#endif
#include "GCBump.h"
#if (VERSIONNUMBER >= 0x050000)
#include "comsafeutilities.h"
#endif
#include "IShUtilities.h"


void Extension3DInit(IMCUnknown* utilities)
{
    TMCFileTypeList typesList;
    gFileFormatUtilities->LoadResourceFTYP(kRID_ImportFilterFamilyID, 'DDEM', 305, 0, typesList);
    gFileFormatUtilities->AddFileTypes(typesList);

}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}


TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	// This method instanciate
{														// the object COM
	TBasicUnknown* res = NULL;
	
	if (classId == CLSID_GCGenerator) 
	{
		res = new GCGenerator; 
	}
	if (classId == CLSID_ShaderFilter) 
	{
		res = new ShaderFilter; 
	}
	if (classId == CLSID_GCShapefile)
	{
		res = new GCShapefile;
	}

	if (classId == CLSID_DEM 
		|| classId == CLSID_r32
		|| classId == CLSID_TER
		|| classId == CLSID_HGT
		|| classId == CLSID_BIL
		|| classId == CLSID_PGM
		|| classId == CLSID_IMG
		|| classId == CLSID_ASC
		|| classId == CLSID_FLT
		|| classId == CLSID_PDS) 
	{
		res = new DEM; 
	}

	if (classId == CLSID_DEMPrim) res = new DEMPrim;
	if (classId == CLSID_GCWater) res = new GCWater;
	if (classId == CLSID_GCBump) res = new GCBump;

	return res;
}


void updateProgressBarCallback(void* data, float farAlong) {
	updateProgressBar* pb = static_cast<updateProgressBar*> (data);
	gShellUtilities->SetProgressValue((pb->progressStart + pb->progressRun * farAlong) * 100.0f, pb->progressKey);
}