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
#ifndef __GCBump__
#define __GCBump__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "GCBumpDef.h"
#include "BasicShader.h"
#include "DCGShaderHelper.h"
#include "DemPrim.h"
#include <string.h>

enum FalloffStyle {foAbsolute = 1, foLinear = 2, foRadial = 3, foInvRadial = 4, foCustom = 5};

struct GCBumpShaderPublicData { 
	boolean  operator==	(const GCBumpShaderPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(GCBumpShaderPublicData)) == 0;
	}
};

extern const MCGUID CLSID_GCBump;
extern const MCGUID IID_GCBump;

class GCBump : public TBasicShader, public DCGShaderHelper
{
public :

	GCBump();
	~GCBump();
	STANDARD_RELEASE;
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
		const        { return sizeof(GCBumpShaderPublicData); }
	virtual void*			MCCOMAPI    GetExtensionDataBuffer();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
	virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
	virtual real			MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);

	virtual boolean			MCCOMAPI	WantsTransform						() { return false; };
	virtual real			MCCOMAPI	GetVector(TVector3& result,ShadingIn& shadingIn);
protected:
	GCBumpShaderPublicData fData;
	MCCOMErr GetDem(ShadingIn &shadingIn, DEMPrim** GCBump);

};
                           
#endif 

