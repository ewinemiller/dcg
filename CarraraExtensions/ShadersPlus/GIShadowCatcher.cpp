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
#include "GIShadowCatcher.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "ShadersPlusdll.h"
#include "I3DShLightsource.h"
#include "I3dShTreeElement.h"


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GIShadowCatcher(R_CLSID_GIShadowCatcher);
#else
const MCGUID CLSID_GIShadowCatcher={R_CLSID_GIShadowCatcher};
#endif

GIShadowCatcher::GIShadowCatcher() 
{
	fData.fBrightness  = 1.0f;
	fData.lSubShaderUse = ssIgnore;
}

void* GIShadowCatcher::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr GIShadowCatcher::ExtensionDataChanged()
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

boolean	GIShadowCatcher::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<GIShadowCatcher*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<GIShadowCatcher*>(aShader))->fData.fBrightness == fData.fBrightness)
		&&
		((static_cast<GIShadowCatcher*>(aShader))->fData.lSubShaderUse == fData.lSubShaderUse)

		);
}  

MCCOMErr GIShadowCatcher::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput GIShadowCatcher::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x060000
	| kUsesGetShaderApproxColor
#endif
#if VERSIONNUMBER >= 0x050000
	| kUseCalculateDirectLighting
	| kUseCalculateIndirectLighting
#endif
	);
}

boolean GIShadowCatcher::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}
#if VERSIONNUMBER >= 0x050000

#if VERSIONNUMBER >= 0x070000
void GIShadowCatcher::InternalCalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor,const LightingContext& lightingContext,const ShadingOut& shading)
{
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
void GIShadowCatcher::InternalCalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext& lightingContext,const ShadingOut& shading)
{
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
const LightingFlags lightingFlags = lightingContext.fLightingFlags;
FirstHitInfo* const firstHitInfo = lightingContext.fFirstHitInfo;

if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting && (shading.fColor.R>0.02f || shading.fColor.G>0.02f || shading.fColor.B>0.02f))
{
 TMCColorRGB indirectLightColor(0.0f,0.0f,0.0f);

 boolean needFullCalculation=true;

 if (firstHitInfo && firstHitInfo->fIndirectLightingInfo.fInitialised)
 {
  if (firstHitInfo->fIndirectLightingInfo.fNormal * shading.fChangedNormal > 0.9f)
  {
   // we do not need to calculate the indirect lighting as it has already been interpolated
   indirectLightColor=firstHitInfo->fIndirectLightingInfo.fColor;
   needFullCalculation=false;
  }
 }
 if (needFullCalculation)
 {
#if VERSIONNUMBER >= 0x070000
    lightingContext.fRaytracer->GetIndirectLighting(indirectLightColor, ambientOcclusionFactor, lightingContext);
#else
    lightingContext.fRaytracer->GetIndirectLighting(indirectLightColor,lightingContext);
#endif

  indirectLightColor *= illumSettings.fIndirectLightingIntensity;

  if (firstHitInfo)
  {
   IndirectLightingInfo& info = firstHitInfo->fIndirectLightingInfo;

   info.fInitialised=true;
   info.fColor=indirectLightColor;
   info.fNormal=shading.fChangedNormal;
  }
 }
 MCAssert(indirectLightColor.R>=0);

 indirectDiffuseColor = ((*(TMCColorRGB*)&shading.fColor) % indirectLightColor) * shading.fLambert;
}
} 

#if VERSIONNUMBER >= 0x070000
MCCOMErr GIShadowCatcher::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr GIShadowCatcher::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

