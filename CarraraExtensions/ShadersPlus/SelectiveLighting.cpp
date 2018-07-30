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
#include "SelectiveLighting.h"
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
const MCGUID CLSID_SelectiveLighting(R_CLSID_SelectiveLighting);
#else
const MCGUID CLSID_SelectiveLighting={R_CLSID_SelectiveLighting};
#endif
SelectiveLighting::SelectiveLighting() 
{
	fData.lEffect = sePassThru;
	fData.bAllowGI = true;
	fData.bAllowAmbient = true;
}

void* SelectiveLighting::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr SelectiveLighting::ExtensionDataChanged()
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

boolean	SelectiveLighting::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<SelectiveLighting*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<SelectiveLighting*>(aShader))->fData.lEffect == fData.lEffect)
		&&
		((static_cast<SelectiveLighting*>(aShader))->fData.sList == fData.sList)
		&&
		((static_cast<SelectiveLighting*>(aShader))->fData.bAllowAmbient == fData.bAllowAmbient)
		&&
		((static_cast<SelectiveLighting*>(aShader))->fData.bAllowGI == fData.bAllowGI)
		);
}  

MCCOMErr SelectiveLighting::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	theFlags.fNeedsPoint  = true;
	return MC_S_OK;
}

EShaderOutput SelectiveLighting::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
	+ kUseCalculateIndirectLighting
#endif
	);
}

boolean SelectiveLighting::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

#if VERSIONNUMBER >= 0x050000
#if VERSIONNUMBER >= 0x070000
MCCOMErr SelectiveLighting::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr SelectiveLighting::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

void SelectiveLighting::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif	
	const LightingFlags lightingFlags = lightingContext.fLightingFlags;

	if (lightingFlags.fComputeDiffuseDirectLighting)
	{
		DirectLighting directLighting;
		const TMCColorRGB& lighting  = directLighting.fLightColor;
		const TVector3& lightDirection = directLighting.fLightDirection;
#if VERSIONNUMBER >= 0x070000
		const TMCColorRGB&	noShadowlighting = directLighting.fNoShadowLightColor;	// the no shadow ligtht color will be used when we need to compute separated shadow
		TMCColorRGB			diffShadow = TMCColorRGB::kBlack;
		TMCColorRGB			specShadow = TMCColorRGB::kBlack;
#endif

		const ShadingOut& shading = *result.fShadingOut;

		//   first get the ambient light

		I3DExRaytracer* raytracer = lightingContext.fRaytracer;

		TMCColorRGBA ambientLight = lightingContext.GetAmbientLight();

		if (fData.bAllowAmbient == false && fData.lEffect != sePassThru)
		{
			ambientLight.Set(0, 0, 0, 0);
		}

		// this attenuates the ambient light to look more like a distant, in case no other lights
		real norme= (shading.fChangedNormal[2]+shading.fChangedNormal[0]) * 0.5f;

		if (norme<0.0f) norme=-norme;
		norme= 0.5f*(norme-0.5f)+shading.fAmbient;

		if (norme<0.0f) norme=0.0f;
		if (norme>1.0f) norme=1.0f;

		// compute the base color
#if VERSIONNUMBER >= 0x050000
		result.fAmbient.R = shading.fColor.R * ambientLight.R * norme + shading.fGlow.R;
		result.fAmbient.G = shading.fColor.G * ambientLight.G * norme + shading.fGlow.G;
		result.fAmbient.B = shading.fColor.B * ambientLight.B * norme + shading.fGlow.B;
#else
		result.fAmbientLight.R = shading.fColor.R * ambientLight.R * norme + shading.fGlow.R;
		result.fAmbientLight.G = shading.fColor.G * ambientLight.G * norme + shading.fGlow.G;
		result.fAmbientLight.B = shading.fColor.B * ambientLight.B * norme + shading.fGlow.B;
#endif
		//  compute the lighting of each light

		const int32 lightCount = raytracer->GetLightCount();

		result.fDiffuseLight = TMCColorRGB::kBlack;
		result.fSpecularLight = TMCColorRGB::kBlack;

		TMCDynamicString localList(fData.sList);
		localList.Append("\r", 1);

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
		{
			boolean bIncludeLight = (fData.lEffect == seExclude);
			TMCCountedPtr<I3DShLightsource> light;
			scene->GetLightsourceByIndex(&light, lightIndex);
			if (MCVerify(light))
			{
				TMCCountedPtr<I3DShTreeElement> lightTree;
				lightTree = light->GetTreeElement();
				if (MCVerify(lightTree))
				{
					TMCString255 name;
					lightTree->GetName(name);
					TMCDynamicString localName(name);
					localName.Append("\r",1);
					uint32 index = localList.SearchStr(localName);
					//is the light in the list?
					if (index != kUINT32_MAX)
					{
						bIncludeLight = !bIncludeLight;
					}
					//is the light member of a group in the list
					else
					{
						TMCCountedPtr<I3DShTreeElement> nextitem;
						lightTree->GetTop(&lightTree);

						while (lightTree != NULL) 
						{
							lightTree->GetName(name);
							TMCDynamicString localName(name);
							localName.Append("\r",1);
							uint32 index = localList.SearchStr(localName);
							//is the light in the list?
							if (index != kUINT32_MAX)
							{
								bIncludeLight = !bIncludeLight;
								lightTree = NULL;
							}
							else
							{
								lightTree -> GetTop(&lightTree);
							}
						}
					}
				}
			}
			if ((bIncludeLight) || (fData.lEffect == sePassThru))
			{
#if VERSIONNUMBER >= 0x050000
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,illumSettings.fShadowsOn, false))
#else
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,lightingContext.fIllumSettings.fShadowsOn))
#endif
				{
					result.fIsInShadow |= directLighting.fIsInShadow;
					const real coef= (shading.fChangedNormal * lightDirection) * shading.fLambert;

					if (coef>0)
					{
#if VERSIONNUMBER >= 0x070000
						if (illumSettings.fComputeSeparatedShadow)
						{
							result.fDiffuseLight.R += coef * noShadowlighting.R;
							result.fDiffuseLight.G += coef * noShadowlighting.G;
							result.fDiffuseLight.B += coef * noShadowlighting.B;

							diffShadow.R += coef * lighting.R;
							diffShadow.G += coef * lighting.G;
							diffShadow.B += coef * lighting.B;
						}
						else
						{
#endif
							result.fDiffuseLight.R += coef * lighting.R;
							result.fDiffuseLight.G += coef * lighting.G;
							result.fDiffuseLight.B += coef * lighting.B;
#if VERSIONNUMBER >= 0x070000
						}
#endif
						const real S_R = lightingContext.fReflectDir * lightDirection;

						if (S_R > 0.0f)
						{
							const real yy=(0.001f+shading.fSpecularSize*shading.fSpecularSize)* kSpecularPowerCoef;

							const real specCoef= 2 * RealPow(S_R,yy);

#if VERSIONNUMBER >= 0x070000
							if (illumSettings.fComputeSeparatedShadow)
							{
								result.fSpecularLight.R += specCoef * noShadowlighting.R;
								result.fSpecularLight.G += specCoef * noShadowlighting.G;
								result.fSpecularLight.B += specCoef * noShadowlighting.B;

								specShadow.R += specCoef * lighting.R;
								specShadow.G += specCoef * lighting.G;
								specShadow.B += specCoef * lighting.B;
							}
							else
							{
#endif
								result.fSpecularLight.R += lighting.R * specCoef;
								result.fSpecularLight.G += lighting.G * specCoef;
								result.fSpecularLight.B += lighting.B * specCoef;
#if VERSIONNUMBER >= 0x070000
							}
#endif
						}
					}
				}
			}//end include light
		}//end for lights

