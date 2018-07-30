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
#include "Clip.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Clip(R_CLSID_Clip);
#else
const MCGUID CLSID_Clip={R_CLSID_Clip};
#endif

Clip::Clip() {
	SetDefaults();
	}

void Clip::SetDefaults() {
	fData.vec2Clip.x = .2f;
	fData.vec2Clip.y = .8f;
	}

void* Clip::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Clip::ExtensionDataChanged(){
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

boolean	Clip::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.vec2Clip==((Clip*)aShader)->fData.vec2Clip)
	  &&
	  (fData.param==((Clip*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Clip::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Clip::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Clip::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Clip::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Clip::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Clip::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(result, fullArea, shadingIn);
			if (result.R > fData.vec2Clip.y) {
				result.R = fData.vec2Clip.y;
				}
			else if(result.R < fData.vec2Clip.x) {
				result.R = fData.vec2Clip.x;
				}
			if (result.G > fData.vec2Clip.y) {
				result.G = fData.vec2Clip.y;
				}
			else if(result.G < fData.vec2Clip.x) {
				result.G = fData.vec2Clip.x;
				}
			if (result.B > fData.vec2Clip.y) {
				result.B = fData.vec2Clip.y;
				}
			else if(result.B < fData.vec2Clip.x) {
				result.B = fData.vec2Clip.x;
				}
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			if (childresult > fData.vec2Clip.y) {
				childresult = fData.vec2Clip.y;
				}
			else if(childresult < fData.vec2Clip.x) {
				childresult = fData.vec2Clip.x;
				}
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			}
		}

	return MC_S_OK;

	}

int16 Clip::GetResID(){
	return 308;
	}
