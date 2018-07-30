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
#include "MultiMultiply.h"
#include "ShaderOps2DLL.h"


const MCGUID CLSID_MultiMultiply(R_CLSID_MultiMultiply);


real MultiMultiply::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	result = 1.0f;
	real alpha = 1.0f;

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			real tempResult = 0;
			alpha *= getChildValue(tempResult, shadingIn, childIndex);
			result *= tempResult;
		}
	}
	return alpha;
}

real MultiMultiply::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	result.Set(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			TMCColorRGBA tempResult(1.0f, 1.0f, 1.0f, 1.0f);
			alpha *= getChildColor(tempResult, shadingIn, childIndex);
			result.red *= tempResult.red;
			result.green *= tempResult.green;
			result.blue *= tempResult.blue;
			result.alpha *= tempResult.alpha;
		}
	}
	return alpha;
}

real MultiMultiply::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	result.SetValues(0, 0, 0);

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		result.SetValues(1.0f, 1.0f, 1.0f);
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			TVector3 tempResult;
			real tempAlpha = getChildVector(tempResult, shadingIn, childIndex);
			alpha *= tempAlpha;
			if (tempResult != TVector3::kZero)
			{
				result = tempResult;
			}
		}
	}
	return alpha;

}