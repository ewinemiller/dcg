/*  Shader Ops - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#include "ProgScale.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_ProgScale(R_CLSID_ProgScale);
#else
const MCGUID CLSID_ProgScale={R_CLSID_ProgScale};
#endif

ProgScale::ProgScale() {
	SetDefaults();
	}
 
void ProgScale::SetDefaults() {
	}

void* ProgScale::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr ProgScale::ExtensionDataChanged(){
	if (!fData.param1) {
		shader1 = NULL;
		}
	else {
		fData.param1->QueryInterface(IID_I3DExShader, (void**)&shader1);
		shader1->GetShadingFlags(ChildFlags1);
		ChildOutput1 = shader1->GetImplementedOutput();
		}
	if (!fData.param2) {
		shader2 = NULL;
		}
	else {
		fData.param2->QueryInterface(IID_I3DExShader, (void**)&shader2);
		shader2->GetShadingFlags(ChildFlags2);
		ChildOutput2 = shader2->GetImplementedOutput();
		}
	return MC_S_OK;
	}

boolean	ProgScale::IsEqualTo(I3DExShader* aShader){

	return (
		(fData.param1==((ProgScale*)aShader)->fData.param1)
		&&
		(fData.param2==((ProgScale*)aShader)->fData.param2)

	  ); 
	}  

MCCOMErr ProgScale::GetShadingFlags(ShadingFlags& theFlags){

	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.CombineFlagsWith(ChildFlags1);
	theFlags.CombineFlagsWith(ChildFlags2);
	return MC_S_OK;
	}

EShaderOutput ProgScale::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 ProgScale::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA getcolorresult;
#else
	TMCColorRGB getcolorresult;
#endif
	int32 retvalue;

	retvalue = GetColor(getcolorresult, fullArea, shadingIn);

	if (retvalue == MC_S_OK)
	{
		result = 0.299 * getcolorresult.R + 0.587 * getcolorresult.G + 0.114 * getcolorresult.B;
	}

	return retvalue;
}

#if (VERSIONNUMBER >= 0x040000)
real ProgScale::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real ProgScale::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr ProgScale::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA tempresult;
#else
	TMCColorRGB tempresult;
#endif

	result.R = 0; result.G = 0; result.B = 0;
	real32 scale = 1.0f;
		TVector3 fPoint = shadingIn.fPoint;
		TVector3 fPointLoc = shadingIn.fPointLoc;

	if (shader2 != NULL) 
	{
		if ((ChildOutput2 & kUsesGetColor) == kUsesGetColor)
		{
			shader2->GetColor(tempresult, fullArea, shadingIn);
			scale = 0.299 * tempresult.R + 0.587 * tempresult.G + 0.114 * tempresult.B;
		}
		else if ((ChildOutput2 & kUsesGetValue) == kUsesGetValue) 
		{
			shader2->GetValue(scale, fullArea, shadingIn);
		}
	}
	if (scale <= 0.001f)
	{
		scale = 0.001f;
	}
	shadingIn.fPointLoc.x/= scale;
	shadingIn.fPointLoc.y/= scale;
	shadingIn.fPointLoc.z/= scale;
	shadingIn.fPoint.x/= scale;
	shadingIn.fPoint.y/= scale;
	shadingIn.fPoint.z/= scale;
	if (shader1 != NULL) {
		if ((ChildOutput1 & kUsesGetColor) == kUsesGetColor) {
			shader1->GetColor(result, fullArea, shadingIn);
			}
		else if ((ChildOutput1 & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader1->GetValue(childresult, fullArea, shadingIn);
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			}
		}

	shadingIn.fPoint = fPoint;
	shadingIn.fPointLoc = fPointLoc;
	return MC_S_OK;

	}
int16 ProgScale::GetResID(){
	return 315;
	}
