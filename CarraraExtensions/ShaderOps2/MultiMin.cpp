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
#include "MultiMin.h"
#include "ShaderOps2DLL.h"


const MCGUID CLSID_MultiMin(R_CLSID_MultiMin);


real MultiMin::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	result = 1;
	real alpha = 1;

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			real tempResult = 0;
			alpha = min(alpha, getChildValue(tempResult, shadingIn, childIndex));
			result = min(result, tempResult);
		}
	}
	return alpha;
}

real MultiMin::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1;
	result.Set(1, 1, 1, 1);

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			TMCColorRGBA tempResult;
			alpha = min(alpha, getChildColor(tempResult, shadingIn, childIndex));
			result.red = min(result.red, tempResult.red);
			result.green = min(result.green, tempResult.green);
			result.blue = min(result.blue, tempResult.blue);
			result.alpha = min(result.alpha, tempResult.alpha);
		}
	}
	return alpha;
}

real MultiMin::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f, maxValue = 100000000.0f;
	result.SetValues(0, 0, 0);

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			real tempValue = 0;
			getChildValue(tempValue, shadingIn, childIndex);
			if (tempValue < maxValue)
			{
				maxValue = tempValue;
				alpha = getChildVector(result, shadingIn, childIndex);
			}
		}
	}
	return alpha;

}