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
#include "GIControl.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "I3dShTreeElement.h"
#include "I3DShInstance.h"
#include "ISceneDocument.h"
#include "I3DRenderingModule.h"
#include "I3dShCamera.h"
#include "I3DShLightsource.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GIControl(R_CLSID_GIControl);
#else
const MCGUID CLSID_GIControl={R_CLSID_GIControl};
#endif

GIControl::GIControl() 
{
	fData.fEffect = 1.0f;
}

void* GIControl::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr GIControl::ExtensionDataChanged()
{
	if (!fData.param) 
	{
		shader = NULL;
		
	}
	else 
	{
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		
	}
	return MC_S_OK;
}

boolean	GIControl::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<GIControl*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<GIControl*>(aShader))->fData.fEffect == fData.fEffect)
		);
}  

MCCOMErr GIControl::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput GIControl::GetImplementedOutput()
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
MCCOMErr GIControl::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr GIControl::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

boolean GIControl::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}
void GIControl::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
	if (shader != NULL)
	{
		//call our child's Calculate so that we can be stacked with other lighting models
		result.fShadingOut->fGlow = fGlow;
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
}

MCCOMErr GIControl::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
		fGlow = result.fGlow;
		result.fGlow *= fData.fEffect;
	}

	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x070000)
void GIControl::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void GIControl::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
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
}