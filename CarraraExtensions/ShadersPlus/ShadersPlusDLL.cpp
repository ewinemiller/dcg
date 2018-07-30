/*  Shader Plus - plug-in for Carrara
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
#include "IShSMP.h"
#include "COMUtilities.h"

#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif
#include "Anisotropic.h"
#include "FauxAnisotropic.h"
#include "Flat.h"
#include "ShadowGlow.h"
#include "Mask.h"
#include "SelectiveLighting.h"
#include "GIShadowCatcher.h"
#include "GIControl.h"
#include "ProceduralLock.h"
#include "Bevel.h"
#if VERSIONNUMBER >= 0x060000
#include "SpecularDiffuseControl.h"
#endif


void Extension3DInit(IMCUnknown* utilities)
{
	plCache.init();
	bevelCache.init();
}

void Extension3DCleanup()
{
	plCache.cleanup();
	bevelCache.cleanup();
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_Anisotropic) res = new Anisotropic;
	if (classId == CLSID_FauxAnisotropic) res = new FauxAnisotropic;
	if (classId == CLSID_Flat) res = new Flat;
	if (classId == CLSID_ShadowGlow) res = new ShadowGlow;
	if (classId == CLSID_Mask) res = new Mask;
	if (classId == CLSID_SelectiveLighting) res = new SelectiveLighting;
	if (classId == CLSID_GIControl) res = new GIControl;
	if (classId == CLSID_GIShadowCatcher) res = new GIShadowCatcher;
	if (classId == CLSID_ProceduralLock) res = new ProceduralLock;
	if (classId == CLSID_Bevel) res = new Bevel;
#if VERSIONNUMBER >= 0x060000
	if (classId == CLSID_SpecularDiffuseControl) res = new SpecularDiffuseControl;
#endif
	return res;
}
