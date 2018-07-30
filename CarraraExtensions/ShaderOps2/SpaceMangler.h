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
#ifndef __SpaceMangler__
#define __SpaceMangler__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif     

#include "SpaceManglerDef.h"
#include "IShComponent.h"
#include "copyright.h"
#include "I3DShShader.h"
#include "MCCountedPtrArray.h"
#include "SingleOpHelper.h"

extern const MCGUID CLSID_SpaceMangler;

enum MangleSelections {mangleGlobalX = 1, mangleGlobalY, mangleGlobalZ, mangleLocalX, mangleLocalY, mangleLocalZ, mangleU, mangleV
	, mangleAngleUX, mangleAngleUY, mangleAngleVX, mangleAngleVY, mangleConstant, mangleGlobalXToOrigin, mangleGlobalYToOrigin, mangleGlobalZToOrigin};

struct SpaceManglerPublicData
{
	//subshaders
	ActionNumber globalX, globalY, globalZ, localX, localY, localZ, u, v;
	real32 globalScaleX, globalScaleY, globalScaleZ, localScaleX, localScaleY, localScaleZ, scaleU, scaleV;
	TMCCountedPtr<IShParameterComponent> childShader;	

	boolean  operator==	(const SpaceManglerPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(SpaceManglerPublicData)) == 0;
	}
};

 
class SpaceMangler : public SingleOpHelper<SpaceManglerPublicData>
{
	public:
		SpaceMangler();
		~SpaceMangler() {};
		STANDARD_RELEASE;

		virtual MCCOMErr MCCOMAPI GetShadingFlags(ShadingFlags& theFlags);
		virtual real MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetVector(TVector3& result,ShadingIn& shadingIn);
		virtual MCCOMErr MCCOMAPI DoShade(ShadingOut& result,ShadingIn& shadingIn);
		virtual void MCCOMAPI GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn);
		virtual int16 MCCOMAPI GetResID( ) {return 105;};

	protected:
		virtual const ShadingIn getMappedShadingIn(const ShadingIn& shadingIn);
		virtual const real32 getMappedValue(const ShadingIn& shadingIn, const MangleSelections mangleSelection);
		virtual const ShadingFlags getMappedShadingFlags(const ShadingFlags& theFlags);
		virtual const boolean isSelected(const MangleSelections mangleSelection);
	};

#endif