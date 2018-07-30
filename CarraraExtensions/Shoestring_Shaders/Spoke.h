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


#ifndef __Spoke__
#define __Spoke__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "SpokeDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "IShComponent.h"
#include "MCPtrArray.h"
#include "MCColorRGBA.h"
#include "copyright.h"

#include "MCCountedPtrHelper.h"

#define	NEG_ONE_PI	 -0.31830988618379067153803535746773
#define SQRT_2 1.4142135623730950488016887242097

// Define the SpokeShader CLSID ( see the SpokeDef.h file to get R_CLSID_Spoke value )

extern const MCGUID CLSID_Spoke;

struct SpokeShaderPublicData
{
	real32		Ulocation;
	real32		Vlocation;
	real32		Height;
	real32		Width;
	int32		Number;
	int32		Offset;
	real32		Taper;
	real32		Circ_Falloff;
	real32		Radial_Start;
	real32		Radial_End;
	real32		Start_Radial_Falloff;
	real32		End_Radial_Falloff;
	boolean		ConstantWidth;
	boolean		FillCenter;
};

//Alan class Spoke : public TBasicShader
class Spoke : public ShaderBase
{
public :

	SpokeShaderPublicData	SpokeData;

	Spoke();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(SpokeShaderPublicData);}
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

	real r;
	real32 segment_size;
	real32 seg_root ;									
	real32  bot;
	real32 top;	
	real32 percent_value;
	real32 falloff_point;
	real32 shift_x;
	real32 shift_y;
	real32 angle;
	real32 width;
	real32  range_check;
	boolean in_the_angle;


};                       
#endif // __Spoke__
