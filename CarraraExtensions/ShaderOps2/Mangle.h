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
#ifndef __Mangle__
#define __Mangle__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MangleDef.h"
#include "BasicShader.h"
#include "copyright.h"

struct ManglePublicData
{

};

extern const MCGUID CLSID_MangleSpecular;
extern const MCGUID CLSID_MangleGlow;
extern const MCGUID CLSID_MangleDiffuse;
extern const MCGUID CLSID_MangleAmbient;
extern const MCGUID CLSID_MangleReflection;
extern const MCGUID CLSID_MangleIllumination;
extern const MCGUID CLSID_MangleColor;
extern const MCGUID CLSID_MangleBackground;
extern const MCGUID CLSID_MangleLightShadow;
extern const MCGUID CLSID_MangleRefraction;
extern const MCGUID CLSID_MangleGlobalIllumination;
extern const MCGUID CLSID_MangleCaustics;
extern const MCGUID CLSID_MangleSubsurfaceScattering;
extern const MCGUID CLSID_MangleAmbientOcclusionFactor;
extern const MCGUID CLSID_MangleAlpha;


enum MangleMode {mmSpecular, mmDiffuse, mmGlow, mmReflection, mmAmbient, mmIllumination, mmColor
	, mmBackground, mmLightShadow, mmRefraction, mmGlobalIllumination, mmCaustics, mmSubsurfaceScattering, mmAmbientOcclusionFactor, mmAlpha};

class Mangle : public TBasicShader {
	public:
		Mangle(MangleMode mangleMode);
		~Mangle();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(ManglePublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual real			MCCOMAPI GetValue							(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real			MCCOMAPI GetColor							(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };

	protected:
		ManglePublicData fData;
		enum MangleMode mangleMode;
	};

#endif