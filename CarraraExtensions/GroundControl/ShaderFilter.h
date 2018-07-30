/*  Ground Control - plug-in for Carrara
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
#ifndef __ShaderFilter__
#define __ShaderFilter__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ShaderFilterDef.h"
#include "BasicTerrainFilter.h"
#include "dem.h"
#include "cTransformer.h"

struct ShaderFilterPublicData
{
	TMCString255 sShader; 
	ActionNumber lShaderPicker;
	real32 fHeight;
	ActionNumber lOperator;
	int32 lPasses;
	TMCString255 sEffectShader; 
	ActionNumber lEffectShaderPicker;
	boolean bUseExistingHeight;
};

enum Operator {opAdd = 1, opMax = 2, opMin = 3, opMultiply = 4, opReplace = 5, opSubtract = 6, opFilter = 7, opSmooth = 8, opAverage = 9};

extern const MCGUID CLSID_ShaderFilter;


class ShaderFilter : public TBasicTerrainFilter, cTransformer
{
	public:
		ShaderFilter()
		{
			fData.lShaderPicker = -1;
			fData.lEffectShaderPicker = -1;
			fData.fHeight = 20000;
			fData.lOperator = opAdd;
			fData.lPasses = 1;
		};
		~ShaderFilter()
		{
		};
		STANDARD_RELEASE;
		virtual void MCCOMAPI Filter(TMCArray<real>& heightField, TVector2& heightBound,
								 const TIndex2& size, const TVector2& cellSize);
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(ShaderFilterPublicData); }
#if (VERSIONNUMBER >= 0x060000)
		virtual void MCCOMAPI SetDefaultSizes(real &sizex, real &sizey) {};
#endif
		virtual boolean MCCOMAPI CanBuildPreview()
			{return true;};
		virtual void*		MCCOMAPI GetExtensionDataBuffer(){return &fData;};
		virtual MCCOMErr	MCCOMAPI ExtensionDataChanged();
		virtual MCCOMErr	MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  

	protected:
		ShaderFilterPublicData fData;
		TMCCountedPtr<I3DShScene> scene;


};

#endif