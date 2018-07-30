/*  Project Gemini - plug-in for Carrara
    Copyright (C) 2002 Eric Winemiller

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
#ifndef __GeminiDeformer__
#define __GeminiDeformer__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicModifiers.h"   
#include "GeminiDeformerDef.h"

//****** Globally Unique Id *************************************************
extern const MCGUID CLSID_GeminiDeformer;
extern const MCGUID IID_GeminiDeformer;

struct GeminiData {
	boolean bEnabled;
	int32 lKeep;
	real32 fMargin;
	};


// Deformer Object :
class GeminiDeformer : public TBasicDeformModifier
{
	private:
		void CopyData(GeminiDeformer* dest) const
		{
			dest->fData = fData;
		};

		GeminiData fData;

	public :  

		GeminiDeformer();
		~GeminiDeformer()
		{
		}
		// IUnknown Interface :
		STANDARD_RELEASE;
		virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
		
  		// I3DExDataExchanger methods :
		virtual void*		MCCOMAPI GetExtensionDataBuffer()
		{
			return &fData;	
		};

		virtual int16		MCCOMAPI GetResID()
		{
			return 308;
		};

		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(GeminiData); }
		virtual void		MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
		{
			TMCCountedCreateHelper<IExDataExchanger>	result(res);

			GeminiDeformer* theClone = new GeminiDeformer();
			ThrowIfNil(theClone);
			theClone->SetControllingUnknown(pUnkOuter);
			CopyData(theClone);

			result = (IExDataExchanger*) theClone;
		};

		// I3DExDeformer methods :
		virtual void			MCCOMAPI SetBoundingBox		(const TBBox3D& bbox)
		{
		};

		virtual MCCOMErr		MCCOMAPI DeformPoint		(const TVector3& point, TVector3& result)
		{
			return MC_S_OK;
		};

		virtual MCCOMErr		MCCOMAPI DeformFacetMesh	(real lod,FacetMesh* in, FacetMesh** outMesh);

		virtual MCCOMErr		MCCOMAPI DeformBBox			(const TBBox3D& in, TBBox3D& out);
		
		virtual TModifierFlags	MCCOMAPI GetModifierFlags	()	
		{
			return fFlags; 
		};
};

#endif