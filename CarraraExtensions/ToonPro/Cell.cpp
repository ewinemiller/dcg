/*  Toon! Pro - plug-in for Carrara
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
#include "Cell.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "MCCountedPtrHelper.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Cel(R_CLSID_Cel);
const MCGUID IID_Cel(R_IID_Cel);
#else
const MCGUID CLSID_Cel={R_CLSID_Cel};
const MCGUID IID_Cel={R_IID_Cel};
#endif

CelShader::CelShader() 
{
	fData.vec2ShadowHighlight.x = .50f;
	fData.vec2ShadowHighlight.y = .75f;
	fData.fShadowBrightness = .30f;
	fData.lLevels = 1;
	defaultlightingmodel = NULL;
}

void* CelShader::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr CelShader::ExtensionDataChanged()
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

boolean	CelShader::IsEqualTo(I3DExShader* aShader)
{
	return ((static_cast<CelShader*>(aShader))->fData.param == fData.param);
}  

MCCOMErr CelShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput CelShader::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesDoShade
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting + kUseCalculateIndirectLighting
#endif		
		);
}

boolean CelShader::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

void CelShader::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
	const LightingFlags lightingFlags = lightingContext.fLightingFlags;

	if (lightingFlags.fComputeDiffuseDirectLighting)
	{
		real32 coefTally = 0, specCoefTally = 0;

		const ShadingOut& shading = *result.fShadingOut;

		//   first get the ambient light

		I3DExRaytracer* raytracer = lightingContext.fRaytracer;

		const TMCColorRGBA& ambientLight = lightingContext.GetAmbientLight();

		// this attenuates the ambient light to look more like a distant, in case no other lights
		real norme= (shading.fChangedNormal[2]+shading.fChangedNormal[0]) * 0.5f;

		if (norme<0.0f) norme=-norme;
			norme= 0.5f*(norme-0.5f)+shading.fAmbient;

		if (norme<0.0f) norme=0.0f;
		if (norme>1.0f) norme=1.0f;

		// compute the base color
		coefTally = ambientLight.Intensity();

		//if we have indirect lighting then get those values to add to our lighting level
		if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting && (shading.fColor.R>0.02f || shading.fColor.G>0.02f || shading.fColor.B>0.02f))
		{
#if VERSIONNUMBER >= 0x070000
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#elif (VERSIONNUMBER >= 0x050000)
			TMCColorRGB indirectDiffuseColor;
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#else
			TMCColorRGB indirectDiffuseColor;
			TBasicShader::CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
			coefTally += indirectDiffuseColor.Intensity();
		}

		//  compute the lighting of each light

		const int32 lightCount = raytracer->GetLightCount();

		result.fDiffuseLight = TMCColorRGB::kBlack;
		result.fSpecularLight = TMCColorRGB::kBlack;

		DirectLighting directLighting;
		const TMCColorRGB& lighting  = directLighting.fLightColor;
		const TVector3& lightDirection = directLighting.fLightDirection;

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
		{
#if VERSIONNUMBER >= 0x070000
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,lightingContext.fIllumSettings->fShadowsOn, false))
#elif VERSIONNUMBER >= 0x050000
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,lightingContext.fIllumSettings.fShadowsOn, false))
#else
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,lightingContext.fIllumSettings.fShadowsOn))
#endif
			{
				result.fIsInShadow |= directLighting.fIsInShadow;

				const real coef= (shading.fChangedNormal * lightDirection) * shading.fLambert;

				if (coef>0)
				{
					coefTally += coef * lighting.Intensity();

					const real S_R = lightingContext.fReflectDir * lightDirection;

					if (S_R > 0.0f)
					{
						const real yy=(0.001f+shading.fSpecularSize*shading.fSpecularSize)* kSpecularPowerCoef;

						const real specCoef= 2 * RealPow(S_R,yy);
						specCoefTally += specCoef;

					}
				}
			}
		}
		//now apply the cell
		if (coefTally >= fData.vec2ShadowHighlight.x)
		{
			real32 effect = 1.0f;

			if (fData.lLevels == 1 || 1 == fData.vec2ShadowHighlight.x)
			{
				//nothing to do
			}
			else
			{
				if (coefTally > 1)
					coefTally = 1;
				uint32 level = fData.lLevels * (coefTally - fData.vec2ShadowHighlight.x) / (1 - fData.vec2ShadowHighlight.x) + 1;

				if (level > fData.lLevels)
					level = fData.lLevels;

				effect = static_cast<real32>(level)/static_cast<real32>(fData.lLevels);

				effect = fData.fShadowBrightness + effect * (1 - fData.fShadowBrightness);
			}
			result.fDiffuseLight = shading.fColor * effect;
#if VERSIONNUMBER >= 0x070000
			result.fGlow = shading.fGlow;
#else
			result.fDiffuseLight += shading.fGlow;
#endif
		}
		else
		{
#if VERSIONNUMBER >= 0x050000
			result.fDiffuseLight.R = shading.fColor.R * fData.fShadowBrightness;
			result.fDiffuseLight.G = shading.fColor.G * fData.fShadowBrightness;
			result.fDiffuseLight.B = shading.fColor.B * fData.fShadowBrightness;
#else
			result.fAmbientLight.R = shading.fColor.R * fData.fShadowBrightness;
			result.fAmbientLight.G = shading.fColor.G * fData.fShadowBrightness;
			result.fAmbientLight.B = shading.fColor.B * fData.fShadowBrightness;
#endif
#if VERSIONNUMBER >= 0x070000
			result.fGlow = shading.fGlow;
#else
			result.fDiffuseLight += shading.fGlow;
#endif
		}
		if (specCoefTally >= fData.vec2ShadowHighlight.y)
		{
			result.fSpecularLight.R += shading.fSpecularColor.R;
			result.fSpecularLight.G += shading.fSpecularColor.G;
			result.fSpecularLight.B += shading.fSpecularColor.B;
		}
#if VERSIONNUMBER >= 0x070000
		result.fLightShadow.Set(1, 1, 1);
#endif

	}
}


#if VERSIONNUMBER >= 0x050000
#if VERSIONNUMBER >= 0x070000
MCCOMErr CelShader::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr CelShader::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

MCCOMErr CelShader::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}
	return MC_S_OK;
}

#if VERSIONNUMBER >= 0x070000
void CelShader::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
{
	//ambientOcclusionFactor = this->ambientOcclusionFactor;
	//indirectDiffuseColor = this->indirectDiffuseColor;
}
#else
void CelShader::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	//do nothing
}
#endif


MCCOMErr CelShader::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_Cel))
	{
		TMCCountedGetHelper<CelShader> result(ppvObj);
		result = (CelShader*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);	
}
