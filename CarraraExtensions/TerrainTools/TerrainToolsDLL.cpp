/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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

#include "Beach.h"
#include "Surf.h"
#include "Intersect.h"
#include "Cutout.h"
#include "NormalToTerrain.h"
#include "TerrainToNormal.h"

#include "IShSMP.h"
#include "COMUtilities.h"
#include "../AnythingGrows/AnythingGrowsDeformer.h"
//#include "TTWave.h"

uint32 lResetValue = 0;



#if VERSIONNUMBER >= 0x050000
const MCGUID IID_AnythingGrowsDeformer(R_IID_AnythingGrowsDeformer);
#else
const MCGUID IID_AnythingGrowsDeformer={R_IID_AnythingGrowsDeformer};
#endif

void InvertTransform(const TTransform3D& in, TTransform3D& out)
{
	in.GetInverse(out);
	out.fTranslation = in.fTranslation;
}


void Extension3DInit(IMCUnknown* utilities)
{

	surfCache.init();
	beachCache.init();
	intersectCache.init();
	cutoutCache.init();
}

void Extension3DCleanup()
{
	surfCache.cleanup();
	beachCache.cleanup();
	intersectCache.cleanup();
	cutoutCache.cleanup();
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{
	TBasicUnknown* res = NULL;
	if (classId == CLSID_Beach)
	{
		res = new Beach;
	}
	if (classId == CLSID_BeachTerrain)
	{
		res = new Beach;
	}
	if (classId == CLSID_Surf)
	{
		res = new Surf;
	}
	if (classId == CLSID_Cutout)
	{
		res = new Cutout;
	}
	if (classId == CLSID_Intersect)
	{
		res = new Intersect;
	}
	if (classId == CLSID_NormalToTerrain)
	{
		res = new NormalToTerrain;
	}
	if (classId == CLSID_TerrainToNormal)
	{
		res = new TerrainToNormal;
	}
	return res;
}

