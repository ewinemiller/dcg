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
#include "Scale.h"
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Scale(R_CLSID_Scale);
#else
const MCGUID CLSID_Scale={R_CLSID_Scale};
#endif

Scale::Scale() 
{
	SetDefaults();
}

void Scale::SetDefaults () 
{
	fData.iSpace = SPACE_LOCAL;
	fData.fScaleX = 2.0f;
	fData.fScaleY = 2.0f;
	fData.fScaleZ = 2.0f;
}

void* Scale::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr Scale::ExtensionDataChanged(){
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

boolean	Scale::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fScaleX ==((Scale*)aShader)->fData.fScaleX )
	  &&
	  (fData.fScaleY ==((Scale*)aShader)->fData.fScaleY )
	  &&
	  (fData.fScaleZ ==((Scale*)aShader)->fData.fScaleZ )
	  &&
	  (fData.iSpace ==((Scale*)aShader)->fData.iSpace )
	  &&
	  (fData.param==((Scale*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Scale::GetShadingFlags(ShadingFlags& theFlags){
	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Scale::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
Scale::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real Scale::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Scale::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr Scale::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		TVector3 fPoint = shadingIn.fPoint;
		TVector3 fPointLoc = shadingIn.fPointLoc;

		if (fData.iSpace == SPACE_LOCAL)
		{
			shadingIn.fPointLoc.x/= fData.fScaleX;
			shadingIn.fPointLoc.y/= fData.fScaleY;
			shadingIn.fPointLoc.z/= fData.fScaleZ;
		}
		else
		{
			shadingIn.fPoint.x/= fData.fScaleX;
			shadingIn.fPoint.y/= fData.fScaleY;
			shadingIn.fPoint.z/= fData.fScaleZ;
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

int16 Scale::GetResID(){
	return 306;
	}
