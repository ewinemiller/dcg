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
#ifndef __ProceduralLock__
#define __ProceduralLock__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ProceduralLockDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "I3DShFacetMesh.h"
#include "I3DShInstance.h"
#include "DCGSharedCache.h"

struct ProceduralLockPublicData
{
	TMCCountedPtr<IShParameterComponent> param;	
};

struct ProceduralLockCache
{
	TMCCountedPtr<FacetMesh> mesh;		
	TMCCountedPtr<I3DShInstance> fakeInstance;


	ProceduralLockCache()
	{
	};
	~ProceduralLockCache()
	{
		cleanup();
	};
	void cleanup()
	{
		fakeInstance = NULL;
		mesh = NULL;
	};

};

struct ProceduralLockKey {
	void* instance;

	ProceduralLockKey()
	{
		this->instance = NULL;
	};

	void fill(void* instance)
	{
		this->instance = instance;
	};

	boolean  operator==	(const ProceduralLockKey& rhs) 
	{ 
		return (this->instance == rhs.instance);
	};
};

extern DCGSharedCache<ProceduralLockCache, LightingContext, ProceduralLockKey> plCache;


extern const MCGUID CLSID_ProceduralLock;


class ProceduralLock : public TBasicShader, public cTransformer
	, public DCGSharedCacheClient<ProceduralLockCache, LightingContext, ProceduralLockKey>
{
	public:
		ProceduralLock();
		~ProceduralLock();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(ProceduralLockPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer				();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged				();
		virtual boolean			MCCOMAPI    IsEqualTo							(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags						(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput				(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
		virtual MCCOMErr		MCCOMAPI	DoShade								(ShadingOut& result,ShadingIn& shadingIn);
#if VERSIONNUMBER >= 0x070000
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
#elif VERSIONNUMBER >= 0x050000
		virtual MCCOMErr		MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel);
#else
		virtual void			MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader);
#endif
#if VERSIONNUMBER >= 0x070000
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
#elif VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#else
		virtual void			CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#endif
#if VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#else
		virtual void			CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#endif
		virtual boolean			MCCOMAPI	WantsTransform						() { return false; };

		void fillElement(ProceduralLockCache& newElement, const LightingContext& lightingContext);
		void emptyElement(ProceduralLockCache& oldElement);

	protected:
		ProceduralLockPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;

		TVector3 fPointLoc;
		TMCCountedPtr<I3DShInstance> fakeInstance;
#if (VERSIONNUMBER >= 0x050000)
			I3DShLightingModel* defaultlightingmodel;
#endif
};

#endif