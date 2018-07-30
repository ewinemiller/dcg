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
#include "Translate.h"
#include "ShaderOpsDLL.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Translate(R_CLSID_Translate);
#else
const MCGUID CLSID_Translate={R_CLSID_Translate};
#endif

Translate::Translate() 
{
	SetDefaults();
}

void Translate::SetDefaults () 
{
	fData.iSpace = SPACE_LOCAL;
	fData.fOffsetX = 10;
	fData.fOffsetY = 10;
	fData.fOffsetZ = 10;
}

void* Translate::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr Translate::ExtensionDataChanged(){
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

boolean	Translate::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fOffsetX==((Translate*)aShader)->fData.fOffsetX)
	  &&
	  (fData.fOffsetY==((Translate*)aShader)->fData.fOffsetY)
	  &&
	  (fData.fOffsetZ ==((Translate*)aShader)->fData.fOffsetZ)
	  &&
	  (fData.iSpace ==((Translate*)aShader)->fData.iSpace)
	  &&
	  (fData.param==((Translate*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Translate::GetShadingFlags(ShadingFlags& theFlags){
	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Translate::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Translate::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Translate::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Translate::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Translate::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		TVector3 fPoint = shadingIn.fPoint;
		TVector3 fPointLoc = shadingIn.fPointLoc;

		if (fData.iSpace == SPACE_LOCAL)
		{
			shadingIn.fPointLoc.x+= fData.fOffsetX;
			shadingIn.fPointLoc.y+= fData.fOffsetY;
			shadingIn.fPointLoc.z+= fData.fOffsetZ;
		}
		else
		{
			shadingIn.fPoint.x+= fData.fOffsetX;
			shadingIn.fPoint.y+= fData.fOffsetY;
			shadingIn.fPoint.z+= fData.fOffsetZ;
		}


		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(result, fullArea, shadingIn);
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			} 
		shadingIn.fPoint = fPoint;
		shadingIn.fPointLoc = fPointLoc;
		}

	return MC_S_OK;

	}

int16 Translate::GetResID(){
	return 305;
	}
