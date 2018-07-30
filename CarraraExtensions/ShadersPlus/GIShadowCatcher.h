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
#ifndef __GIShadowCatcher__
#define __GIShadowCatcher__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GIShadowCatcherDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "I3dShScene.h"


struct GIShadowCatcherPublicData
{
	real32 fBrightness;
	uint32 lSubShaderUse;
	TMCCountedPtr<IShParameterComponent> param;	
};

enum SubShaderUse {ssIgnore = 0, ssTint = 1, ssReplace = 2};

//MCDEFINE_GUID2(CLSID_GIShadowCatcher, R_CLSID_GIShadowCatcher);
extern const MCGUID CLSID_GIShadowCatcher;


class GIShadowCatcher : public TBasicShader, public cTransformer {
	public:
		GIShadowCatcher();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(GIShadowCatcherPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
#if VERSIONNUMBER >= 0x070000
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void			MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
		virtual void			MCCOMAPI	InternalCalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor,const LightingContext& lightingContext,const ShadingOut& shading);
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
#elif VERSIONNUMBER >= 0x050000
		virtual MCCOMErr		MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel);
		virtual void			MCCOMAPI	InternalCalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext& lightingContext,const ShadingOut& shading);
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void			MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#else
		virtual void			CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void			CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#endif


		virtual void			MCCOMAPI GetShaderApproxColor				(TMCColorRGBA& result,ShadingIn& shadingIn)
		{
			if (shader != NULL)
			{
				boolean fullArea = false;
				shader->GetColor(result, fullArea, shadingIn);
			}
		};
		virtual MCCOMErr		MCCOMAPI DoShade							(ShadingOut& result,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };

	protected:
		GIShadowCatcherPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
		TVector3 changedNormal;
		TMCCountedPtr<I3DShScene> scene;
		TMCColorRGB indirectDiffuseColor;

#if VERSIONNUMBER >= 0x07000
		real32 ambientOcclusionFactor;
#endif

#if VERSIONNUMBER >= 0x050000
			I3DShLightingModel* defaultlightingmodel;
#endif
	};

#endif