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
#include "Max.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Max(R_CLSID_Max);
#else
const MCGUID CLSID_Max={R_CLSID_Max};
#endif

Max::Max() {
	SetDefaults();
	}
 
void Max::SetDefaults() {
	}

void* Max::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Max::ExtensionDataChanged(){
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

boolean	Max::IsEqualTo(I3DExShader* aShader){

	return (
		(fData.param1==((Max*)aShader)->fData.param1)
		&&
		(fData.param2==((Max*)aShader)->fData.param2)

	  ); 
	}  

MCCOMErr Max::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags1;
	theFlags.CombineFlagsWith(ChildFlags2);
	return MC_S_OK;
	}

EShaderOutput Max::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Max::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Max::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Max::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Max::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA tempresult;
#else
	TMCColorRGB tempresult;
#endif

	result.R = 0; result.G = 0; result.B = 0;

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

	if (shader2 != NULL) {
		if ((ChildOutput2 & kUsesGetColor) == kUsesGetColor) {
			shader2->GetColor(tempresult, fullArea, shadingIn);
			}
		else if ((ChildOutput2 & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader2->GetValue(childresult, fullArea, shadingIn);
			tempresult.R = childresult;
			tempresult.G = childresult;
			tempresult.B = childresult;
			}
		if (tempresult.R > result.R)
		{
			result.R = tempresult.R;	
		}
		if (tempresult.G > result.G)
		{
			result.G = tempresult.G;	
		}
		if (tempresult.B > result.B)
		{
			result.B = tempresult.B;	
		}
	}
	return MC_S_OK;

	}

int16 Max::GetResID(){
	return 315;
	}
