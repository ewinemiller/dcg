/*  Shader Ops - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#ifndef __RADIALGRADIENT__
#define __RADIALGRADIENT__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "RadialGradientDef.h"
#include "BasicShader.h"
#include "copyright.h"

struct RadialGradientPublicData{
	TVector2 vec2Range;
	real32 fPosU;
	real32 fPosV;
	};

//MCDEFINE_GUID2(CLSID_RadialGradient, R_CLSID_RadialGradient);
extern const MCGUID CLSID_RadialGradient;


class RadialGradient : public TBasicShader {
	public:
		RadialGradient();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(RadialGradientPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
#if (VERSIONNUMBER >= 0x030000)
		virtual real			MCCOMAPI GetValue							(real& result,boolean& fullArea,ShadingIn& shadingIn);
#else
		virtual MCCOMErr		MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
#endif
#if (VERSIONNUMBER >= 0x040000)
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };
#endif
		virtual int16 			MCCOMAPI	GetResID();

	protected:
		RadialGradientPublicData fData;
	};

#endif