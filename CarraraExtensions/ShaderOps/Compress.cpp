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
#include "Compress.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Compress(R_CLSID_Compress);
#else
const MCGUID CLSID_Compress={R_CLSID_Compress};
#endif

Compress::Compress() {
	SetDefaults();
	}

void Compress::SetDefaults() {
	fData.vec2Compress.x = .2f;
	fData.vec2Compress.y = .8f;
	}

void* Compress::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Compress::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}
	fRange = fData.vec2Compress.y - fData.vec2Compress.x;
	return MC_S_OK;
	}

boolean	Compress::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.vec2Compress==((Compress*)aShader)->fData.vec2Compress)
	  &&
	  (fData.param==((Compress*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Compress::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Compress::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Compress::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Compress::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Compress::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Compress::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(result, fullArea, shadingIn);
			result.R = fData.vec2Compress.x + result.R * fRange;
			result.G = fData.vec2Compress.x + result.G * fRange;
			result.B = fData.vec2Compress.x + result.B * fRange;
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			childresult = fData.vec2Compress.x + childresult * fRange;
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			}
		}

	return MC_S_OK;

	}

int16 Compress::GetResID(){
	return 309;
	}
