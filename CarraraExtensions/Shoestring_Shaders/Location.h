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

#ifndef __Location__
#define __Location__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "LocationDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "IShComponent.h"
#include "MCPtrArray.h"
#include "MCColorRGBA.h"
#include "copyright.h"

// Define the LocationShader CLSID ( see the LocationDef.h file to get R_CLSID_Location value )

extern const MCGUID CLSID_Location;

struct AxisInfo
{
	real32		Bound1;
	real32		Bound2;
	boolean		Axis;
	int32		Direction;
};

struct LocationShaderPublicData
{
	AxisInfo	Axes[3];
	int32		Mode;
	int32		Coord;
	boolean		Use_shader;
	int32		Shader_factor;
	TMCCountedPtr<IShParameterComponent> fMixShade;		
};

//Alan class Location : public TBasicShader
class Location : public ShaderBase
{
public :

	LocationShaderPublicData	LocationData;

	Location();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(LocationShaderPublicData);}
#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x030000)
	virtual real			MCCOMAPI	GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#endif

protected:

	TMCCountedPtr<I3DShShader> fMixShader;
	enum
	{
		kGreater = IDTYPE('G', 'R', 'T', 'R'),
		kLesser = IDTYPE('L', 'E', 'S', 'R'),
		kOr_mode = IDTYPE('O', 'R', 'M', 'D'),
		kAnd_mode = IDTYPE('A', 'N', 'D', 'M'),
		kLocal = IDTYPE('L','O','C','L'),
		kGlobal = IDTYPE('G','L','B','L')
	};

private:

	real32 DoAxis(real32 Start, real32 End, real32 coord, int32 Greater, real32 rand);
	AxisInfo Axes[3];
	uint32 Imp_Out;
	real32 luma;
	real32 last_luma;
};                       
#endif // __Location__
