/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#ifndef __LightMangler__
#define __LightMangler__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "LightManglerDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "I3DShFacetMesh.h"
#include "I3DShInstance.h"
#include "DCGSharedCache.h"
#include "rendertypes.h"
#include <string.h>


struct LightManglerPublicData
{
	boolean calculateBackground;
	boolean calculateIllumination;

	TMCCountedPtr<IShParameterComponent> fullShader;	
	TMCCountedPtr<IShParameterComponent> mangleAlpha;	
	TMCCountedPtr<IShParameterComponent> mangleAmbient;	
	TMCCountedPtr<IShParameterComponent> mangleBackground;				
	TMCCountedPtr<IShParameterComponent> mangleCaustics;					
	TMCCountedPtr<IShParameterComponent> mangleDiffuse;	
	TMCCountedPtr<IShParameterComponent> mangleGlobalIllumination;		
	TMCCountedPtr<IShParameterComponent> mangleGlow;	
	TMCCountedPtr<IShParameterComponent> mangleLightShadow;				
	TMCCountedPtr<IShParameterComponent> mangleReflection;	
	TMCCountedPtr<IShParameterComponent> mangleRefraction;				
	TMCCountedPtr<IShParameterComponent> mangleSpecular;	
	TMCCountedPtr<IShParameterComponent> mangleSubsurfaceScattering;	


    LightManglerPublicData()
	{
		calculateBackground = true;
		calculateIllumination = true;
	};

	boolean  operator==	(const LightManglerPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(LightManglerPublicData)) == 0;
	}
};



extern const MCGUID CLSID_LightMangler;


class LightMangler : public TBasicShader
{
	public:
		LightMangler();
		~LightMangler();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(LightManglerPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer				();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged				();

		virtual boolean			MCCOMAPI    IsEqualTo							(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags						(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput				(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
		virtual MCCOMErr		MCCOMAPI	DoShade								(ShadingOut& result,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI	WantsTransform						() { return false; };
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
		virtual void MCCOMAPI CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void MCCOMAPI CalculateReflection(TMCColorRGB& reflectionColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateCaustics(TMCColorRGB& causticColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateTransparency(TMCColorRGB& resColor,real32& outAlpha,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateSubsurfaceScattering(TMCColorRGB& subsurfaceColor,const TMCColorRGB& currentColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateAlpha(LightingDetail& result,const LightingContext& lightingContext,const ShadingOut& shading,TAbsorptionFunction* absorptionFunction);

	protected:
		LightManglerPublicData fData;
		ShadingFlags childFlags;
		EShaderOutput fullChildOutput, childOutputSpecular, childOutputDiffuse, childOutputGlow, childOutputAmbient, childOutputReflection
			, childOutputBackground, childOutputLightShadow, childOutputRefraction, childOutputGlobalIllumination, childOutputCaustics
			, childOutputSubsurfaceScattering, childOutputAmbientOcclusionFactor, childOutputAlpha;
		TMCCountedPtr<I3DExShader> fullShader;
		TMCCountedPtr<I3DExShader> mangleSpecular;
		TMCCountedPtr<I3DExShader> mangleDiffuse;
		TMCCountedPtr<I3DExShader> mangleGlow;
		TMCCountedPtr<I3DExShader> mangleAmbient;
		TMCCountedPtr<I3DExShader> mangleReflection;
		TMCCountedPtr<I3DExShader> mangleBackground;				
		TMCCountedPtr<I3DExShader> mangleLightShadow;				
		TMCCountedPtr<I3DExShader> mangleRefraction;				
		TMCCountedPtr<I3DExShader> mangleGlobalIllumination;		
		TMCCountedPtr<I3DExShader> mangleCaustics;					
		TMCCountedPtr<I3DExShader> mangleSubsurfaceScattering;	
		TMCCountedPtr<I3DExShader> mangleAlpha;

		I3DShLightingModel* defaultlightingmodel;
		TMCColorRGB reflectionColor;
		TMCColorRGB indirectDiffuseColor;
		real ambientOcclusionFactor;
		TMCColorRGB transparencyColor;
		real32 transparencyAlpha;
		TMCColorRGB causticColor;
		TMCColorRGB subsurfaceColor;
		LightingDetail*  resultLocal;
		TAbsorptionFunction* absorptionFunction;
		//real32 alpha;

		void extensionDataChangedDoMangler(I3DExShader** subShader, IShParameterComponent* parameter, EShaderOutput& childOutput);
		void mangle(I3DExShader* subShader, const EShaderOutput childOutput, TMCColorRGB& result, ShadingIn& shadingIn);
		void mangle(I3DExShader* subShader, const EShaderOutput childOutput, real& result, ShadingIn& shadingIn);

};

#endif