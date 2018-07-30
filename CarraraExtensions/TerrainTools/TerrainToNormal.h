/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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
#ifndef __TerrainToNormal__
#define __TerrainToNormal__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif     

#include "TerrainToNormalDef.h"
#include "BasicShader.h" 
#include "IShComponent.h"
#include "MCCountedPtr.h"
#include "copyright.h"

struct TerrainToNormalPublicData{
	TMCCountedPtr<IShParameterComponent> param;	
	};

//MCDEFINE_GUID2(CLSID_TerrainToNormal, R_CLSID_TerrainToNormal);
extern const MCGUID CLSID_TerrainToNormal;
 

class TerrainToNormal : public TBasicShader {
	public:
		TerrainToNormal();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(TerrainToNormalPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual real			MCCOMAPI GetValue							(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real			MCCOMAPI GetColor							(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI WantsTransform						() { return false; };
		virtual MCCOMErr		MCCOMAPI	DoShade							(ShadingOut& result,ShadingIn& shadingIn);

	protected:
		TerrainToNormalPublicData fData;
		TMCCountedPtr<I3DExShader> shader;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
	};

#endif