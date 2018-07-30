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
#include "Brightness.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Brightness(R_CLSID_Brightness);
#else
const MCGUID CLSID_Brightness={R_CLSID_Brightness};
#endif

Brightness::Brightness() {
	SetDefaults();
	}

void Brightness::SetDefaults() {
	fData.fBrightness = 2.0;
	}

void* Brightness::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Brightness::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}

	return MC_S_OK;
	}

boolean	Brightness::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fBrightness ==((Brightness*)aShader)->fData.fBrightness)
	  &&
	  (fData.param==((Brightness*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Brightness::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Brightness::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Brightness::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Brightness::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Brightness::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Brightness::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) 
		{
			shader->GetColor(result, fullArea, shadingIn);
			if (result.A < 1.0f)
			{
				result *= fData.fBrightness;
			}
			else
			{
				result.R *= fData.fBrightness;
				result.G *= fData.fBrightness;
				result.B *= fData.fBrightness;
			}
			
			if (result.R > 1) result.R = 1;
			if (result.G > 1) result.G = 1;
			if (result.B > 1) result.B = 1;
			if (result.A > 1) result.A = 1;
		}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) 
		{
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			childresult *= fData.fBrightness;
			if (childresult > 1) childresult = 1;
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			} 
		}

	return MC_S_OK;

	}

int16 Brightness::GetResID(){
	return 314;
	}
