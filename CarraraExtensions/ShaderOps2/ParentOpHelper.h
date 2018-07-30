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
#ifndef __ParentOpHelper__
#define __ParentOpHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "MCCountedPtr.h"
#include "BasicShader.h" 
#include "DCGUtil.h"
#include <string.h>


template<class T> class ParentOpHelper : public TBasicShader{


public:
	ParentOpHelper<T>()  {
		extensionDataChangeDirty = true;
	};

	~ParentOpHelper<T>(void){
	};

	STANDARD_RELEASE;

	virtual int32 MCCOMAPI GetParamsBufferSize() const { 
		return sizeof(T);
	};

	virtual void* MCCOMAPI GetExtensionDataBuffer() {
		return ((void*) &(fData));
	};

	virtual boolean	MCCOMAPI IsEqualTo(I3DExShader* aShader){
		return (fData == static_cast<ParentOpHelper<T>*>(aShader)->fData);
	};

	virtual MCCOMErr MCCOMAPI ExtensionDataChanged() {
		extensionDataChangeDirty = true;
		return MC_S_OK;
	};

	virtual boolean	MCCOMAPI WantsTransform() {
		return false;
	};

	virtual EShaderOutput MCCOMAPI GetImplementedOutput() {

		return (EShaderOutput)(kUsesGetValue + kUsesGetColor + kUsesGetVector + kUsesGetShaderApproxColor + kUsesDoShade + kUsesGetShaderApprox); 
	};

	virtual MCCOMErr MCCOMAPI GetShadingFlags(ShadingFlags& theFlags) {
		if (extensionDataChangeDirty)
			getChildShaders();

		theFlags.CombineFlagsWith(combinedChildFlags);
		return MC_S_OK;
	};

protected:
	ShadingFlags combinedChildFlags, EmptyChildFlags;
	T fData;
	boolean extensionDataChangeDirty;

	virtual void getChildShaders() = 0;

	void twiddleShadingInToUse(ShadingIn& shadingInToUse, const ShadingFlags& childFlags, const int32 x, const int32 y)
	{
		if (childFlags.fNeedsUV)
		{
			shadingInToUse.fUV += x * shadingInToUse.fUVx + y * shadingInToUse.fUVy;
		}
		if (childFlags.fNeedsPoint)
		{
			shadingInToUse.fPoint += x * shadingInToUse.fPointx + y * shadingInToUse.fPointy;
		}
		if (childFlags.fNeedsPointLoc)
		{
			shadingInToUse.fPointLoc += x * shadingInToUse.fPointLocx + y * shadingInToUse.fPointLocy;
		}
		if (childFlags.fNeedsNormal)
		{
			shadingInToUse.fGNormal += x * shadingInToUse.fNormalx + y * shadingInToUse.fNormaly;
		}
		if (childFlags.fNeedsNormalLoc)
		{
			shadingInToUse.fNormalLoc += x * shadingInToUse.fNormalLocx + y * shadingInToUse.fNormalLocy; 
		}
	}

	real getChildValue(I3DShShader* childShader, const EShaderOutput childOutput, real32& result, ShadingIn& shadingIn)
	{
		real retval = 0;
		boolean fullArea;

		if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			retval = childShader->GetValue(result, fullArea, shadingIn);
		}
		else if ((childOutput & kUsesGetColor) == kUsesGetColor) 
		{
			TMCColorRGBA getcolorresult;
			retval = childShader->GetColor(getcolorresult, fullArea, shadingIn);
			result = getcolorresult.Intensity();
		}
		else if ((childOutput & kUsesDoShade) == kUsesDoShade)
		{
			ShadingOut childResult;
			childShader->DoShade(childResult, shadingIn);
			result = childResult.fColor.Intensity();
		}
		else if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			TMCColorRGBA getcolorresult;
			childShader->GetShaderApproxColor(getcolorresult, shadingIn);
			result = getcolorresult.Intensity();
		}
		return retval;
	}

	real getChildColor(I3DShShader* childShader, const EShaderOutput childOutput, TMCColorRGBA& result, ShadingIn& shadingIn)
	{
		real retval = 0;
		boolean fullArea;

		if ((childOutput & kUsesGetColor) == kUsesGetColor) 
		{
			retval = childShader->GetColor(result, fullArea, shadingIn);

		}
		else if ((childOutput & kUsesDoShade) == kUsesDoShade)
		{
			ShadingOut childResult;
			childShader->DoShade(childResult, shadingIn);
			result = childResult.fColor;
		}
		else if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			real childresult;
			retval = childShader->GetValue(childresult, fullArea, shadingIn);
			result.Set(childresult, childresult, childresult, 1.0f);
		}
		else if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			childShader->GetShaderApproxColor(result, shadingIn);

		}
		return retval;
	}

	void getChildShaderApproxColor(I3DShShader* childShader, const EShaderOutput childOutput, TMCColorRGBA& result,ShadingIn& shadingIn)
	{
		if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			childShader->GetShaderApproxColor(result, shadingIn);

		}
		else if ((childOutput & kUsesGetShaderApprox) == kUsesGetShaderApprox)
		{
			ShadingOut tempResult;
			childShader->GetShaderApproximation(tempResult, shadingIn);
			result = tempResult.fColor;
		} 
		else if ((childOutput & kUsesGetColor) == kUsesGetColor) 
		{
			boolean fullArea;
			childShader->GetColor(result, fullArea, shadingIn);
		}
		else if ((childOutput & kUsesDoShade) == kUsesDoShade)
		{
			ShadingOut childResult;
			childShader->DoShade(childResult, shadingIn);
			result = childResult.fColor;
		}
		else if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			boolean fullArea;
			real childresult;
			childShader->GetValue(childresult, fullArea, shadingIn);
			result.Set(childresult, childresult, childresult, 1.0f);
		}
	}

	MCCOMErr doChildShade(I3DShShader* childShader, const EShaderOutput childOutput, ShadingOut& result,ShadingIn& shadingIn)
	{

		if ((childOutput & kUsesDoShade) == kUsesDoShade)
		{
			childShader->DoShade(result, shadingIn);
		} 
		else if ((childOutput & kUsesGetColor) == kUsesGetColor) 
		{
			boolean fullArea;
			childShader->GetColor(result.fColor, fullArea, shadingIn);
		}
		else if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			boolean fullArea;
			real childresult;
			childShader->GetValue(childresult, fullArea, shadingIn);
			result.fColor.Set(childresult, childresult, childresult, 1.0f);
		}
		else if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			childShader->GetShaderApproxColor(result.fColor, shadingIn);

		}
		return MC_S_OK;
	}

	void getChildShaderApproximation(I3DShShader* childShader, const EShaderOutput childOutput, ShadingOut& result,ShadingIn& shadingIn)
	{

		if ((childOutput & kUsesGetShaderApprox) == kUsesGetShaderApprox)
		{
			childShader->GetShaderApproximation(result, shadingIn);
		} 
		else if ((childOutput & kUsesDoShade) == kUsesDoShade)
		{
			childShader->DoShade(result, shadingIn);
		} 
		else if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			TMCColorRGBA tempResult;
			childShader->GetShaderApproxColor(tempResult, shadingIn);
			result.fColor = tempResult;
		}
		else if ((childOutput & kUsesGetColor) == kUsesGetColor) 
		{
			boolean fullArea;
			childShader->GetColor(result.fColor, fullArea, shadingIn);
		}
		else if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			boolean fullArea;
			real childresult;
			childShader->GetValue(childresult, fullArea, shadingIn);
			result.fColor.Set(childresult, childresult, childresult, 1.0f);
		}
		else if ((childOutput & kUsesGetShaderApproxColor) == kUsesGetShaderApproxColor) 
		{
			childShader->GetShaderApproxColor(result.fColor, shadingIn);

		}
	}	
	
	real getChildVector(I3DShShader* childShader, const EShaderOutput childOutput, const ShadingFlags& childFlags
		, TVector3& result, ShadingIn& shadingIn)
	{

		real retval = 1.0f;
		if ((childOutput & kUsesGetVector) == kUsesGetVector) 
		{
			retval = childShader->GetVector(result, shadingIn);
		}
		else 
		{
			ShadingIn shadingInToUse = shadingIn;

			real libresult[4];

			TVector3 AVec, BVec;
			
			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, childFlags, -1, 0);

			getChildValue(childShader, childOutput, libresult[0], shadingInToUse);

			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, childFlags, 0, -1);

			getChildValue(childShader, childOutput, libresult[1], shadingInToUse);

			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, childFlags, 1, 0);

			getChildValue(childShader, childOutput, libresult[2], shadingInToUse);

			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, childFlags, 0, 1);

			getChildValue(childShader, childOutput, libresult[3], shadingInToUse);
			
			TVector2 uv(libresult[2] - libresult[0], libresult[3] - libresult[1]);

			AVec = -shadingIn.fIsoU;	
			BVec = -shadingIn.fIsoV;	

			AVec = AVec * uv.x;
			BVec = BVec * uv.y;
			result = AVec + BVec;

		}

		return retval;
	}


};



#endif