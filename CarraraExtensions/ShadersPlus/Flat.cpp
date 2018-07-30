/*  Shader Plus - plug-in for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#include "Flat.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Flat(R_CLSID_Flat);
#else
const MCGUID CLSID_Flat={R_CLSID_Flat};
#endif

Flat::Flat() 
{
}

void* Flat::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr Flat::ExtensionDataChanged()
{
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

boolean	Flat::IsEqualTo(I3DExShader* aShader)
{
	return ((static_cast<Flat*>(aShader))->fData.param == fData.param);
}  

MCCOMErr Flat::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput Flat::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting + kUseCalculateIndirectLighting
#endif
	);
}

boolean Flat::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}
void Flat::CalculateDirectLighting(LightingDetail& result, const LightingContext& lightingContext)
{
	const ShadingOut& shading = *result.fShadingOut;

#if VERSIONNUMBER >= 0x070000
	result.fLightShadow.Set(1, 1, 1);
#endif
#if  VERSIONNUMBER >= 0x050000
	result.fAmbient.Set(0,0,0);
	result.fDiffuseLight = shading.fColor;
//#elif VERSIONNUMBER >= 0x050000
//	result.fAmbient = shading.fColor;
#else
	result.fAmbientLight = shading.fColor;
#endif

}


#if (VERSIONNUMBER >= 0x070000)
void Flat::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
{
//	ambientOcclusionFactor = 0;
}
#else
void Flat::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{

}
#endif


MCCOMErr Flat::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}

	return MC_S_OK;
}


