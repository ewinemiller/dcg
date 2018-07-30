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
#include "Threshold.h" 
#include "ShaderOpsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Threshold(R_CLSID_Threshold);
#else
const MCGUID CLSID_Threshold={R_CLSID_Threshold};
#endif

Threshold::Threshold() {
	SetDefaults(); 
	}

void Threshold::SetDefaults() {
	fData.fThreshold = 0.5;
	}

void* Threshold::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Threshold::ExtensionDataChanged(){
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

boolean	Threshold::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fThreshold ==((Threshold*)aShader)->fData.fThreshold)
	  &&
	  (fData.param==((Threshold*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr Threshold::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput Threshold::GetImplementedOutput(){
	return kUsesGetValue;
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Threshold::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn){
	if (shader != NULL) {
		shader->GetValue(result, fullArea, shadingIn);
			if (result > fData.fThreshold) {
				result = 1;
				}
			else {
				result = 0;
				}
		}
	return MC_S_OK;
	}

int16 Threshold::GetResID(){
	return 312;
	}
