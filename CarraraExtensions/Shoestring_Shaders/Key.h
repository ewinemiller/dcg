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

#ifndef __Key__
#define __Key__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "KeyDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"


// Define the KeyShader CLSID ( see the KeyDef.h file to get R_CLSID_Key value )

extern const MCGUID CLSID_Key;

struct KeyShaderPublicData
{
	TVector2	Red_range;
	TVector2	Grn_range;
	TVector2	Blu_range;
	int32		Mode;				// Key or Alpha
	TMCColorRGBA KeyColor;
	TMCCountedPtr<IShParameterComponent> fInputShader;		
};


//Alanclass Key : public TBasicShader
class Key : public ShaderBase
{
public :

	KeyShaderPublicData	KeyData;

	Key();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(KeyData);}

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
		kAlpha_mode	= IDTYPE('A', 'L', 'P', 'H'), //IDs from .rsr
		kKey_mode	= IDTYPE('K', 'E', 'Y', 'M')
	};

private :
	uint32	Imp_Out;
	TVector2 red_range;
	TVector2 green_range;
	TVector2 blue_range;
	
#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif
};
                           
#endif // __Key__
