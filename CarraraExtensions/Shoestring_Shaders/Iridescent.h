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

#ifndef __Iridescent__
#define __Iridescent__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "IridescentDef.h"
//Alan
#include "ShaderBase.h"
#include "BasicShader.h"
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

// Define the IridescentShader CLSID ( see the IridescentDef.h file to get R_CLSID_Iridescent value )

extern const MCGUID CLSID_Iridescent;

struct Color_spec
{
	TMCColorRGBA Color;			// color picker
	real32	fCenter;				// Center
	real32	fFalloff;
};


struct IridescentShaderPublicData
{
	TMCString255 objectname;
	Color_spec  Colors[7];	
	boolean		last_is_rest;
	boolean		copy_falloff;
	boolean		grayscale;
	boolean		inv_gray;
	real32		yaw;
	real32		pitch;
	int32		Mode;				// smooth mix or shader influenced
	int32		VecMode;				// use light or reference vector
	TMCCountedPtr<IShParameterComponent> fMixShade;		

};


//Alan class Iridescent : public TBasicShader
class Iridescent : public ShaderBase
{
public :

	IridescentShaderPublicData	IridescentPublicData;

	Iridescent();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(IridescentShaderPublicData);}

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

	TMCCountedPtr<I3DShShader> fMixShader;

	enum
	{
		kSmooth_mode	= IDTYPE('S', 'M', 'M', 'X'), //IDs from .rsr
		kShader_mode	= IDTYPE('S', 'H', 'M', 'X')
	};

	enum
	{
		kUseObject = IDTYPE('O', 'B', 'J', 'T'),
		kUseRefVec = IDTYPE('R', 'F', 'V', 'T')
	};

private :

	real32 Calc_weight (real32 falloff, real32 angle, real32 center);
	TVector3 Matrix_mult(TVector3 vector, real32 rot_matrix[3][3]);
	real32 ref_len;
	real32 angle;
	real32 weight[7];
	real32 sine_z;
	TVector3 ref_vec;
	real32 rot_matrix[3][3];
	real32 cos_rot;
	real32 sin_rot;
	uint32	Imp_Out;
	real32 luma;
	real32 last_luma;
	TMCColorRGBA Color;
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DShTreeElement> objecttree;
};
                           
#endif // __Iridescent__
