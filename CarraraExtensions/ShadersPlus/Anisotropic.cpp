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
#include "Anisotropic.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "ShadersPlusdll.h"
#include "I3DShFacetMesh.h"
#include "DCGUtil.h"
#include "publicutilities.h"
#include "WireframeFacetMeshAccumulator.h"



#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Anisotropic(R_CLSID_Anisotropic);
#else
const MCGUID CLSID_Anisotropic={R_CLSID_Anisotropic};
#endif

#define EXPIRE_INSTANCE_LIMIT 10000

Anisotropic::Anisotropic() 
{
	fData.fRoughnessY = 0.075f;
	fData.fRoughnessX = 1.0f;

}

void* Anisotropic::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr Anisotropic::ExtensionDataChanged()
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

boolean	Anisotropic::IsEqualTo(I3DExShader* aShader)
{
	return (
		
		((static_cast<Anisotropic*>(aShader))->fData.param == fData.param)
		&&
		((static_cast<Anisotropic*>(aShader))->fData.fRoughnessX == fData.fRoughnessX)
		&&
		((static_cast<Anisotropic*>(aShader))->fData.fRoughnessY == fData.fRoughnessY)
		
		);
}  


MCCOMErr Anisotropic::GetShadingFlags(ShadingFlags& theFlags)
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

EShaderOutput Anisotropic::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
#endif
	);
}

boolean Anisotropic::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

void Anisotropic::BuildCache(instanceCache& instance, const LightingContext& lightingContext)
{
	FacetMesh*	fFacetMesh = lightingContext.fHit->fFacetMesh;
	uint32 maxvertices = fFacetMesh->fVertices.GetElemCount();

	instance.myinstance = static_cast<ShadingIn>(*lightingContext.fHit).fInstance;
	instance.active = true;
	instance.sincelastcall = 0;


	uint32 pointCount;

	{//so the accumulator goes out of scope when it's done
		WireframeFacetMeshAccumulator accu;
		FacetMeshFacetIterator facetIterator;

		facetIterator.Initialize(fFacetMesh);			
		accu.PrepareAccumulation(fFacetMesh->fFacets.GetElemCount());
		for (facetIterator.First(); facetIterator.More(); facetIterator.Next()) 
		{
			TFacet3D aFacet = facetIterator.GetFacet();
			accu.AccumulateFacet(&aFacet);
		}
		{
			TMCCountedPtr<FacetMesh> junkMesh;		
			accu.MakeFacetMesh(&junkMesh);
			pointCount = junkMesh->fVertices.GetElemCount();
		}

		instance.tangentindex.SetElemCount(maxvertices);
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			instance.tangentindex[vertexindex] = accu.GetVertexIndex(fFacetMesh->fVertices[vertexindex]);
		}

	}//end accu life span

	//initialize our vectors
	instance.tangent.SetElemCount(pointCount);
	instance.binormal.SetElemCount(pointCount);
	for (uint32 vectorindex = 0; vectorindex < pointCount; vectorindex++)
	{
		instance.tangent[vectorindex].SetValues(0,0,0);
		instance.binormal[vectorindex].SetValues(0,0,0);
	}

	//now spin through the facets and look for facets where one of those is a member
	uint32 maxfacets = fFacetMesh->fFacets.GetElemCount();
	for (uint32 fFacetIndex = 0; fFacetIndex < maxfacets; fFacetIndex++)
	{
		//first calculate the tangental vector and binormal vector at the vertices
		const uint32& pi1 = fFacetMesh->fFacets[fFacetIndex].pt1;
		const uint32& pi2 = fFacetMesh->fFacets[fFacetIndex].pt2;
		const uint32& pi3 = fFacetMesh->fFacets[fFacetIndex].pt3;
		const TVector3& pt1 = fFacetMesh->fVertices[pi1]; 
		const TVector3& pt2 = fFacetMesh->fVertices[pi2]; 
		const TVector3& pt3 = fFacetMesh->fVertices[pi3]; 

		const TVector2& uv1 = fFacetMesh->fuv[pi1]; 
		const TVector2& uv2 = fFacetMesh->fuv[pi2]; 
		const TVector2& uv3 = fFacetMesh->fuv[pi3]; 

		real32 x1 = pt2.x - pt1.x;
		real32 x2 = pt3.x - pt1.x;
		real32 y1 = pt2.y - pt1.y;
		real32 y2 = pt3.y - pt1.y;
		real32 z1 = pt2.z - pt1.z;
		real32 z2 = pt3.z - pt1.z;
		
		real32 s1 = uv2.x - uv1.x;
		real32 s2 = uv3.x - uv1.x;
		real32 t1 = uv2.y - uv1.y;
		real32 t2 = uv3.y - uv1.y;
		
		if (s1 * t2 - s2 * t1 != 0)
		{
			real32 r = 1.0F / (s1 * t2 - s2 * t1);
			TVector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
					(t2 * z1 - t1 * z2) * r);
			TVector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
					(s1 * z2 - s2 * z1) * r);
			
			instance.tangent[instance.tangentindex[pi1]] += sdir;
			instance.binormal[instance.tangentindex[pi1]] += tdir;
			instance.tangent[instance.tangentindex[pi2]] += sdir;
			instance.binormal[instance.tangentindex[pi2]] += tdir;
			instance.tangent[instance.tangentindex[pi3]] += sdir;
			instance.binormal[instance.tangentindex[pi3]] += tdir;
		}
	}//end facet loop

	//normalize our results
	for (uint32 vectorindex = 0; vectorindex < pointCount; vectorindex++)
	{
		instance.tangent[vectorindex].Normalize(instance.tangent[vectorindex]);
		instance.binormal[vectorindex].Normalize(instance.binormal[vectorindex]);
	}


}


