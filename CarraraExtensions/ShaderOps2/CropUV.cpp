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
#include "CropUV.h"
#include "ShaderOps2DLL.h"

const MCGUID CLSID_CropUV(R_CLSID_CropUV);

CropUV::CropUV() 
{
	fData.UCrop.SetValues(0, 1.0f);
	fData.VCrop.SetValues(0, 1.0f);
}

CropUV::~CropUV()
{
}


void CropUV::cropUV(const ShadingIn &shadingIn, ShadingIn &shadingInToUse)
{
	shadingInToUse.fUV.x = fData.UCrop.x + shadingIn.fUV.x * (fData.UCrop.y - fData.UCrop.x);
	shadingInToUse.fUV.y = fData.VCrop.x + shadingIn.fUV.y * (fData.VCrop.y - fData.VCrop.x);
}

real CropUV::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{

	if (extensionDataChangeDirty)
		getChildShaders();

	ShadingIn shadingInToUse = shadingIn;
	cropUV(shadingIn, shadingInToUse);

	return getChildValue(result, fullArea, shadingInToUse);
}


real CropUV::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();

	ShadingIn shadingInToUse = shadingIn;	
	cropUV(shadingIn, shadingInToUse);

	return getChildColor(result, fullArea, shadingInToUse);
}


real CropUV::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;
	cropUV(shadingIn, shadingInToUse);

	return getChildVector(result, shadingInToUse);
}

void CropUV::GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;
	cropUV(shadingIn, shadingInToUse);

	getChildShaderApproxColor(result, shadingInToUse);
}

void CropUV::GetShaderApproximation(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;
	cropUV(shadingIn, shadingInToUse);

	getChildShaderApproximation(result, shadingInToUse);
}

MCCOMErr CropUV::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;	
	cropUV(shadingIn, shadingInToUse);

	return doChildShade(result, shadingInToUse);
};
