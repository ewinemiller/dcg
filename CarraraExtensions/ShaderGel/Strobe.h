/*  Shader Gel and Strobe - plug-in for Carrara
    Copyright (C) 2003 Michael Clarke

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


#ifndef __Strobe__
#define __Strobe__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "StrobeDef.h"
#include "BasicShader.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"
#include "genrand.h"

// Number of strobe steps

#define NUM_STEPS 12
#define NUM_SHADERS 3

struct StrobeData {
	// This is it's fData block...
	
	int32		steps;
	int32		seed;
				
	real32		rotation;
	real32		cycle_length;
	real32		cycle_phase;

	boolean		fromCognito;
	boolean		invertCog;

	int32		step[NUM_STEPS];
	real32		strength1[NUM_STEPS];		
	real32		strength2[NUM_STEPS];		
	real32		strength3[NUM_STEPS];
	int32		effect[NUM_STEPS];
					
	TMCString255	strobe;
				
	TMCCountedPtr<IShParameterComponent> fShader[NUM_SHADERS];

};



// Different strobing effects

#define EFFECT_FADE	0
#define EFFECT_HOLD	1
#define EFFECT_STROBE	2
#define EFFECT_FLICKER	3
#define EFFECT_SPECKLE	4

// Define the Strobe CLSID 

extern const MCGUID CLSID_MCStrobe;
extern const MCGUID CLSID_Strobe;

class Strobe : public TBasicShader {
	
public :

	Strobe();
	~Strobe();

	STANDARD_RELEASE;
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(StrobeData); }
  
	// We don't have to take care of the none implemented methods because the TBasicShader
	// do this for us

	virtual void*		MCCOMAPI    GetExtensionDataBuffer();
	virtual MCCOMErr	MCCOMAPI    ExtensionDataChanged();
	virtual boolean		MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
	virtual MCCOMErr	MCCOMAPI    GetShadingFlags(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI    GetImplementedOutput();
	virtual boolean 	MCCOMAPI    AssignsChannels();

#if (VERSIONNUMBER >= 0x040000)
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
	{
		TMCColorRGB innerResult;
		real returnValue = GetColor(innerResult, fullArea, shadingIn);
		result = innerResult;
		return returnValue;
	};
#endif

#if (VERSIONNUMBER >= 0x030000)
	virtual real		MCCOMAPI GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn);
#else
	virtual MCCOMErr	MCCOMAPI GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn);
#endif

protected:

	// Private variables
	
	
	TMCCountedPtr<I3DShShader> fShaders[NUM_SHADERS];

	float	last_time;

	bool	strobe_on;
	bool	flicker_on;
	
	mtRand *myRand;

	StrobeData fData;

};
                           
class MCStrobe : public TBasicShader {
	
public :

	MCStrobe();
	~MCStrobe();

	STANDARD_RELEASE;
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(StrobeData); }
  
	// We don't have to take care of the none implemented methods because the TBasicShader
	// do this for us

	virtual void*		MCCOMAPI    GetExtensionDataBuffer();
	virtual MCCOMErr	MCCOMAPI    ExtensionDataChanged();
	virtual boolean		MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
	virtual MCCOMErr	MCCOMAPI    GetShadingFlags(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI    GetImplementedOutput();
	virtual boolean 	MCCOMAPI    AssignsChannels();

	virtual MCCOMErr	MCCOMAPI    DoShade(ShadingOut &result, ShadingIn &ShIn);

protected:

	// Private variables
	
#define NUM_SHADERS 3
	
	TMCCountedPtr<I3DShShader> fShaders[NUM_SHADERS];

	float	last_time;

	bool	strobe_on;
	bool	flicker_on;
	
	mtRand *myRand;

	// This is it's fData block...
	
	StrobeData fData;

};
                           
#endif 