//void Anisotropic::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader)
void Anisotropic::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif
	LightingFlags lightingFlags = lightingContext.fLightingFlags;
	instanceCache* thisinstance = NULL;

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
		ShadingIn shin = static_cast<ShadingIn>(*lightingContext.fHit);
		if (lightingContext.fHit->fFacetMesh != NULL)
		{
			
			//find the instance in our cache
			uint32 instancecount = instances.GetElemCount();
			void* localinstance = shin.fInstance;
			for (uint i = 0; i < instancecount; i++)
			{
				instances[i].sincelastcall++;
				
				if (instances[i].myinstance == localinstance)
				{
					thisinstance = &instances[i];
					instances[i].sincelastcall = 0;
				}
				if (instances[i].sincelastcall > EXPIRE_INSTANCE_LIMIT)
				{
					instances[i].active = false;
					instances[i].myinstance = 0;
					instances[i].tangent.ArrayFree();
					instances[i].binormal.ArrayFree();
					instances[i].tangentindex.ArrayFree();
				}
			}
			if (thisinstance == NULL)
			{
				//first look for a blank one
				for (uint i = 0; i < instancecount; i++)
				{
					if (instances[i].active == false)
					{
						thisinstance = &instances[i];
						BuildCache(*thisinstance, lightingContext);
						i = instancecount;
					}
				}
				//still haven't found anything, add a new one to the array
				if (thisinstance == NULL)
				{
					instances.AddElemCount(1);
					thisinstance = &instances[instancecount];
					BuildCache(*thisinstance, lightingContext);
				}
			}//end new instance

		}

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

		result.fDiffuseLight = TMCColorRGB::kBlack;
		result.fSpecularLight = TMCColorRGB::kBlack;
		
		TVector3 UVX, UVY;
		real32 alphax = fData.fRoughnessX;
		real32 alphay = fData.fRoughnessY; 
		real32 thetar = lightingContext.fReflectDir * lightingContext.fNormal;
		TVector3 nnormal = lightingContext.fNormal;
		nnormal.Normalize(nnormal);

		if (lightingContext.fHit->fFacetMesh != NULL)
		{
			FacetMesh*	fFacetMesh = lightingContext.fHit->fFacetMesh;

			//these are the vertex indices we care about

			uint32 vi[3];

			vi[0] = fFacetMesh->fFacets[lightingContext.fHit->fFacetIndex].pt1;
			vi[1] = fFacetMesh->fFacets[lightingContext.fHit->fFacetIndex].pt2;
			vi[2] = fFacetMesh->fFacets[lightingContext.fHit->fFacetIndex].pt3;

			const TVector3& vert1 = fFacetMesh->fVertices[vi[0]]; 
			const TVector3& vert2 = fFacetMesh->fVertices[vi[1]]; 
			const TVector3& vert3 = fFacetMesh->fVertices[vi[2]]; 


			vi[0] = thisinstance->tangentindex[vi[0]];
			vi[1] = thisinstance->tangentindex[vi[1]];
			vi[2] = thisinstance->tangentindex[vi[2]];
			UVX = thisinstance->tangent[vi[0]] * lightingContext.fHit->fBaryCoord[0] 
				+ thisinstance->tangent[vi[1]] * lightingContext.fHit->fBaryCoord[1]
				+ thisinstance->tangent[vi[2]] * lightingContext.fHit->fBaryCoord[2];
			UVY = thisinstance->binormal[vi[0]] * lightingContext.fHit->fBaryCoord[0]
				+ thisinstance->binormal[vi[1]] * lightingContext.fHit->fBaryCoord[1]
				+ thisinstance->binormal[vi[2]] * lightingContext.fHit->fBaryCoord[2];

		}
		else
		{
			UVX = lightingContext.fHit->fUVDerivative[0];
			UVY = lightingContext.fHit->fUVDerivative[1];
		}
		LocalToGlobalVector(lightingContext.fHit->fT, UVX, UVX);
		LocalToGlobalVector(lightingContext.fHit->fT, UVY, UVY);

		UVX.Normalize(UVX);
		UVY.Normalize(UVY);

		//  compute the lighting of each light
		const int32 lightCount = raytracer->GetLightCount();
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
						real32 thetai = lightDirection * lightingContext.fNormal;

						TVector3 hnormal = lightingContext.fReflectDir + lightDirection;
						hnormal.Normalize(hnormal);
						real32 value = ( 1.0f / sqrt(thetai * thetar)) * (1.0f /( 4.0f * PI * alphax * alphay));
						real32 exp = -2.0f * (pow((float)((hnormal * UVX) / alphax), 2.0f) 
								+ pow((float)((hnormal * UVY) / alphay), 2.0f)) / (1.0f + hnormal * nnormal);



						real32 specCoef =  value * pow(2.71828183f, (float)exp);

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

MCCOMErr Anisotropic::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}
	return MC_S_OK;
}



