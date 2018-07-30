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


#ifndef __Proximity__
#define __Proximity__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ProximityDef.h"
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

// Define the ProximityShader CLSID ( see the ProximityDef.h file to get R_CLSID_Proximity value )

extern const MCGUID CLSID_Proximity;

struct ProximityShaderPublicData
{
	TMCString255 objectname;
	boolean		Use_shader;
	int32		Shader_factor;
	int32		Point_mode;
	boolean		X_axis;
	boolean		Y_axis;
	boolean		Z_axis;
	real32		Limit;
	real32		DistanceFade;
	real32		Offset;
	boolean		Single;
	TMCCountedPtr<IShParameterComponent> fMixShade;		
};


//Alan class Proximity : public TBasicShader
class Proximity : public ShaderBase
{
public :

	ProximityShaderPublicData	ProximityPublicData;

	Proximity();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(ProximityShaderPublicData);}

#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x030000)
	virtual real 		MCCOMAPI    GetValue		(real& result,boolean& fullArea, ShadingIn& shadingIn);
#endif

protected:

	TMCCountedPtr<I3DShShader> fMixShader;
	enum
	{
		kSurfPoint = IDTYPE('O', 'p', 't', '1'),
		kHotPoint = IDTYPE('O', 'p', 't', '3')
	};

private :
	uint32	Imp_Out;
	real32 luma;
	real32 last_luma;
	TMCString255 sObject_name;
	uint32 length;
	TVector3 find_point;
	real32	FadeLength;
};
                           
#endif // __Proximity__
