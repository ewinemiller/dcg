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
#ifndef __Eraser__
#define __Eraser__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "PublicUtilities.h"
#include "BasicModifiers.h"   
#include "EraserDef.h"

//****** Globally Unique Id *************************************************
extern const MCGUID CLSID_Eraser;

struct EraserData {
	boolean bEnabled;
	int32 lStartFrom;
	real32 fErase;
	};

// Deformer Object :
class Eraser : public TBasicDeformModifier
{
	private:
		void CopyData(Eraser* dest) const
		{
			dest->fData = fData;
		};

		EraserData fData;
		void DoPositiveDirection(const uint& lIndex,const TBBox3D& range, FacetMeshAccumulator& accu, FacetMeshFacetIterator& iter);
		void DoNegativeDirection(const uint& lIndex,const TBBox3D& range, FacetMeshAccumulator& accu, FacetMeshFacetIterator& iter);

	public :  
		Eraser();

		~Eraser()
		{
		}
		// IUnknown Interface :
		STANDARD_RELEASE;
		
  		// I3DExDataExchanger methods :
		virtual void*		MCCOMAPI GetExtensionDataBuffer()
		{
			return &fData;	
		};

		virtual int16		MCCOMAPI GetResID()
		{
			return 309;
		};

		virtual void		MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
		{
			TMCCountedCreateHelper<IExDataExchanger>	result(res);

			Eraser* theClone = new Eraser();
			ThrowIfNil(theClone);
			theClone->SetControllingUnknown(pUnkOuter);
			CopyData(theClone);

			result = (IExDataExchanger*) theClone;
		};
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(EraserData); }

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