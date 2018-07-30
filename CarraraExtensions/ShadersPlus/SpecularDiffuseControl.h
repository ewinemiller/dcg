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
#ifndef __SpecularDiffuseControl__
#define __SpecularDiffuseControl__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "SpecularDiffuseControlDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"
#include "I3dShScene.h"
#include "IShTokenStream.h"
#include "IMFListPart.h"
#include <string.h>
#include "MCClassArray.h"

struct SpecularDiffuseControlPublicData
{
	real32 ambientLight;
	real32 giLight;
	boolean synchAllLights;
	TMCCountedPtr<IShParameterComponent> param;	
	boolean  operator==	(const SpecularDiffuseControlPublicData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(SpecularDiffuseControlPublicData)) == 0;
	}
};


//MCDEFINE_GUID2(CLSID_SpecularDiffuseControl, R_CLSID_SpecularDiffuseControl);
extern const MCGUID CLSID_SpecularDiffuseControl;

class SpecularDiffuseLevel 
{
public:
	SpecularDiffuseLevel()
	{
		fSpecular = 1.0f;
		fDiffuse = 1.0f;
		shadowIntensity = 1.0f;
	}

	TMCString255 name;
	real32 fSpecular;
	real32 fDiffuse;
	real32 shadowIntensity;
};

class SpecularDiffuseControl : public TBasicShader 
	, public IExStreamIO 
{
public:
	SpecularDiffuseControl();
	STANDARD_RELEASE;
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
		const        { return sizeof(SpecularDiffuseControlPublicData); }
	virtual void*			MCCOMAPI    GetExtensionDataBuffer();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
	virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
	virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
	virtual void	MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter);
	virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  
	virtual MCCOMErr		MCCOMAPI	Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr		MCCOMAPI	Write(IShTokenStream* stream);
#if (VERSIONNUMBER >= 0x060000)
	virtual MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext) {return MC_S_OK;};
#endif
	virtual uint32 			MCCOMAPI	AddRef();
	virtual MCCOMErr 		MCCOMAPI	QueryInterface(const MCIID &riid, void** ppvObj);
#if VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#if VERSIONNUMBER >= 0x070000
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
#else
		virtual void			MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
		virtual MCCOMErr		MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel);
#endif
#else
	virtual void			CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
	virtual void			CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#endif
	virtual MCCOMErr		MCCOMAPI DoShade							(ShadingOut& result,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI WantsTransform						() { return false; };

protected:
	SpecularDiffuseControlPublicData fData;
	ShadingFlags ChildFlags;
	EShaderOutput ChildOutput;
	TMCCountedPtr<I3DExShader> shader;
#if VERSIONNUMBER >= 0x050000
	I3DShLightingModel* defaultlightingmodel;
#endif
	TMCCountedPtr<I3DShScene> scene;
	TMCClassArray<SpecularDiffuseLevel> levels;
	void mergeLevelsWithScene();
	void fillLightList(TMCCountedPtr<IMFListPart> list);
	void CopyDataFrom(const SpecularDiffuseControl* source);
};

#endif