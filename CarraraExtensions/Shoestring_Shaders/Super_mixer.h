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


#ifndef __SUPER_MIXER__
#define __SUPER_MIXER__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Super_mixerDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "MCColorRGBA.h"
#include "copyright.h"


// Define the Super_mixer CLSID ( see the Super_mixerDef.h file to get R_CLSID_Super_mixer value )

extern const MCGUID CLSID_Super_mixer;

struct Super_mixerShaderPublicData
{
	int32	BlendMode;
	int32	MixMode;				// pick add or mix by value
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


//class Super_mixer : public TBasicShader
class Super_mixer : public ShaderBase
{
public :

	Super_mixerShaderPublicData Data;

	Super_mixer();

	STANDARD_RELEASE;
  
// We don't have to take care of the none implemented methods because the TBasicShader
// does this for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(Super_mixerShaderPublicData);}
#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetColor			(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetColor			(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER == 0x030000)
	virtual real 		MCCOMAPI    GetColor			(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);

#elif (VERSIONNUMBER >= 0x040000)
	virtual real 		MCCOMAPI    GetColor			(TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual boolean		MCCOMAPI    WantsTransform	();  
#endif

protected:

	TMCCountedPtr<I3DShShader> fShaders[12];

	enum
{
	kMix_mode	= IDTYPE('M', 'X', 'M', 'D'), //IDs from .rsr
	kAdd_mode	= IDTYPE('A', 'D', 'M', 'D')
};

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
};
                           
#endif // __Super_mixer__
