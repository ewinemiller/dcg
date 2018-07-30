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
#ifndef __ControllingOpHelper__
#define __ControllingOpHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "MCCountedPtr.h"
#include "ParentOpHelper.h" 
#include "DCGUtil.h"
#include <string.h>

struct ControllingOpPublicData
{
	TMCCountedPtr<IShParameterComponent> controllingShader;	

	//subshaders
	TMCCountedPtrArray<IShParameterComponent> childShaders;	

	boolean  operator==	(const ControllingOpPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(ControllingOpPublicData)) == 0;
	}
};


template<class T> class ControllingOpHelper : public ParentOpHelper<T>{


public:
	using ParentOpHelper<T>::fData;
	using ParentOpHelper<T>::combinedChildFlags;
	using ParentOpHelper<T>::EmptyChildFlags;
	using ParentOpHelper<T>::extensionDataChangeDirty;

	ControllingOpHelper<T>()  {
	};

	~ControllingOpHelper<T>(void){
	};

protected:
	TMCCountedPtr<I3DShShader> controllingShader;
	EShaderOutput controllingOutput;

	TMCCountedPtrArray<I3DShShader> childShaders;
	TMCArray<EShaderOutput> childOutput;
	TMCArray<ShadingFlags> childFlags;

	real getChildValue(real32& result, ShadingIn& shadingIn
		, unsigned int childIndex)
	{
		return ParentOpHelper<T>::getChildValue(childShaders[childIndex], childOutput[childIndex], result, shadingIn);
	}

	real32 getControllingValue(ShadingIn& shadingIn)
	{
		real32 result = 0;
		
		if (controllingShader) 
		{
			ParentOpHelper<T>::getChildValue(controllingShader, controllingOutput, result, shadingIn);
		}
		if (result < 0)
			result = 0;
		if (result > 1.0f)
			result = 1.0f;

		return result;
	}

	real getChildColor(TMCColorRGBA& result, ShadingIn& shadingIn
		, unsigned int childIndex)
	{
		return ParentOpHelper<T>::getChildColor(childShaders[childIndex], childOutput[childIndex], result, shadingIn);
	}
	
	real getChildVector(TVector3& result, ShadingIn& shadingIn
		, unsigned int childIndex)
	{
		return ParentOpHelper<T>::getChildVector(childShaders[childIndex], childOutput[childIndex],  childFlags[childIndex], result, shadingIn);

	}

	void getChildShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn
		, unsigned int childIndex)
	{
		ParentOpHelper<T>::getChildShaderApproxColor(childShaders[childIndex], childOutput[childIndex], result, shadingIn);
	}

	void doChildShade(ShadingOut& result,ShadingIn& shadingIn
		, unsigned int childIndex)
	{
		ParentOpHelper<T>::doChildShade(childShaders[childIndex], childOutput[childIndex], result, shadingIn);
	}

	virtual void getChildShaders()
	{
		if (extensionDataChangeDirty)
		{
			combinedChildFlags = EmptyChildFlags;

			uint32 childCount = fData.childShaders.GetElemCount();

			childShaders.SetElemCount(childCount);
			childOutput.SetElemCount(childCount);
			childFlags.SetElemCount(childCount);

			for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
			{
				childFlags[childIndex] = EmptyChildFlags;
				childOutput[childIndex] = kNotImplemented;

				if (fData.childShaders[childIndex] == NULL) 
				{
					childShaders.SetElem(childIndex, NULL);
				}
				else 
				{
					TMCCountedPtr<I3DShShader> tempshader;
					fData.childShaders[childIndex]->QueryInterface(IID_I3DShShader, (void**)&tempshader);
					childShaders.SetElem(childIndex, tempshader);

					childShaders[childIndex]->GetShadingFlags(childFlags[childIndex]);
					combinedChildFlags.CombineFlagsWith(childFlags[childIndex]);

					childOutput[childIndex] = childShaders[childIndex]->GetImplementedOutput();
				}

			}

			if (!fData.controllingShader)
			{
				controllingShader = NULL;
			}
			else
			{
				ShadingFlags tempFlags;

				fData.controllingShader->QueryInterface(IID_I3DShShader, (void**)&controllingShader);

				controllingShader->GetShadingFlags(tempFlags);
				combinedChildFlags.CombineFlagsWith(tempFlags);

				controllingOutput = controllingShader->GetImplementedOutput();
			}

			extensionDataChangeDirty = false;
		}
	}


};



#endif