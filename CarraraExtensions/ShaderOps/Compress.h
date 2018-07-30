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
#ifndef __COMPRESS__
#define __COMPRESS__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif     

#include "CompressDef.h"
#include "BasicShader.h"
#include "IShComponent.h"
#include "MCCountedPtr.h"
#include "copyright.h"

struct CompressPublicData{
	TVector2 vec2Compress;
	TMCCountedPtr<IShParameterComponent> param;	
	};

//MCDEFINE_GUID2(CLSID_Compress, R_CLSID_Compress);
extern const MCGUID CLSID_Compress;


class Compress : public TBasicShader {
	public:
		Compress();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(CompressPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
#if (VERSIONNUMBER >= 0x030000)
		virtual real			MCCOMAPI GetValue							(real& result,boolean& fullArea,ShadingIn& shadingIn);
#if (VERSIONNUMBER >= 0x040000)
		virtual real			MCCOMAPI GetColor							(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
#else
		virtual real			MCCOMAPI GetColor							(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn);
#endif 
#else
		virtual MCCOMErr		MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual MCCOMErr		MCCOMAPI	GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn);
#endif
#if (VERSIONNUMBER >= 0x040000)
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };
#endif
		virtual int16 			MCCOMAPI	GetResID();

	protected:
		CompressPublicData fData;
		TMCCountedPtr<I3DExShader> shader;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		real32 fRange;
		void SetDefaults();
	};

#endif