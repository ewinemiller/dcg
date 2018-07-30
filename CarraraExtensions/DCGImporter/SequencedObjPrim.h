/*  DCG Importer - plug-in for Carrara
    Copyright (C) 2012 Eric Winemiller

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
#ifndef __SequencedObjPrim__
#define __SequencedObjPrim__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "SequencedObjPrimDef.h"
#include "BasicPrimitive.h"
#include "copyright.h"

#include "I3DExImportExport.h"
#include "I3DShObject.H"
#include "IShTokenStream.h"
#include <string.h>

extern const MCGUID CLSID_SequencedObjPrim;
extern const MCGUID IID_SequencedObjPrim;

enum PreviewMode {pmBoundingBox = 1, pmCurrentFrame = 3};
enum LastCaller {lcUnknown = 0, lcGetFacetMesh = 1, lcGetRenderingFacetMesh};
enum ForceLoad {flUnknown = 0, flBoundingBox = 1, flFullMesh = 2};

struct SequencedObjPrimData {
	real32 animate;
	ActionNumber previewMode;
	boolean mapObjectYToCarraraZ;
	real32 conversion;
	real32 smoothing;
	int32 objectType;
	boolean uvBetween0And1;

	boolean  operator==	(const SequencedObjPrimData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(SequencedObjPrimData)) == 0;
	}

	boolean  operator!=	(const SequencedObjPrimData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(SequencedObjPrimData)) != 0;
	}

}; 



class SequencedObjPrim : public TBasicPrimitive, public IExStreamIO
	
{
public :  
	SequencedObjPrim();
	~SequencedObjPrim();
	
	STANDARD_RELEASE;
	virtual MCCOMErr 	MCCOMAPI QueryInterface			(const MCIID &riid, void** ppvObj);
 	virtual uint32 		MCCOMAPI AddRef 				();

	virtual int32   MCCOMAPI GetParamsBufferSize    ()
		const        { return sizeof(SequencedObjPrimData); }

	//data exchanger calls
	virtual void*	 MCCOMAPI GetExtensionDataBuffer()
	{
		return &fData;
	};
	virtual MCCOMErr MCCOMAPI ExtensionDataChanged();

	virtual void	MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);

	// Geometric Calls
	virtual void	 MCCOMAPI GetBoundingBox			(TBBox3D& bbox);
	virtual MCCOMErr MCCOMAPI GetFacetMesh		(uint32 lodIndex, FacetMesh** outMesh);	// lod = level of detail. See theory text in .cpp
	virtual boolean	 MCCOMAPI HasRenderingFacetMesh() { return true;};
	virtual MCCOMErr MCCOMAPI GetRenderingFacetMesh(FacetMesh** outMesh);
	virtual MCCOMErr MCCOMAPI GetNbrLOD			(int16& nbrLod);
	virtual MCCOMErr MCCOMAPI GetLOD			(int16 lodIndex, real& lod);
	virtual boolean	 MCCOMAPI AutoSwitchToModeler() const
	{return true;};

	virtual MCCOMErr MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr MCCOMAPI Write(IShTokenStream* stream);
	virtual MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext) {return MC_S_OK;};

	// Shading Calls
	virtual uint32	 MCCOMAPI GetUVSpaceCount			();
	virtual MCCOMErr MCCOMAPI GetUVSpace				(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);

	SequencedObjPrimData	fData;
	SequencedObjPrimData	lastfData;
	
	TMCClassArray<UVSpaceInfo> spaceInfo;
	TMCClassArray<TMCDynamicString> foundFiles;
	TMCClassArray<TBBox3D> boundingBoxes;

	int32 currentFrame;
	int32 lastPreviewFrame;
	TMCCountedPtr<FacetMesh> mesh;

private :
	TMCCountedPtr<I3DExImportFilter> objImporter;
	
	void loadIfNeeded(ForceLoad forceLoad);
	void loadOBJFile();
	void loadBoundingBox();
	void buildCubeMesh(FacetMesh** outMesh, const TBBox3D& boundingBox);
	void buildPlaceHolderMesh(FacetMesh** outMesh);
	uint32 getCalculatedFrame() {
		return floor(fData.animate * static_cast<real32>(foundFiles.GetElemCount() - 1));
	}
	MCCOMErr getFile(TMCDynamicString& fileName, IMCFile** file);
	void loadDataFromStub();

	LastCaller lastCaller;
	boolean inImport;
	boolean stubDataLoaded;
	TMCCountedPtr<I3DShScene> sceneStub;

};

#endif // __SequencedObjPrim__