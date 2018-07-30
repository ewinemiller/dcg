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
#include "Stretch.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Stretch(R_CLSID_Stretch);
#else
const MCGUID CLSID_Stretch={R_CLSID_Stretch};
#endif

Stretch::Stretch() {
	SetDefaults();
	}

void Stretch::SetDefaults(){
	fData.vec2Stretch.x = .2f;
	fData.vec2Stretch.y = .8f;
	}

void* Stretch::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Stretch::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}

	fStretchFactor = 1 / (fData.vec2Stretch.y - fData.vec2Stretch.x);
	return MC_S_OK;
	}

boolean	Stretch::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.vec2Stretch==((Stretch*)aShader)->fData.vec2Stretch)
	  &&
	  (fData.param==((Stretch*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Stretch::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Stretch::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
Stretch::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Stretch::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Stretch::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Stretch::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(result, fullArea, shadingIn);
			result.R = (result.R - fData.vec2Stretch.x)
				* fStretchFactor;
			result.G = (result.G - fData.vec2Stretch.x)
				* fStretchFactor;
			result.B = (result.B - fData.vec2Stretch.x)
				* fStretchFactor;
			if (result.R > 1) {
				result.R = 1;
				}
			else if(result.R < 0) {
				result.R = 0;
				}
			if (result.G > 1) {
				result.G = 1;
				}
			else if(result.G < 0) {
				result.G = 0;
				}
			if (result.B > 1) {
				result.B = 1;
				}
			else if(result.B < 0) {
				result.B = 0;
				}
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			childresult = (childresult - fData.vec2Stretch.x)
				* fStretchFactor;
			if (childresult > 1) {
				childresult = 1;
				}
			else if(childresult < 0) {
				childresult = 0;
				}
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			}
		}

	return MC_S_OK;

	}

int16 Stretch::GetResID(){
	return 310;
	}
