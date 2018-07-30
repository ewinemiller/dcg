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


#ifndef __MultiPassManager__
#define __MultiPassManager__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MultiPassManagerDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "MCColorRGBA.h"
#include "copyright.h"


// Define the MultiPassManager CLSID ( see the MultiPassManagerDef.h file to get R_CLSID_MultiPassManager value )

extern const MCGUID CLSID_MultiPassManager;

struct MultiPassManagerShaderPublicData
{
	TMCString255 label1;
	TMCString255 label2;
	TMCString255 label3;
	TMCString255 label4;
	TMCString255 label5;
	TMCString255 label6;
	TMCString255 label7;
	TMCString255 label8;
	TMCString255 label9;
	TMCString255 label10;
	TMCCountedPtr<IShParameterComponent> fShader[11];		// these are the shaders below us 
};

//Alan class MultiPassManager : public TBasicShader
class MultiPassManager : public ShaderBase
{
public :

	MultiPassManagerShaderPublicData Data;

	MultiPassManager();

	STANDARD_RELEASE;
  
// We don't have to take care of the none implemented methods because the TBasicShader
// does this for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(MultiPassManagerShaderPublicData);}

#if (VERSIONNUMBER >= 0x040000)
	virtual boolean		MCCOMAPI    WantsTransform	();  
#endif

	virtual MCCOMErr	MCCOMAPI DoShade  (ShadingOut&  result, ShadingIn &  shadingIn); 
 
protected:

	TMCCountedPtr<I3DShShader> fShaders[11];

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
		uint32			Imp_Out[11];	// keep track of what the implemented outputs are
		real32			last_luma;	// last successful (<100) luma value
		real32			mix;
		boolean			HaveAShader;
};
#endif                         
 // __MultiPassManager__