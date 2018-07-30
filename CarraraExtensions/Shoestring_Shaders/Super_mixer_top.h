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


#ifndef __Super_mixer_top__
#define __Super_mixer_top__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Super_mixer_topDef.h"
#include "BasicShader.h"
//Alan
#include "Shaderbase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "MCColorRGBA.h"
#include "copyright.h"


// Define the Super_mixer_top CLSID ( see the Super_mixer_topDef.h file to get R_CLSID_Super_mixer_top value )

extern const MCGUID CLSID_Super_mixer_top;

struct Super_mixer_topShaderPublicData
{
	int32	BlendMode;
	TVector2 vec2Influence[10];		// grayscale value where shader kicks in
	boolean	Smooth;
	boolean	Linear;
	boolean	Fill;
	int32	PercentBlend;
	int32	Initialize;
	int32	Intensity;
	int32	Luma_value;
	TMCColorRGBA Color;
	boolean	Show_Mix;
	TMCCountedPtr<IShParameterComponent> fShader[12];		// these are the shaders below us 
};


//Alan class Super_mixer_top : public TBasicShader
class Super_mixer_top : public ShaderBase
{
public :

	Super_mixer_topShaderPublicData Data;

	Super_mixer_top();

	STANDARD_RELEASE;
  
// We don't have to take care of the none implemented methods because the TBasicShader
// does this for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(Super_mixer_topShaderPublicData);}

#if (VERSIONNUMBER >= 0x040000)
	virtual boolean		MCCOMAPI    WantsTransform	();  
#endif

	virtual MCCOMErr	MCCOMAPI DoShade  (ShadingOut&  result, ShadingIn &  shadingIn); 
 
protected:

	TMCCountedPtr<I3DShShader> fShaders[12];

	enum
{
	kSmooth_mode	= IDTYPE('S', 'M', 'M', 'X'), //IDs from .rsr
	kShader_mode	= IDTYPE('S', 'H', 'M', 'X')
};
	enum
{
	kInit_Auto	= IDTYPE('A', 'U', 'D', 'I'), //IDs from .rsr
	kManual	= IDTYPE('M', 'A', 'D', 'I')
};

private:

#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif
		real32			luma;		// brightness number for shader w/out color return
		int32 			quant;		// count how many shaders influence a particular color
		uint32			Imp_Out[12];	// keep track of what the implemented outputs are
		real32			last_luma;	// last successful (<100) luma value
		real32			mix;
		real32			last_mix;
		real32			weight;
		real32			per_along_curve;
		boolean			HaveAShader;
};
#endif                         
 // __Super_mixer_top__