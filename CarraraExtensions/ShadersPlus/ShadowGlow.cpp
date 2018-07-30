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
#include "ShadowGlow.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_ShadowGlow(R_CLSID_ShadowGlow);
#else
const MCGUID CLSID_ShadowGlow={R_CLSID_ShadowGlow};
#endif

ShadowGlow::ShadowGlow() 
{
	fData.fEffect = 1.0f;
#if VERSIONNUMBER >= 0x050000
	defaultlightingmodel = NULL;
#endif
}

void* ShadowGlow::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr ShadowGlow::ExtensionDataChanged()
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

boolean	ShadowGlow::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<ShadowGlow*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<ShadowGlow*>(aShader))->fData.fEffect == fData.fEffect)

		);
}  

MCCOMErr ShadowGlow::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput ShadowGlow::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting + kUseCalculateIndirectLighting
#endif
	);
}

boolean ShadowGlow::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

#if VERSIONNUMBER >= 0x050000
#if VERSIONNUMBER >= 0x070000
MCCOMErr ShadowGlow::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr ShadowGlow::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

void ShadowGlow::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{

	//defaultlightingmodel->CalculateDirectLighting(result, lightingContext);
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

		const TMCColorRGBA& ambientLight = lightingContext.GetAmbientLight();

		// this attenuates the ambient light to look more like a distant, in case no other lights
		real norme= (shading.fChangedNormal[2]+shading.fChangedNormal[0]) * 0.5f;

		if (norme<0.0f) norme=-norme;
		norme= 0.5f*(norme-0.5f)+shading.fAmbient;

		if (norme<0.0f) norme=0.0f;
		if (norme>1.0f) norme=1.0f;

		// compute the base color
#if VERSIONNUMBER >= 0x050000
		result.fAmbient.R = shading.fColor.R * ambientLight.R * norme;// + shading.fGlow.R;
		result.fAmbient.G = shading.fColor.G * ambientLight.G * norme;// + shading.fGlow.G;
		result.fAmbient.B = shading.fColor.B * ambientLight.B * norme;// + shading.fGlow.B;
#else
		result.fAmbientLight.R = shading.fColor.R * ambientLight.R * norme;// + shading.fGlow.R;
		result.fAmbientLight.G = shading.fColor.G * ambientLight.G * norme;// + shading.fGlow.G;
		result.fAmbientLight.B = shading.fColor.B * ambientLight.B * norme;// + shading.fGlow.B;
#endif
		//  compute the lighting of each light

		const int32 lightCount = raytracer->GetLightCount();

		result.fDiffuseLight = TMCColorRGB::kBlack;
		result.fSpecularLight = TMCColorRGB::kBlack;

		TMCColorRGB lighttally = ambientLight;

		//if we have indirect lighting then get those values to add to our lighting level
		if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting 
			&& (shading.fColor.R>0.02f || shading.fColor.G>0.02f || shading.fColor.B>0.02f))
		{
#if (VERSIONNUMBER >= 0x070000)
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#elif (VERSIONNUMBER >= 0x050000)
			defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#else
			TBasicShader::CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
			lighttally += indirectDiffuseColor;
		}

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
		{
#if VERSIONNUMBER >= 0x050000
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,illumSettings.fShadowsOn, false))
#else
			if (raytracer->GetLightIntensity(directLighting
					,lightIndex,lightingContext,illumSettings.fShadowsOn))
#endif
			{
				result.fIsInShadow |= directLighting.fIsInShadow;

				const real coef= (shading.fChangedNormal * lightDirection) * shading.fLambert;

				if (coef>0)
				{
					lighttally += lighting * coef;

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
							result.fSpecularLight.R += specCoef * noShadowlighting.R * S_R;
							result.fSpecularLight.G += specCoef * noShadowlighting.G * S_R;
							result.fSpecularLight.B += specCoef * noShadowlighting.B * S_R;

							specShadow.R += specCoef * lighting.R * S_R;
							specShadow.G += specCoef * lighting.G * S_R;
							specShadow.B += specCoef * lighting.B * S_R;
						}
						else
						{
#endif
							result.fSpecularLight.R += lighting.R * specCoef * S_R;
							result.fSpecularLight.G += lighting.G * specCoef * S_R;
							result.fSpecularLight.B += lighting.B * specCoef * S_R;
#if VERSIONNUMBER >= 0x070000
						}
#endif
					}
				}
			}
		}//end for lights
		TMCColorRGB glow;

		if (fData.fEffect == 0)
		{
			glow = shading.fGlow;

			if (lighttally.R > 0)
				glow.R = 0;
			if (lighttally.B > 0)
				glow.B = 0;
			if (lighttally.G > 0)
				glow.G = 0;
		}
		else if (fData.fEffect > 0)
		{
			glow = shading.fGlow - lighttally / fData.fEffect;
			
			if (glow.R < 0) glow.R = 0;
			if (glow.B < 0) glow.B = 0;
			if (glow.G < 0) glow.G = 0;

		}
		else
		{
			glow = lighttally *(-fData.fEffect);
			
			if (glow.R > shading.fGlow.R) glow.R = shading.fGlow.R;
			if (glow.B > shading.fGlow.B) glow.B = shading.fGlow.B;
			if (glow.G > shading.fGlow.G) glow.G = shading.fGlow.G;
		}
#if VERSIONNUMBER >= 0x070000
		result.fGlow = glow;
#elif VERSIONNUMBER >= 0x050000
		result.fAmbient += glow;
#else
		result.fAmbientLight += glow;
#endif
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

MCCOMErr ShadowGlow::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{

	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}

	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x070000)
void ShadowGlow::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void ShadowGlow::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
	const LightingFlags lightingFlags = lightingContext.fLightingFlags;
	if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting 
			&& (shading.fColor.R>0.02f || shading.fColor.G>0.02f || shading.fColor.B>0.02f))
	{
		indirectDiffuseColor = this->indirectDiffuseColor;

#if VERSIONNUMBER >= 0x070000
		ambientOcclusionFactor = this->ambientOcclusionFactor;
#endif
	}
}
