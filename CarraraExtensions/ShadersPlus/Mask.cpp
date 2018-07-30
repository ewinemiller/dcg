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
#include "Mask.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "I3dShTreeElement.h"
#include "I3DShInstance.h"
#include "ISceneDocument.h"
#include "I3DRenderingModule.h"
#include "I3dShCamera.h"
#include "I3dShScene.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Mask(R_CLSID_Mask);
#else
const MCGUID CLSID_Mask={R_CLSID_Mask};
#endif

Mask::Mask() 
{
	fData.lEffect = mePassThru;
	fData.customColor.Set(0,0,1,0);
#if VERSIONNUMBER >= 0x050000
	defaultlightingmodel = NULL;
#endif

}

void* Mask::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr Mask::ExtensionDataChanged()
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

boolean	Mask::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<Mask*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<Mask*>(aShader))->fData.lEffect == fData.lEffect)
		&&
		((static_cast<Mask*>(aShader))->fData.customColor == fData.customColor)

		);
}  


MCCOMErr Mask::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	theFlags.fNeedsPoint  = true;
	return MC_S_OK;
}

EShaderOutput Mask::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
	+ kUseCalculateIndirectLighting

#endif
	);
}

#if VERSIONNUMBER >= 0x050000
#if VERSIONNUMBER >= 0x070000
MCCOMErr Mask::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else VERSIONNUMBER >= 0x050000
MCCOMErr Mask::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

boolean Mask::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}
void Mask::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x050000
	TMCColorRGB& ambient = result.fAmbient;
#else
	TMCColorRGB& ambient = result.fAmbientLight;
#endif
	

	switch (fData.lEffect)
	{
	case mePassThru:
		if (shader != NULL)
		{
#if (VERSIONNUMBER >= 0x050000)
			if ((ChildOutput & kUseCalculateDirectLighting) == kUseCalculateDirectLighting)
			{
				(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateDirectLighting(result, lightingContext);
			}
			else
			{
				defaultlightingmodel->CalculateDirectLighting(result, lightingContext);
			}
#else
			(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateDirectLighting(result, lightingContext);
#endif
		}
		break;
	case meBlack:
		ambient = TMCColorRGB::kBlack;
		break;
	case meWhite:
		ambient = TMCColorRGB::kWhite;
		break;
	case meCustom:
		ambient = fData.customColor;
		break;

	}
}

MCCOMErr Mask::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{

	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}

	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x070000)
void Mask::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void Mask::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{

	if (shader != NULL)
	{
#if (VERSIONNUMBER >= 0x050000)
		if ((ChildOutput & kUseCalculateIndirectLighting) == kUseCalculateIndirectLighting)
		{
#if (VERSIONNUMBER >= 0x070000)
			(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
			(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
		else
		{
#if (VERSIONNUMBER >= 0x070000)
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
#else
		(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
	}
	indirectDiffuseColor.Set(0, 0, 0);
#if (VERSIONNUMBER >= 0x070000)
	ambientOcclusionFactor = 0;
#endif
	
}
