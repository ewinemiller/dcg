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
#include "RadialGradient.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_RadialGradient(R_CLSID_RadialGradient);
#else
const MCGUID CLSID_RadialGradient={R_CLSID_RadialGradient};
#endif

RadialGradient::RadialGradient() {
	fData.fPosU = .5;
	fData.fPosV = .5;
	fData.vec2Range.x = 0;
	fData.vec2Range.y = .5;
	}

void* RadialGradient::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr RadialGradient::ExtensionDataChanged(){
	return MC_S_OK;
	}

boolean	RadialGradient::IsEqualTo(I3DExShader* aShader){
	return (
		(fData.fPosU ==((RadialGradient*)aShader)->fData.fPosU)
		&&(fData.fPosV ==((RadialGradient*)aShader)->fData.fPosV)
		&&(fData.vec2Range ==((RadialGradient*)aShader)->fData.vec2Range)
		);
	}  

MCCOMErr RadialGradient::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
	}

EShaderOutput RadialGradient::GetImplementedOutput(){
	return kUsesGetValue;
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 RadialGradient::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn){

	real32 fDistance = sqrt((shadingIn.fUV.x  - fData.fPosU )*(shadingIn.fUV.x - fData.fPosU) 
							 + (shadingIn.fUV.y - fData.fPosV )*(shadingIn.fUV.y  - fData.fPosV ));
	fullArea = false;
	if (fDistance <= fData.vec2Range.x ) {
		result = 0;
		return MC_S_OK;
		}
	else if (fDistance >= fData.vec2Range.y) {
		result = 1;
		return MC_S_OK;
		}
	else {
		result = (fDistance - fData.vec2Range.x) / (fData.vec2Range.y - fData.vec2Range.x);
		return MC_S_OK;
		}


	}

int16 RadialGradient::GetResID(){
	return 322;
	}
