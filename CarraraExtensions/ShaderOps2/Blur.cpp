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
#include "Blur.h"
#include "ShaderOps2DLL.h"

const MCGUID CLSID_Blur(R_CLSID_Blur);

Blur::Blur() 
{
	fData.blur = 1;
	sampleArea = 1.0f / 9.0f;
}

Blur::~Blur()
{
}


real Blur::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{

	if (extensionDataChangeDirty)
		getChildShaders();

	result = 0;
	ShadingIn shadingInToUse;


	for (int32 x = -fData.blur; x <= fData.blur; x++)
	{
		for (int32 y = -fData.blur; y <= fData.blur; y++)
		{
			real32 tempValue = 0;
			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, x, y);
			getChildValue(tempValue, fullArea, shadingInToUse);
			result += tempValue;

		}
	}
	
	result *= sampleArea;
	return MC_S_OK;
}


real Blur::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();

	result = TMCColorRGBA::kBlackNoAlpha;
	ShadingIn shadingInToUse;

	for (int32 x = -fData.blur; x <= fData.blur; x++)
	{
		for (int32 y = -fData.blur; y <= fData.blur; y++)
		{
			TMCColorRGBA tempValue = TMCColorRGBA::kBlackNoAlpha;
			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, x, y);
			getChildColor(tempValue, fullArea, shadingInToUse);
			result += tempValue;

		}
	}
	
	result *= sampleArea;
	return MC_S_OK;

}


real Blur::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;
	boolean fullArea = false;

	real accumulator[4] = {0, 0, 0, 0};

	for (int32 x = -fData.blur - 1; x <= fData.blur + 1; x++)
	{
		for (int32 y = -fData.blur - 1; y <= fData.blur + 1; y++)
		{
			real32 tempValue = 0;
			shadingInToUse = shadingIn;
			twiddleShadingInToUse (shadingInToUse, x, y);

			getChildValue(tempValue, fullArea, shadingInToUse);
			if (x <= (fData.blur - 1) && y >= -fData.blur && y <= fData.blur)
				accumulator[0] += tempValue;

			if (x >= (-fData.blur + 1) && y >= -fData.blur && y <= fData.blur)
				accumulator[2] += tempValue;

			if (y <= (fData.blur - 1) && x >= -fData.blur && x <= fData.blur)
				accumulator[1] += tempValue;

			if (y >= (-fData.blur + 1) && x >= -fData.blur && x <= fData.blur)
				accumulator[3] += tempValue;
		}
	}
	
	accumulator[0] *= sampleArea;
	accumulator[1] *= sampleArea;
	accumulator[2] *= sampleArea;
	accumulator[3] *= sampleArea;

	TVector2 uv(accumulator[2] - accumulator[0], accumulator[3] - accumulator[1]);

	TVector3 AVec, BVec;

	AVec = -shadingIn.fIsoU;	
	BVec = -shadingIn.fIsoV;	

	AVec = AVec * uv.x;
	BVec = BVec * uv.y;
	result = AVec + BVec;

	return MC_S_OK;
}

void Blur::GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	getChildShaderApproxColor(result, shadingIn);
}

void Blur::GetShaderApproximation(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	getChildShaderApproximation(result, shadingIn);
}

MCCOMErr Blur::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn shadingInToUse = shadingIn;	
	doChildShade(result, shadingIn);

	for (int32 x = -fData.blur; x <= fData.blur; x++)
	{
		for (int32 y = -fData.blur; y <= fData.blur; y++)
		{
			if (y != 0 || x != 0)
			{
				ShadingOut tempValue;

				shadingInToUse = shadingIn;
				twiddleShadingInToUse (shadingInToUse, x, y);
				doChildShade(tempValue, shadingInToUse);
				result += tempValue;
			}
		}
	}
	result *= sampleArea;
	return MC_S_OK;

};
