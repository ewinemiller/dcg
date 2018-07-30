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
#include "RealUV.h"
#include "ShaderOpsDLL.h"
#define MyEven(xx)	(!(xx & 0x00000001))

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_RealUV(R_CLSID_RealUV);
#else
const MCGUID CLSID_RealUV={R_CLSID_RealUV};
#endif

RealUV::RealUV() {
	SetDefaults();
	}

void RealUV::SetDefaults () {
	fData.iDirection = DIRECTION_XY;
	fData.iSpace = SPACE_LOCAL;
	fData.fSizeU = 48;
	fData.fSizeV = 48;
	fData.iRotation  = 0;
	fData.fOffsetU = 0;
	fData.fOffsetV = 0;  
	fData.fOddLap = 0;
	fData.fEvenLap = 0;  
	}

void* RealUV::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr RealUV::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		if (fData.iSpace == SPACE_GLOBAL) {
			ChildFlags.fNeedsPoint = true;
			}
		if (fData.iSpace == SPACE_LOCAL) {
			ChildFlags.fNeedsPointLoc = true;
			}
		ChildOutput = shader->GetImplementedOutput();
		}
	return MC_S_OK;
	}

boolean	RealUV::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fSizeU==((RealUV*)aShader)->fData.fSizeU)
	  &&
	  (fData.fSizeV==((RealUV*)aShader)->fData.fSizeV)
	  &&
	  (fData.iDirection ==((RealUV*)aShader)->fData.iDirection)
	  &&
	  (fData.fOffsetU ==((RealUV*)aShader)->fData.fOffsetU )
	  &&
	  (fData.fOffsetV ==((RealUV*)aShader)->fData.fOffsetV )
	  &&
	  (fData.fEvenLap ==((RealUV*)aShader)->fData.fEvenLap )
	  &&
	  (fData.fOddLap ==((RealUV*)aShader)->fData.fOddLap )
	  &&
	  (fData.iRotation ==((RealUV*)aShader)->fData.iRotation)
	  &&
	  (fData.iSpace ==((RealUV*)aShader)->fData.iSpace)
	  &&
	  (fData.param==((RealUV*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr RealUV::GetShadingFlags(ShadingFlags& theFlags){
	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput RealUV::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 RealUV::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
real RealUV::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real RealUV::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr RealUV::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		TVector2 fUV = shadingIn.fUV;
		TVector3 thePoint;
		real32 uspace;
		real32 vspace;
		real32 tempspace;
		if (fData.iSpace == SPACE_GLOBAL) {
			thePoint = shadingIn.fPoint;
			} 
		else {//iSpace == SPACE_LOCAL
			thePoint = shadingIn.fPointLoc;
			} 

		if (fData.iDirection == DIRECTION_XY) {
			uspace = thePoint.x;
			vspace = thePoint.y;
			}
		else if (fData.iDirection == DIRECTION_XZ) {
			uspace = thePoint.x;
			vspace = thePoint.z;
			}
		else if (fData.iDirection == DIRECTION_YZ) {
			uspace = thePoint.y;
			vspace = thePoint.z;
			}
		switch (fData.iRotation) {
			case 1:
				tempspace = uspace;
				uspace = -vspace;
				vspace = tempspace;
				break;
			case 2:
				uspace = -uspace;
				vspace = -vspace;
				break;
			case 3:
				tempspace = uspace;
				uspace = vspace;
				vspace = -tempspace;
				break;
			}
		 
		uspace -= fData.fOffsetU;
		vspace -= fData.fOffsetV;

		if MyEven((int32)(floor(vspace/fData.fSizeV))) {
			uspace += fData.fSizeU * fData.fEvenLap;
			}
		else {
			uspace += fData.fSizeU * fData.fOddLap;
			}
		
		shadingIn.fUV.y = (vspace - ((int32)(vspace / fData.fSizeV)) * fData.fSizeV)
			/fData.fSizeV;
		shadingIn.fUV.x = (uspace - ((int32)(uspace / fData.fSizeU)) * fData.fSizeU)
			/fData.fSizeU;

		if (shadingIn.fUV.y < 0) {
			shadingIn.fUV.y = 1 + shadingIn.fUV.y;
			}
		
		if (shadingIn.fUV.x < 0) {
			shadingIn.fUV.x = 1 + shadingIn.fUV.x;
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
		shadingIn.fUV = fUV;
		}

	return MC_S_OK;

	}

int16 RealUV::GetResID(){
	return 304;
	}
