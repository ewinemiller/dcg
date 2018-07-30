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
#include "Layer.h"
#include "ShaderOps2DLL.h"


const MCGUID CLSID_Layer(R_CLSID_Layer);


real Layer::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;

	if (childShader1)
	{
		getChildValue(childShader1, childOutput1, result, shadingIn);
	}
	else
	{
		return 0;
	}
	if (childShader2)
		getChildValue(childShader2, childOutput2, alpha, shadingIn);
	
	return alpha;
}

real Layer::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	
	if (childShader1)
	{
		getChildColor(childShader1, childOutput1, result, shadingIn);
	}
	else
	{
		return 0;
	}
	if (childShader2)
		getChildValue(childShader2, childOutput2, alpha, shadingIn);

	return alpha;
}

real Layer::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	
	if (childShader1)
	{
		getChildVector(childShader1, childOutput1, childFlags1, result, shadingIn);
	}
	else
	{
		return 0;
	}
	if (childShader2)
		getChildValue(childShader2, childOutput2, alpha, shadingIn);

	return alpha;

}