/*  Anything Grows - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#ifndef __Billboard__
#define __Billboard__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BillboardDef.h"
#include "BasicPrimitive.h"

// Define the Billboard CLSID ( see the BillboardDef.h file to get R_CLSID_Billboard value )

extern const MCGUID CLSID_Billboard;


// Billboard Object definition : 
// Billboard Object :
struct BillboardData {
	real32	fWidth, fHeight;
	int32 lPanels;
	real32 fZStart;
	}; 

class Billboard : public TBasicPrimitive
{
	public :  
		Billboard();
		
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(BillboardData); }

		virtual void* MCCOMAPI GetExtensionDataBuffer()
		{
			return &fData;
		};

		virtual int16	 MCCOMAPI GetResID()
		{
			return 311;
		};

		// Geometric Calls
		virtual void	 MCCOMAPI GetBoundingBox	(TBBox3D& bbox);
		virtual MCCOMErr MCCOMAPI GetNbrLOD			(int16& nbrLod);
		virtual MCCOMErr MCCOMAPI GetFacetMesh		(uint32 lodIndex, FacetMesh** outMesh);	// lod = level of detail. See theory text in .cpp
		virtual boolean	 MCCOMAPI AutoSwitchToModeler() const
		{	
			return true;
		};


		// Shading Calls
		virtual uint32	 MCCOMAPI GetUVSpaceCount	();
		virtual MCCOMErr MCCOMAPI GetUVSpace		(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

private:
		BillboardData	fData;         // Deformer Data

};

#endif // __Billboard__