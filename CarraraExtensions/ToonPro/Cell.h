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
#ifndef __Cell__
#define __Cell__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "CellDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"

struct CelPublicData{
	TVector2 vec2ShadowHighlight;
	int32 lLevels;
	real32 fShadowBrightness;
	TMCCountedPtr<IShParameterComponent> param;	
	};

extern const MCGUID CLSID_Cel;
extern const MCGUID IID_Cel;


class CelShader : public TBasicShader {
	public:
		CelShader();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(CelPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
#if VERSIONNUMBER >= 0x070000
		virtual void MCCOMAPI CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
#elif VERSIONNUMBER >= 0x050000
		virtual void MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual MCCOMErr		MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel);
#else
		virtual void CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#endif
		virtual MCCOMErr		MCCOMAPI DoShade							(ShadingOut& result,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };
			virtual MCCOMErr 		MCCOMAPI	QueryInterface(const MCIID &riid, void** ppvObj);

	protected:
		CelPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;

#if VERSIONNUMBER >= 0x070000
		real ambientOcclusionFactor;
		TMCColorRGB indirectDiffuseColor;
#endif
#if VERSIONNUMBER >= 0x050000
		I3DShLightingModel* defaultlightingmodel;
#endif
	};

#endif