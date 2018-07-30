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


#ifndef __MultiPassControl__
#define __MultiPassControl__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MultiPassControlDef.h"
#include "BasicShader.h"
//Alan
#include "ShaderBase.h"
#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"


// Define the MultiPassControlShader CLSID ( see the MultiPassControlDef.h file to get R_CLSID_MultiPassControl value )

extern const MCGUID CLSID_MultiPassControl;

struct MultiPassControlShaderPublicData
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
	int32		Level;				// 
	TMCCountedPtr<IShParameterComponent> fInputShader;		
};


//Alan class MultiPassControl : public TBasicShader
class MultiPassControl : public ShaderBase
{
public :

	MultiPassControlShaderPublicData	MultiPassControlData;

	MultiPassControl();

	STANDARD_RELEASE;

//	We don't have to take care of the non implemented methods because the TBasicShader
//	do it for us

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(MultiPassControlShaderPublicData);}

#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x030000)
	virtual real			MCCOMAPI	GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
#endif

#if (VERSIONNUMBER >= 0x040000)
	virtual boolean		MCCOMAPI    WantsTransform	();  
#endif

	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();

protected:

	TMCCountedPtr<I3DShShader> fInputShader;

	enum
	{
		kOne	= IDTYPE('L', 'O', 'N', 'E'), //IDs from .rsr
		kTwo	= IDTYPE('L', 'T', 'W', 'O'),
		kThree	= IDTYPE('L', 'T', 'H', 'R'),
		kFour	= IDTYPE('L', 'F', 'O', 'R'),
		kFive	= IDTYPE('L', 'F', 'I', 'V'),
		kSix	= IDTYPE('L', 'S', 'I', 'X'),
		kSeven	= IDTYPE('L', 'S', 'V', 'N'),
		kEight	= IDTYPE('L', 'E', 'G', 'T'),
		kNine	= IDTYPE('L', 'N', 'I', 'N'),
		kTen	= IDTYPE('L', 'T', 'E', 'N'),
		kRef	= IDTYPE('L', 'R', 'E', 'F')
	};

private :
	uint32	Imp_Out;
#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB Color;
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
	TMCColorRGBA hold;
#endif
};
                           
#endif // __MultiPassControl__
