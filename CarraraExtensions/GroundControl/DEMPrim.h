/*  Ground Control - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#ifndef __DEMPrim__
#define __DEMPrim__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "DEMPrimDef.h"
#include "BasicPrimitive.h"
#include "IShTokenStream.h"
#include "DEMPrimData.h"
#include "ElevationMesh.h"
#include "Dem.h"
#include "copyright.h"

// Define the DEMPrim CLSID ( see the DEMPrimDef.h file to get R_CLSID_DEMPrim value )

extern const MCGUID CLSID_DEMPrim;
extern const MCGUID IID_DEMPrim;

#define SPLIT_DUMB 0
#define SPLIT_SHORT 1
#define SPLIT_SMART 2

#define PREVIEW_16 0
#define PREVIEW_32 1
#define PREVIEW_64 2
#define PREVIEW_128 3
#define PREVIEW_FULL 4

class DEMPrim : public TBasicPrimitive, public IExStreamIO, public ElevationMesh
	
{
public :  
	DEMPrim();
	~DEMPrim();
	
	virtual MCCOMErr 	MCCOMAPI QueryInterface			(const MCIID &riid, void** ppvObj);
 	virtual uint32 		MCCOMAPI AddRef 				();
	STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(DEMPrimData); }

	//data exchanger calls
	virtual void*	 MCCOMAPI GetExtensionDataBuffer()
	{
		return &fData;
	};
	virtual int16		MCCOMAPI GetResID()
	{
		return 131;
	};
	virtual void	MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr MCCOMAPI ExtensionDataChanged();
	virtual MCCOMErr 			MCCOMAPI HandleEvent			(MessageID message, IMFResponder* source, void* data);


	// Geometric Calls
	virtual void	 MCCOMAPI GetBoundingBox			(TBBox3D& bbox);
	virtual MCCOMErr MCCOMAPI GetFacetMesh		(uint32 lodIndex, FacetMesh** outMesh);	// lod = level of detail. See theory text in .cpp
#if (VERSIONNUMBER >= 0x050000)
	virtual boolean	   MCCOMAPI HasRenderingFacetMesh()
	{ return true;};
	virtual MCCOMErr   MCCOMAPI GetRenderingFacetMesh(FacetMesh** outMesh);
#else
	virtual MCCOMErr MCCOMAPI GetNbrLOD			(int16& nbrLod)
	{
		nbrLod = 2;
		return MC_S_OK;
	};

	virtual MCCOMErr MCCOMAPI GetLOD			(int16 lodIndex, real& lod)
	{
		switch(lodIndex) {
			case 0: 
				lod = 0;
				break;
			case 1:
				lod = .000001f;
				break;
			}
		return MC_S_OK;
	};
#endif


	// Shading Calls
	virtual uint32	 MCCOMAPI GetUVSpaceCount			();
	virtual MCCOMErr MCCOMAPI GetUVSpace				(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

	// IExStreamIO calls
	virtual MCCOMErr			MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr			MCCOMAPI Write(IShTokenStream* stream);
#if (VERSIONNUMBER >= 0x060000)
	virtual MCCOMErr MCCOMAPI FinishRead		(IStreamContext* streamContext) {return MC_S_OK;};
#endif
		
private :
	TMCCountedPtr<IMFPart> warnings;
	DEMPrimData	fData;
	TMCCountedPtr<FacetMesh> lastPreviewMesh;
	real32 lastPreviewFrame;
	void updateFacets();

};

#endif // __DEMPrim__