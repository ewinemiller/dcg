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
#include "MultiSubtract.h"
#include "ShaderOps2DLL.h"


const MCGUID CLSID_MultiSubtract(R_CLSID_MultiSubtract);


real MultiSubtract::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
			alpha *= getChildValue(tempResult, shadingIn, childIndex);
			result -= tempResult;
		}
		if (result < 0)
			result = 0;
	}
	return alpha;
}

real MultiSubtract::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	result.Set(1, 1, 1, 1);
	real alpha = 1;

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {

		alpha = getChildColor(result, shadingIn, 0);

		for (uint32 childIndex = 1; childIndex < childCount; childIndex++)
		{
			TMCColorRGBA tempResult;
			alpha *= getChildColor(tempResult, shadingIn, childIndex);
			result.red  -= tempResult.red;
			result.green  -= tempResult.green;
			result.blue  -= tempResult.blue;
			result.alpha  *= tempResult.alpha;
		}
		if (result.red < 0)
			result.red = 0;
		if (result.green < 0)
			result.green = 0;
		if (result.blue < 0)
			result.blue = 0;
	}
	return alpha;
}

real MultiSubtract::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 0;
	result.SetValues(0, 0, 0);

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) {
		for (uint32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			TVector3 tempResult;

			alpha += getChildVector(tempResult, shadingIn, childIndex);
			result += tempResult;
		}

		alpha /= childCount;
		result /= childCount;
	}
	return alpha;

}