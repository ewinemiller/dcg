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


#ifndef __Translucent__
#define __Translucent__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "TranslucentDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"

#include "MCCountedPtrHelper.h"
#include "COMUtilities.h"
#include "BasicModifiers.h"
#include "I3DShRenderFeature.h"
#include "I3DShRenderable.h"
#include "I3DShTreeElement.h"
#include "I3DShScene.h"
#include "copyright.h"

// Define the TranslucentShader CLSID ( see the TranslucentDef.h file to get R_CLSID_Translucent value )

extern const MCGUID CLSID_Translucent;

struct TranslucentShaderPublicData
{
	TMCString255 cameraname;
	TMCString255 lightname;
	boolean		inv_gray;
	int32		Mode;				// smooth mix or shader influenced
	TVector2	Angle;
	int32		Point_mode;
	TVector2	Ease;
};


//Alan class Translucent : public TBasicShader
class Translucent : public ShaderBase
{
public :

	TranslucentShaderPublicData	TranslucentPublicData;

	Translucent();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(TranslucentShaderPublicData);}

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

protected:

	enum
	{
		kBezier_mode	= IDTYPE('E', 'X', 'P', 'O'), //IDs from .rsr
		kLinear_mode	= IDTYPE('L', 'I', 'N', 'E')
	};

	enum
	{
		kSurfPoint = IDTYPE('O', 'p', 't', '1'),
		kHotPoint = IDTYPE('O', 'p', 't', '3')
	};

private :
	real32	cp[4];
	real32   ax, bx, cx;
    real32   ay, by, cy;
	real32 angle;
	real32 last_luma;
#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif
	TVector3 find_point;
};
                           
#endif // __Translucent__
