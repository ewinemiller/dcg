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

#ifndef __Lit__
#define __Lit__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "LitDef.h"
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

// Define the LitShader CLSID ( see the LitDef.h file to get R_CLSID_Lit value )

extern const MCGUID CLSID_Lit;

struct LitShaderPublicData
{
	TMCString255 lightname;
	boolean		inv_gray;
	boolean		color;
	boolean		shine_thru;
	TVector2	Angle;
	int32		Point_mode;
	boolean		New_mode;
};

//class Lit : public TBasicShader
class Lit : public ShaderBase
{
public :

	LitShaderPublicData	LitPublicData;

	Lit();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(LitShaderPublicData);}

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

	virtual real			MCCOMAPI	GetValue		(real& result,boolean& fullArea,ShadingIn& shadingIn);
protected:

	enum
	{
		kSurfPoint = IDTYPE('O', 'p', 't', '1'),
		kHotPoint = IDTYPE('O', 'p', 't', '3')
	};

	enum
	{
		kAmbientLight,
		kDistantLight,
		kPointLight,
		kSpotLight
	};

private :

	real32 angle;
	real32 last_luma;
	TMCColorRGBA Color;
	TVector3 find_point;
};
                           
#endif // __Lit__
