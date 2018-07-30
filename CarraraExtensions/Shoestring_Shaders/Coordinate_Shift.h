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

#ifndef __Coordinate_Shift__
#define __Coordinate_Shift__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Coordinate_ShiftDef.h"
#include "GrayDef.h"
//Alan
#include "ShaderBase.h"
#include "BasicShader.h"
#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3DShGroup.h"
#include "PublicUtilities.h"

#include "MCCountedPtrHelper.h"
#include "COMUtilities.h"
#include "BasicModifiers.h"
#include "I3DShRenderFeature.h"
#include "I3DShRenderable.h"
#include "I3DShTreeElement.h"

// Define the Coordinate_ShiftShader CLSID ( see the Coordinate_ShiftDef.h file to get R_CLSID_Coordinate_Shift value )

extern const MCGUID CLSID_Coordinate_Shift;

struct Coordinate_ShiftShaderPublicData
{
	int32			Mode;
	TMCString255	Objectname;
	real32			Xoffset;
	real32			Yoffset;
	real32			Zoffset;
	boolean			GlobalX;
	boolean			GlobalY;
	boolean			GlobalZ;
	boolean			LocalX;
	boolean			LocalY;
	boolean			LocalZ;
	TMCCountedPtr<IShParameterComponent> fInputShader;		
};


//Alan class Coordinate_Shift : public TBasicShader
class Coordinate_Shift : public ShaderBase
{
public :

	Coordinate_ShiftShaderPublicData	Coordinate_ShiftData;

	Coordinate_Shift();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize		() const {return sizeof(Coordinate_ShiftData);}

	virtual MCCOMErr	MCCOMAPI DoShade  (ShadingOut&  result, ShadingIn &  shadingIn); 

#if (VERSIONNUMBER >= 0x040000)
	virtual boolean		MCCOMAPI    WantsTransform	();  

#endif

	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();

protected:

	TMCCountedPtr<I3DShShader> fInputShader;

	enum
	{
		kOffset_mode	= IDTYPE('O', 'F', 'F', 'M'), //IDs from .rsr
		kObject_mode	= IDTYPE('O', 'B', 'J', 'M')
	};

private :

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DShTreeElement> objecttree;
	uint32	Imp_Out;
	TTransform3D sped;
	TMCString255 sObject_name;

	TVector3 object_loc;
	TVector3 offset_vector;

	boolean fullArea;
	real32 luma;

#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
#endif
};
                           
#endif // __Coordinate_Shift__