void GIShadowCatcher::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
	const LightingFlags lightingFlags = lightingContext.fLightingFlags;

	if (lightingFlags.fComputeDiffuseDirectLighting)
	{
#if VERSIONNUMBER >= 0x050000
			TMCColorRGB& resultAmbient = result.fAmbient;
#else
			TMCColorRGB& resultAmbient = result.fAmbientLight;
#endif		
			real32 intensity;

		ShadingIn shin = static_cast<ShadingIn>(*lightingContext.fHit);
		ShadingOut& shading = *result.fShadingOut;

		if (fData.lSubShaderUse != ssIgnore && shader != NULL)
		{
			LocalToGlobalVector(lightingContext.fHit->fT, changedNormal, shading.fChangedNormal);
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
			shading.fChangedNormal = shin.fGNormal;
		}
		//   first get the ambient light

		I3DExRaytracer* raytracer = lightingContext.fRaytracer;

		const TMCColorRGBA& ambientLight = lightingContext.GetAmbientLight();

		//  compute the lighting of each light

		const int32 lightCount = raytracer->GetLightCount();


		DirectLighting directLighting;
		const TMCColorRGB& lighting  = directLighting.fLightColor;
		const TVector3& lightDirection = directLighting.fLightDirection;
		TMCColorRGB lighttally = ambientLight;

		//if we have indirect lighting then get those values to add to our lighting level
		if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting)
		{
			TMCColorRGBA tempshading = shading.fColor;
			shading.fColor.Set(1.0f, 1.0f, 1.0f, 1.0f);
#if VERSIONNUMBER >= 0x070000
			InternalCalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
			lighttally += indirectDiffuseColor;
#elif VERSIONNUMBER >= 0x050000
			InternalCalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
			lighttally += indirectDiffuseColor;
#else
			TBasicShader::CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
			lighttally += indirectDiffuseColor;
#endif
			shading.fColor = tempshading;
		}

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
		{
			TMCColorRGB lightresult(0,0,0);
			real shadowIntensity;
			real distance;
			TVector3 resultDirection;
			TMCCountedPtr<I3DShLightsource> light;
			scene->GetLightsourceByIndex(&light, lightIndex);

			light->GetDirection(shin.fPoint, resultDirection, distance);
			light->GetColor(shin.fPoint, resultDirection, distance, lightresult, shadowIntensity );
			real angl= (shin.fGNormal * resultDirection);
			if (angl >= 0)
			{
#if VERSIONNUMBER >= 0x050000
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,illumSettings.fShadowsOn, false))
#else
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,lightingContext.fIllumSettings.fShadowsOn))
#endif
				{
					lightresult = lighting;
				}
			}
		
			lighttally += lightresult;

		}//end for lights
		TMCColorRGBA color;
		boolean	fullAreaDone;
		lightingContext.fRaytracer-> GetBackGroundColor(color, fullAreaDone,
												*lightingContext.fIncomingRay,
												lightingContext.fScreenCoordinates,
												true,
												true,
												false);

		intensity = lighttally.Intensity() / fData.fBrightness;

		if (intensity >= 1.0f)
		{
			result.fDiffuseLight = color;
			resultAmbient = TMCColorRGB::kBlack;
			result.fSpecularLight = TMCColorRGB::kBlack;
			
			shading.fColor.alpha = 0;
		}
		else
		{
			switch (fData.lSubShaderUse)
			{
			case ssIgnore:
#if VERSIONNUMBER >= 0x070000
				if (illumSettings.fComputeSeparatedShadow)
				{
					result.fDiffuseLight = color;
					result.fLightShadow.Set(intensity, intensity, intensity);
				}
				else
				{
#endif
					result.fDiffuseLight = color * intensity;
#if VERSIONNUMBER >= 0x070000
				}
#endif
				resultAmbient = TMCColorRGB::kBlack;
				result.fSpecularLight = TMCColorRGB::kBlack;
				break;
			case ssTint:
				shading.fColor.red = shading.fColor.red * (1.0f - intensity) + intensity;
				shading.fColor.green = shading.fColor.green * (1.0f - intensity) + intensity;
				shading.fColor.blue = shading.fColor.blue * (1.0f - intensity) + intensity;

#if VERSIONNUMBER >= 0x070000
				if (illumSettings.fComputeSeparatedShadow)
				{
					result.fDiffuseLight = color % shading.fColor;
					result.fLightShadow.Set(intensity, intensity, intensity);
				}
				else
				{
#endif
					result.fDiffuseLight = color % shading.fColor * intensity;
#if VERSIONNUMBER >= 0x070000
				}
#endif
				resultAmbient = TMCColorRGB::kBlack;
				result.fSpecularLight = TMCColorRGB::kBlack;
				break;
			case ssReplace:
#if VERSIONNUMBER >= 0x070000
				if (illumSettings.fComputeSeparatedShadow)
				{
					result.fLightShadow.Set(1.0f, 1.0f, 1.0f);
				}
#endif
				result.fDiffuseLight.Set(color.red * intensity + result.fDiffuseLight.red * (1.0f - intensity)
					, color.green * intensity + result.fDiffuseLight.green * (1.0f - intensity)
					, color.blue * intensity + result.fDiffuseLight.blue * (1.0f - intensity));


#if VERSIONNUMBER >= 0x050000
				resultAmbient = result.fAmbient * (1.0f - intensity);
#endif
				result.fSpecularLight = result.fSpecularLight * (1.0f - intensity);
				break;
			}
			shading.fColor.alpha = 1.0f - intensity;
		}//end in shadow

	}
}

MCCOMErr GIShadowCatcher::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	TMCCountedPtr<I3DShTreeElement> tree;
	if (MCVerify(shadingIn.fInstance)) 
	{
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK) 
		{
			// Find the scene
			tree->GetScene(&scene);
		} 
	} 
	if (fData.lSubShaderUse != ssIgnore && shader != NULL)
	{
		shader->DoShade(result, shadingIn);
		if (result.fChangedNormalLoc != TVector3::kZero)
		{
			changedNormal = result.fChangedNormalLoc;
			result.fChangedNormal = TVector3::kZero;
			result.fChangedNormalLoc = shadingIn.fNormalLoc;
		}
		
	}
	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x070000)
void GIShadowCatcher::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void GIShadowCatcher::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{
#if VERSIONNUMBER >= 0x070000
	ambientOcclusionFactor = 0;
#endif
}
