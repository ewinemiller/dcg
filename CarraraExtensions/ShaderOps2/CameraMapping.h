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
#ifndef __CameraMapping__
#define __CameraMapping__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif     

#include "CameraMappingDef.h"
#include "IShComponent.h"
#include "copyright.h"
#include "I3DShShader.h"
#include "MCCountedPtrArray.h"
#include "MultiOpHelper.h"
#include "SingleOpHelper.h"


struct CameraMappingData
{
	int32 direction;
	TMatrix33SymQuaternion mappingTransform;
	boolean ignoreNormals;
	ActionNumber uvSpaceId;
	TMCCountedPtrArray<IShParameterComponent> childShaders;	
	int32 requestor;
	TMCString255 cameraName;
	int32 xRatio, yRatio;

	boolean  operator==	(const CameraMappingData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(CameraMappingData)) == 0;
	}
};

extern const MCGUID CLSID_CameraMapping;
extern const MCGUID CLSID_CameraMappingInChannel;
 
class CameraMapping : public MultiOpHelper<CameraMappingData>
{
	public:
		CameraMapping() 
		{
			fData.uvSpaceId = -2;
			fData.xRatio = fData.yRatio = 1;
		};
		~CameraMapping() {};
		STANDARD_RELEASE;
		virtual MCCOMErr MCCOMAPI GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput MCCOMAPI GetImplementedOutput();

		virtual real MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetVector(TVector3& result,ShadingIn& shadingIn);
		virtual MCCOMErr MCCOMAPI DoShade(ShadingOut& result,ShadingIn& shadingIn);
		virtual void MCCOMAPI GetReflection(TReflectionParameters& reflectionParams,const ShadingIn& shadingIn);
		virtual void MCCOMAPI GetTransparency(TTransparencyParameters& transparencyParams,const ShadingIn& shadingIn);
		virtual void MCCOMAPI GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn);
		virtual void MCCOMAPI GetShaderApproximation(ShadingOut& result,ShadingIn& shadingIn);
		virtual MCCOMErr		MCCOMAPI GetBump							(TVector3& result,ShadingIn& shadingIn);
		virtual boolean	MCCOMAPI HasExplicitLightingModel(const LightingContext& lightingContext);
		virtual void MCCOMAPI CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
		virtual void MCCOMAPI CalculateReflection(TMCColorRGB& reflectionColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateCaustics(TMCColorRGB& causticColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateTransparency(TMCColorRGB& resColor,real32& outAlpha,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual void MCCOMAPI CalculateAlpha(LightingDetail& result,const LightingContext& lightingContext,const ShadingOut& shading,TAbsorptionFunction* absorptionFunction);
		virtual void MCCOMAPI CalculateSubsurfaceScattering(TMCColorRGB& subsurfaceColor,const TMCColorRGB& currentColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);

		virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);

	protected:
		virtual const ShadingIn getShadingInToUse(const ShadingIn& shadingIn);
};

#endif