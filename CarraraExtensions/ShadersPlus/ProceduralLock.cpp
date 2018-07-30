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
#include "ProceduralLock.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "rendertypes.h"
#include "I3dExRenderFeature.h"
#include "ShadersPlusDll.h"
#include "I3DExVertexPrimitive.h"
#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3dExPrimitive.h"
#include "comutilities.h"
#include "ishutilities.h"
#include "I3DShMasterGroup.h"
#include "I3dShTreeElement.h"
#include "dcgmeshutil.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"

#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_ProceduralLock(R_CLSID_ProceduralLock);
#else
const MCGUID CLSID_ProceduralLock={R_CLSID_ProceduralLock};
#endif

#define EXPIRE_INSTANCE_LIMIT 10000

DCGSharedCache<ProceduralLockCache, LightingContext, ProceduralLockKey> plCache;


ProceduralLock::ProceduralLock() :DCGSharedCacheClient<ProceduralLockCache, LightingContext, ProceduralLockKey>(plCache)
{
}

ProceduralLock::~ProceduralLock() 
{
	releaseCache();
}

void* ProceduralLock::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr ProceduralLock::ExtensionDataChanged()
{
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}
	releaseCache();
	return MC_S_OK;
}

boolean	ProceduralLock::IsEqualTo(I3DExShader* aShader)
{
	return (
		((static_cast<ProceduralLock*>(aShader))->fData.param == fData.param)

		);
}  

MCCOMErr ProceduralLock::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput ProceduralLock::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
	+ kUseCalculateIndirectLighting
#endif
	);
}

boolean ProceduralLock::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

#if VERSIONNUMBER >= 0x070000
MCCOMErr ProceduralLock::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#elif VERSIONNUMBER >= 0x050000
MCCOMErr ProceduralLock::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#else
void ProceduralLock::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader)
#endif
{
	#if (VERSIONNUMBER >= 0x050000)
		defaultlightingmodel = inDefaultLightingModel;
	#endif
	if (shader != NULL)
	{
		ShadingIn shadingIn = static_cast<ShadingIn>(*lightingContext.fHit);
		fPointLoc = shadingIn.fPointLoc;
		fakeInstance = shadingIn.fInstance;
		//adjust the local coordinates
		if (lightingContext.fHit->fFacetMesh != NULL)
		{
			if (cacheElement == NULL || globalStorageKey.instance != shadingIn.fInstance)
			{
				releaseCache();
				globalStorageKey.fill(shadingIn.fInstance);
				getCache(lightingContext);
			}

			if (cacheElement->mesh != NULL)
			{
				//translate the local coordinates into their pre-boned/morph target position
				FacetMesh*	fFacetMesh = lightingContext.fHit->fFacetMesh;
				const int32 lFacetIndex = lightingContext.fHit->fFacetIndex;

				//these are the vertex indices we care about
				
				const uint32 vi[3] = {fFacetMesh->fFacets[lFacetIndex].pt1, fFacetMesh->fFacets[lFacetIndex].pt2, fFacetMesh->fFacets[lFacetIndex].pt3};
				const TVector3& vert1 = cacheElement->mesh->fVertices[vi[0]]; 
				const TVector3& vert2 = cacheElement->mesh->fVertices[vi[1]]; 
				const TVector3& vert3 = cacheElement->mesh->fVertices[vi[2]]; 
				fPointLoc = vert1 * lightingContext.fHit->fBaryCoord[0] + vert2 * lightingContext.fHit->fBaryCoord[1] + vert3 * lightingContext.fHit->fBaryCoord[2];
				fakeInstance = cacheElement->fakeInstance;
			}
		}
		
#if VERSIONNUMBER >= 0x050000
		//shader->ShadeAndLight(result, lightingContext, inDefaultLightingModel);
		//DoShade(*result.fShadingOut, static_cast<ShadingIn>(*lightingContext.fHit));
		//(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateDirectLighting(result, lightingContext);
#else
		shader->ShadeAndLight(result, lightingContext, inShader);
#endif

	}
#if VERSIONNUMBER >= 0x050000
	return MC_E_NOTIMPL;
#endif
}

void ProceduralLock::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
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
}

#if (VERSIONNUMBER >= 0x070000)
void ProceduralLock::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void ProceduralLock::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
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


MCCOMErr ProceduralLock::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{

	if (shader != NULL)
	{

		TVector3 tempPoint = shadingIn.fPointLoc;
		TMCCountedPtr<I3DShInstance> tempInstance;

		tempInstance = shadingIn.fInstance;
		shadingIn.fPointLoc = fPointLoc;
		if (fakeInstance != NULL)
		{
			shadingIn.fInstance  = fakeInstance;
		}
		shader->DoShade(result, shadingIn);		
		shadingIn.fPointLoc = tempPoint;
		shadingIn.fInstance = tempInstance;
		fakeInstance = NULL;
	}
	return MC_S_OK;
}

void ProceduralLock::emptyElement(ProceduralLockCache& oldElement) {
	oldElement.cleanup();
}

void ProceduralLock::fillElement(ProceduralLockCache& newElement, const LightingContext& lightingContext) {
	TMCCountedPtr<I3DShObject> baseobject;
	TMCCountedPtr<I3DShObject> newobject;
	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	TMCCountedPtr<I3DShPrimitive> primitive;

	ShadingIn shin = static_cast<ShadingIn>(*lightingContext.fHit);

	if (shin.fInstance->Get3DObject(&baseobject)==MC_S_OK) {

		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<IShComponent> comp;
		TMCCountedPtr<I3DShExternalPrimitive> extprimitive;

		{
			baseobject->Clone(&newobject, kNoAnim);
		}
		ThrowIfNil(newobject);

		newobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
		ThrowIfNil(extprimitive);

		extprimitive->GetPrimitiveComponent(&comp);
		ThrowIfNil(comp);

		comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
		ThrowIfNil(primcomp);

		newobject->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
		ThrowIfNil(primitive);

		if (primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive)==MC_S_OK) 
		{
			removeBonesAndMorphs(vertexPrimitive);

#if (VERSIONNUMBER >= 0x050000)
			{
				primitive->GetRenderingFacetMesh(&newElement.mesh);
			}
#else
			primitive->GetFMesh(0.00025000001f, &newElement.mesh);
#endif
			if (newobject) {

				gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&newElement.fakeInstance);	
				ThrowIfNil(newElement.fakeInstance);
				newElement.fakeInstance->Set3DObject(newobject);
				TMCCountedPtr<I3DShTreeElement> instanceTree;
				newElement.fakeInstance->QueryInterface(IID_I3DShTreeElement, (void**) &instanceTree);
				//set the master group
				TMCCountedPtr<I3DShTreeElement> originalTree;
				TMCCountedPtr<I3DShScene> scene;
				shin.fInstance->QueryInterface(IID_I3DShTreeElement, (void**) &originalTree);
				originalTree->GetScene(&scene);
				MCAssert(scene);
				TMCCountedPtr<I3DShMasterGroup> masterGroup;
				scene->QueryInterface(IID_I3DShMasterGroup,(void**)&masterGroup);
				MCAssert(masterGroup);
				instanceTree->SetMasterGroup(masterGroup);

			}
		}


	}

}

