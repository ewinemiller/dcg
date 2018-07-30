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
#ifndef __SingleOpHelper__
#define __SingleOpHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "MCCountedPtr.h"
#include "ParentOpHelper.h" 
#include "DCGUtil.h"
#include <string.h>

struct SingleOpPublicData
{
	//subshaders
	TMCCountedPtr<IShParameterComponent> childShader;	

	boolean  operator==	(const SingleOpPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(SingleOpPublicData)) == 0;
	}
};

template<class T> class SingleOpHelper : public ParentOpHelper<T>{


public:
	using ParentOpHelper<T>::fData;
	using ParentOpHelper<T>::combinedChildFlags;
	using ParentOpHelper<T>::EmptyChildFlags;
	using ParentOpHelper<T>::extensionDataChangeDirty;
	
	SingleOpHelper<T>()  {
	};

	~SingleOpHelper<T>(void){
	};

	real getChildValue(real32& result,boolean& fullArea, ShadingIn& shadingIn)
	{
		return ParentOpHelper<T>::getChildValue(childShader, childOutput, result, shadingIn);
	}

	real getChildColor(TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn)
	{
		return ParentOpHelper<T>::getChildColor(childShader, childOutput, result, shadingIn);
	}

	real getChildVector(TVector3& result, ShadingIn& shadingIn)
	{
		return ParentOpHelper<T>::getChildVector(childShader, childOutput, childFlags, result, shadingIn);
	}

	void getChildShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
	{
		ParentOpHelper<T>::getChildShaderApproxColor(childShader, childOutput, result, shadingIn);
	}
	
	void getChildShaderApproximation(ShadingOut& result,ShadingIn& shadingIn)
	{
		ParentOpHelper<T>::getChildShaderApproximation(childShader, childOutput, result, shadingIn);
	}

	MCCOMErr doChildShade(ShadingOut& result,ShadingIn& shadingIn)
	{
		return ParentOpHelper<T>::doChildShade(childShader, childOutput, result, shadingIn);
	}

protected:
	TMCCountedPtr<I3DShShader> childShader;
	EShaderOutput childOutput;
	ShadingFlags childFlags;

	virtual void getChildShaders()
	{
		if (extensionDataChangeDirty)
		{
			combinedChildFlags = EmptyChildFlags;
			childOutput = kNotImplemented;

			if (!fData.childShader) 
			{
				childShader = NULL;
			}
			else 
			{
				fData.childShader->QueryInterface(IID_I3DShShader, (void**)&childShader);

				childFlags = EmptyChildFlags;
				childShader->GetShadingFlags(childFlags);
				combinedChildFlags.CombineFlagsWith(childFlags);

				childOutput = childShader->GetImplementedOutput();
			}

			extensionDataChangeDirty = false;
		}
	}

	void twiddleShadingInToUse(ShadingIn& shadingInToUse, const int32 x, const int32 y)
	{
		ParentOpHelper<T>::twiddleShadingInToUse(shadingInToUse, childFlags, x, y);
	}
};



#endif