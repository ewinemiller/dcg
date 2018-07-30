/*  Shoestring Shaders - plug-in for Carrara
    Copyright (C) 2003  Mark DesMarais

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

#ifndef __Drip__
#define __Drip__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "DripDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "IShComponent.h"
#include "MCPtrArray.h"
#include "MCColorRGBA.h"
#include "copyright.h"

#define	NEG_ONE_PI	 -0.31830988618379067153803535746773

// Define the DripShader CLSID ( see the DripDef.h file to get R_CLSID_Drip value )

extern const MCGUID CLSID_Drip;

struct DripShaderPublicData
{
	real32		Ulocation;
	real32		Vlocation;
	real32		Height;
	real32		Width;
	real32		Offset;
	real32		Completion;
	int32		Inter_ripple_freq;
	boolean		Use_shaping;
	boolean		AutoWidth;
	boolean		Rebound;
	real32		Rebound_Size;
	real32		Rebound_Lag;
	boolean		Show_Center;
};

//Alan class Drip : public TBasicShader
class Drip : public ShaderBase
{
public :

	DripShaderPublicData	DripData;

	Drip();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(DripShaderPublicData);}
#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x030000)
	virtual real			MCCOMAPI	GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
	#if (VERSIONNUMBER >= 0x040000)
		virtual boolean		MCCOMAPI    WantsTransform	();
	#endif

#endif

private:
	struct Ring_return {
		real	value;
		real	slope;
	};
	Ring_return Calculate_ring(real dd_completion, real dd_width, real dd_height);
	real r;
	TVector3		VectorRotate(TVector3 shaft, TVector3 vector, real32 asin_angle);
};                       
#endif // __Drip__
