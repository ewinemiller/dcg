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
#ifndef __BinaryOpHelper__
#define __BinaryOpHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "MCCountedPtr.h"
#include "ParentOpHelper.h" 
#include "DCGUtil.h"
#include <string.h>

struct BinaryOpPublicData
{
	//subshaders
	TMCCountedPtr<IShParameterComponent> childShader1;	
	TMCCountedPtr<IShParameterComponent> childShader2;	

	boolean  operator==	(const BinaryOpPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(BinaryOpPublicData)) == 0;
	}
};

template<class T> class BinaryOpHelper : public ParentOpHelper<T>{


public:
	using ParentOpHelper<T>::fData;
	using ParentOpHelper<T>::combinedChildFlags;
	using ParentOpHelper<T>::EmptyChildFlags;
	using ParentOpHelper<T>::extensionDataChangeDirty;
	
	BinaryOpHelper<T>()  {
	};

	~BinaryOpHelper<T>(void){
	};

protected:
	TMCCountedPtr<I3DShShader> childShader1;
	TMCCountedPtr<I3DShShader> childShader2;
	EShaderOutput childOutput1, childOutput2;
	ShadingFlags childFlags1, childFlags2;

	virtual void getChildShaders()
	{
		if (extensionDataChangeDirty)
		{
			combinedChildFlags = EmptyChildFlags;
			childFlags1 = EmptyChildFlags;
			childFlags2 = EmptyChildFlags;
			childOutput1 = kNotImplemented;
			childOutput2 = kNotImplemented;

			if (!fData.childShader1) 
			{
				childShader1 = NULL;
			}
			else 
			{
				fData.childShader1->QueryInterface(IID_I3DShShader, (void**)&childShader1);

				childShader1->GetShadingFlags(childFlags1);
				combinedChildFlags.CombineFlagsWith(childFlags1);

				childOutput1 = childShader1->GetImplementedOutput();
			}

			if (!fData.childShader2) 
			{
				childShader2 = NULL;
			}
			else 
			{
				fData.childShader2->QueryInterface(IID_I3DShShader, (void**)&childShader2);

				childShader2->GetShadingFlags(childFlags2);
				combinedChildFlags.CombineFlagsWith(childFlags2);

				childOutput2 = childShader2->GetImplementedOutput();
			}
			extensionDataChangeDirty = false;
		}
	}

};



#endif