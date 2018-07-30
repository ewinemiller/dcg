/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#ifndef __MultiMax__
#define __MultiMax__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif     

#include "MultiMaxDef.h"
#include "IShComponent.h"
#include "copyright.h"
#include "I3DShShader.h"
#include "MCCountedPtrArray.h"
#include "MultiOpHelper.h"

extern const MCGUID CLSID_MultiMax;
 
class MultiMax : public MultiOpHelper<MultiOpPublicData>
{
	public:
		MultiMax() {};
		~MultiMax() {};
		STANDARD_RELEASE;


		virtual real MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real MCCOMAPI GetVector(TVector3& result,ShadingIn& shadingIn);
	};

#endif