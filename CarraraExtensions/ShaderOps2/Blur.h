/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#ifndef __Blur__
#define __Blur__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BlurDef.h"
#include "IShComponent.h"
#include "copyright.h"
#include "I3DShShader.h"
#include "MCCountedPtrArray.h"
#include "SingleOpHelper.h"

#include <string.h>

struct BlurPublicData
{
	int32 blur;
	TMCCountedPtr<IShParameterComponent> childShader;	
	boolean  operator==	(const BlurPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(BlurPublicData)) == 0;
	}
};

extern const MCGUID CLSID_Blur;

class Blur : public SingleOpHelper<BlurPublicData>
{
	public:
		Blur();
		~Blur();
		STANDARD_RELEASE;

		virtual real MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetVector(TVector3& result,ShadingIn& shadingIn);
		virtual MCCOMErr MCCOMAPI DoShade(ShadingOut& result,ShadingIn& shadingIn);
		virtual void MCCOMAPI GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn);
		virtual void MCCOMAPI GetShaderApproximation(ShadingOut& result,ShadingIn& shadingIn);

		virtual MCCOMErr MCCOMAPI ExtensionDataChanged() {
			SingleOpHelper<BlurPublicData>::ExtensionDataChanged();
			sampleArea = 1.0f / static_cast<real32>((fData.blur * 2 + 1) * (fData.blur * 2 + 1));
			return MC_S_OK;
		};

	protected:
		real32 sampleArea;
};

#endif