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

#ifndef __Gray__
#define __Gray__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GrayDef.h"
//Alan
#include "ShaderBase.h" 
#include "BasicShader.h"
#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"


// Define the GrayShader CLSID ( see the GrayDef.h file to get R_CLSID_Gray value )

extern const MCGUID CLSID_Gray;

struct GrayShaderPublicData
{
	real32		Gray_level;
	real32		Desat_Red;
	real32		Desat_Grn;
	real32		Desat_Blu;
	int32		Mode;				// grayscale or de-saturate
	TMCCountedPtr<IShParameterComponent> fInputShader;	
	boolean		Do_Color;
	boolean		Do_Glow;
	boolean		Do_Spec;
	boolean		Do_Trans;
};


//Alanclass Gray : public TBasicShader
class Gray : public ShaderBase
{
public :

	GrayShaderPublicData	GrayData;

	Gray();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(GrayData);}
	virtual MCCOMErr	MCCOMAPI DoShade  (ShadingOut&  result, ShadingIn &  shadingIn); 

#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER == 0x030000)
	virtual real 		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER >= 0x040000)
	virtual real 		MCCOMAPI    GetColor		(TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual boolean		MCCOMAPI    WantsTransform	();  

#endif

	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();

protected:

	TMCCountedPtr<I3DShShader> fInputShader;

	enum
	{
		kDesat_mode	= IDTYPE('D', 'E', 'S', 'T'), //IDs from .rsr
		kGray_mode	= IDTYPE('G', 'R', 'S', 'C')
	};

private :

	real32 HueToRGB(real32 n1, real32 n2, real32 hue);
	TMCColorRGBA SatControl(TMCColorRGBA Color);	
	uint32	Imp_Out;
	real32 luma;
	real32 desat_red;
	real32 desat_green;
	real32 desat_blue;
	boolean Colorset;
	boolean Glowset;
	boolean Specularset;
	boolean Transset;
	
#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif
};
                           
#endif // __Gray__