#if VERSIONNUMBER >= 0x070000
		if (illumSettings.fComputeSeparatedShadow)
		{
			// we can now compute the fLightShadow using the information from the lighting with and without shadow.

			TMCColorRGB shadowFactorDiffuse;
			for (int32 i=0; i<3; ++i)
			{
				if (result.fDiffuseLight[i] > 0) shadowFactorDiffuse[i] = diffShadow[i] / result.fDiffuseLight[i];
				else shadowFactorDiffuse[i] = (result.fIsInShadow ? 0 : 1);
			}

			// the result diffuse and specular light coorespond to the lighting without shadowing
			result.fDiffuseLight = result.fDiffuseLight % shading.fColor;
			result.fSpecularLight = result.fSpecularLight % shading.fSpecularColor;

			// this resColor is the diffuse and specular lighting with shadow. we use it to compute
			// fLightShadow
			TMCColorRGB resColor = (diffShadow % shading.fColor) + (specShadow % shading.fSpecularColor);

			const TMCColorRGB noShadowDiffuseSpecular = result.fDiffuseLight + result.fSpecularLight;
			for (int32 i=0; i<3; ++i)
			{
				if (noShadowDiffuseSpecular[i] > 1e-3f)
					result.fLightShadow[i] = resColor[i] / noShadowDiffuseSpecular[i];
				else
					result.fLightShadow[i] = shadowFactorDiffuse[i];
			}
		}
		else
		{
#endif
			// only the shading color is missing to have the correct result

			result.fDiffuseLight = result.fDiffuseLight % shading.fColor;
			result.fSpecularLight = result.fSpecularLight % shading.fSpecularColor;
#if VERSIONNUMBER >= 0x070000
			result.fLightShadow.Set(1, 1, 1);
		}
#endif
	}//end do direct
}

MCCOMErr SelectiveLighting::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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

	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}

	return MC_S_OK;
}
#if (VERSIONNUMBER >= 0x070000)
void SelectiveLighting::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void SelectiveLighting::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{
	if (fData.bAllowGI || fData.lEffect == sePassThru)
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
	else
	{
#if (VERSIONNUMBER >= 0x070000)
		if (lightingContext.fIllumSettings->fGlobalIlluminationType == eIndirectLight)
		{
			ambientOcclusionFactor = 0;
		}
		else
		{
			ambientOcclusionFactor = 1;
		}
#endif
		indirectDiffuseColor.Set(0,0,0);
	}
}
