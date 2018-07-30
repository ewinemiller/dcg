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
#ifndef __FauxAnisotropic__
#define __FauxAnisotropic__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "FauxAnisotropicDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "IShComponent.h"

struct FauxAnisotropicPublicData
{
	real32 fRoughnessX;
	real32 fRoughnessY;
	TMCCountedPtr<IShParameterComponent> param;	
};

//MCDEFINE_GUID2(CLSID_FauxAnisotropic, R_CLSID_FauxAnisotropic);
extern const MCGUID CLSID_FauxAnisotropic;


class FauxAnisotropic : public TBasicShader 
{
	public:
		FauxAnisotropic();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(FauxAnisotropicPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
//		virtual int16 			MCCOMAPI	GetResID();
		virtual MCCOMErr		MCCOMAPI	DoShade							(ShadingOut& result,ShadingIn& shadingIn);
#if VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#else
		virtual void			CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#endif
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };

	protected:
		FauxAnisotropicPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
};

#endif
