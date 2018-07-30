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
#include "FauxAnisotropic.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "ShadersPlusdll.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_FauxAnisotropic(R_CLSID_FauxAnisotropic);
#else
const MCGUID CLSID_FauxAnisotropic={R_CLSID_FauxAnisotropic};
#endif

FauxAnisotropic::FauxAnisotropic() 
{
	fData.fRoughnessY = 0.075f;
	fData.fRoughnessX = 1.0f;
}

void* FauxAnisotropic::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr FauxAnisotropic::ExtensionDataChanged()
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

boolean	FauxAnisotropic::IsEqualTo(I3DExShader* aShader)
{
	return (
		
		((static_cast<FauxAnisotropic*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<FauxAnisotropic*>(aShader))->fData.fRoughnessX == fData.fRoughnessX)
		&&
		((static_cast<FauxAnisotropic*>(aShader))->fData.fRoughnessY == fData.fRoughnessY)
		
		);
}  


MCCOMErr FauxAnisotropic::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsNormalDerivative = true;
	theFlags.fNeedsNormalLoc = true;
	theFlags.fNeedsNormalLocDerivative = true;
	theFlags.fNeedsPixelRatio = true;
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fNeedsNormalLoc = true;
	theFlags.fNeedsIsoUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput FauxAnisotropic::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
#endif
	);
}

boolean FauxAnisotropic::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

//void FauxAnisotropic::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader)
void FauxAnisotropic::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
	LightingFlags lightingFlags = lightingContext.fLightingFlags;
	ShadingIn shin = static_cast<ShadingIn>(*lightingContext.fHit);

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

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
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
						real32 thetai = (lightDirection) * shading.fChangedNormal;
						real32 thetar = (lightingContext.fReflectDir) * shading.fChangedNormal;
						real32 alphax = fData.fRoughnessX;
						real32 alphay = fData.fRoughnessY; 


						TVector3 xnormal = shading.fChangedNormal ^ shin.fPointx;//shading.fChangedNormal ^ shin.fIsoU;
						TVector3 ynormal = shading.fChangedNormal ^ shin.fPointy;//shading.fChangedNormal ^ shin.fIsoV;
						xnormal.Normalize(xnormal);
						ynormal.Normalize(ynormal);

						TVector3 hnormal = (lightingContext.fReflectDir) + (lightDirection);
						TVector3 nnormal = shading.fChangedNormal;
						hnormal.Normalize(hnormal);
						nnormal.Normalize(nnormal);
						real32 value = ( 1.0f / sqrt(thetai * thetar)) * (1.0f /( 4.0f * PI * alphax * alphay));
						real32 exp = -2.0f * (pow((float)((hnormal * xnormal) / alphax), 2.0f) + pow((float)((hnormal * ynormal) / alphay), 2.0f)) / (1.0f + hnormal * nnormal);

						real32 specCoef =  value * pow(10.0f, (float)exp);

						//const real yy=(0.001f+shading.fSpecularSize*shading.fSpecularSize)* kSpecularPowerCoef;

						//const real specCoef= 2 * RealPow(S_R,yy);
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
		}//end loop
		result.fSpecularLight += result.fDiffuseLight / PI;
#if VERSIONNUMBER >= 0x070000
		if (illumSettings.fComputeSeparatedShadow)
		{
			specShadow += diffShadow / PI;
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

MCCOMErr FauxAnisotropic::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}
	return MC_S_OK;
}




