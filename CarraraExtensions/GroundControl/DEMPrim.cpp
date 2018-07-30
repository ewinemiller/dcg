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
#include "COM3DUtilities.h"
#include "COMUtilities.h"
#include "I3DShUtilities.h"
#include "IShUtilities.h"
#include "DEMPrim.h"
#include "float.h"
#include "dcgdebug.h"
#include "IMFResponder.h"
#include "IMFPart.h"


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_DEMPrim(R_CLSID_DEMPrim);
const MCGUID IID_DEMPrim(R_IID_DEMPrim);
#else
const MCGUID CLSID_DEMPrim={R_CLSID_DEMPrim};
const MCGUID IID_DEMPrim={R_IID_DEMPrim};
#endif

template <class c>
inline real32 sqr(c pfIn)
{
	return pfIn * pfIn;
}

template<class c>
inline real32 sqr(const c &first, const c &second) 
{
	return sqr(first.x - second.x) + sqr(first.y - second.y) + sqr(first.z - second.z);
}

template<class c>
inline real32 distance(const c &first,const c &second) 
{
	return sqrt(sqr(first, second));
}

inline real32 intcompare(const real32 &in) {
	return floorf(in * 100000 + .5);
}

// Constructor of the C++ Object
DEMPrim::DEMPrim()
{
	fData.lPreviewSize = PREVIEW_64;
	fData.sizeX = 20.0f;
	fData.sizeY = 20.0f;
	fData.topElevation = 2.0f;
	fData.baseElevation = 0;
	fData.adaptiveError = .01f;
	fData.animate = 0;
	fData.interpolateFrames = true;
	fData.meshType = gcGridSmartSplit;
	

	lastPreviewFrame = 0;
}

DEMPrim::~DEMPrim()
{
}

MCCOMErr DEMPrim::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_DEMPrim))
	{
		TMCCountedGetHelper<DEMPrim> result(ppvObj);
		result = (DEMPrim*)this;
		return MC_S_OK;
	}
	else
		return TBasicPrimitive::QueryInterface(riid, ppvObj);	
}

uint32 DEMPrim::AddRef()
{
	return TBasicPrimitive::AddRef();
}


void DEMPrim::Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger>	result(res);

	DEMPrim* theClone = new DEMPrim();
	ThrowIfNil(theClone);

	theClone->SetControllingUnknown(pUnkOuter);
	theClone->fData = fData;
	theClone->fullMapInfo.samples[GC_NORTH] = fullMapInfo.samples[GC_NORTH];
	theClone->fullMapInfo.samples[GC_EAST] = fullMapInfo.samples[GC_EAST];
	theClone->fElevation = fElevation;
	theClone->elevationRange = elevationRange;
	theClone->lastPreviewMesh = lastPreviewMesh;
	theClone->lastPreviewFrame = lastPreviewFrame;
	theClone->completeElevationRange = completeElevationRange;

	result = (IExDataExchanger*) theClone;
}

// I3DExGeometricPrimitive methods
// -- Geometry calls

// Give the boundary Box
void DEMPrim::GetBoundingBox(TBBox3D& bbox) 
{
	setCurrentFrame(fData.animate * static_cast<real32>(fElevation.GetElemCount() - 1), fData.interpolateFrames);

	bbox.fMax.x =fData.sizeX * 0.5;
	bbox.fMin.x=-bbox.fMax.x;
	bbox.fMax.y=fData.sizeY * 0.5;
	bbox.fMin.y=-bbox.fMax.y;
	
	bbox.fMax.z=fData.baseElevation + (fData.topElevation - fData.baseElevation) * getElevationTop();
	bbox.fMin.z=fData.baseElevation + (fData.topElevation - fData.baseElevation) * getElevationBottom();
}

// -- Shading calls
uint32 DEMPrim::GetUVSpaceCount() 
{
	return 1; // the DEMPrim is describe with only 1 UV-Space
}

MCCOMErr DEMPrim::GetUVSpace( uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo) 
{
	if (uvSpaceID == 0)
	{     
		uvSpaceInfo->fWraparound[0] = false; 
		uvSpaceInfo->fWraparound[1] = false;
	}
	return MC_S_OK;
}

MCCOMErr DEMPrim::GetFacetMesh (uint32 lodIndex, FacetMesh** outMesh)
{
	setCurrentFrame(fData.animate * static_cast<real32>(fElevation.GetElemCount() - 1), fData.interpolateFrames);

	try
	{
		if (lastPreviewMesh && lastPreviewFrame == getCurrentInterpolatedFrame())
		{
			lastPreviewMesh->AddRef();
			*outMesh = lastPreviewMesh;
			return MC_S_OK;
		}
		lastPreviewFrame = getCurrentInterpolatedFrame();

		TMCCountedPtr<IMCUnknown> progressKey;
		TMCString255 progressMsg;
		gResourceUtilities->GetIndString('3Din','DDEM',progressMsg, 130, 2);
		gShellUtilities->BeginProgress(progressMsg, &progressKey);

		FacetMesh::Create(&lastPreviewMesh);
		ThrowIfNil(lastPreviewMesh);

		unsigned long targetPolyCount;

		switch (fData.lPreviewSize) {
			case PREVIEW_16: 
				targetPolyCount = 16 * 16;
				break;			
			case PREVIEW_32: 
				targetPolyCount = 32 * 32;
				break;			
			case PREVIEW_64: 
				targetPolyCount = 64 * 64;
				break;			
			case PREVIEW_128:  
				targetPolyCount = 128 * 128;
				break;	
			default:
				unsigned long fullPolyCount = gcGetFullPolyCount(&fullMapInfo);
				if (fData.fResolution == 1.0f) {
					targetPolyCount = fullPolyCount;
				}
				else {
					targetPolyCount = (float)fullPolyCount * fData.fResolution;
				}
				break;
				}

		int meshType = GC_MESH_GRID_POINT;
		if (fData.lPreviewSize == PREVIEW_FULL) {
			switch (fData.meshType) {
				case gcGridPointSampling:
					meshType = GC_MESH_GRID_POINT;
					break;
				case gcGridBilinearFiltering:
					meshType = GC_MESH_GRID_BILINEAR;
					break;
				case gcGridSmartSplit:
					meshType = GC_MESH_GRID_SMART;
					break;
				case gcAdaptive:
					meshType = GC_MESH_ADAPTIVE;
					break;
			}
		}

		BuildMesh(lastPreviewMesh, fData.sizeX, fData.sizeY, fData.topElevation - fData.baseElevation, fData.baseElevation, meshType, targetPolyCount, fData.adaptiveError);

		TMCString255 facets;
		facets.FromInt32(lastPreviewMesh->fFacets.GetElemCount());

		fData.facets = facets + TMCString255(" facets");
		updateFacets();

		lastPreviewMesh->AddRef();
		*outMesh = lastPreviewMesh;

		gShellUtilities->EndProgress(progressKey);
	}
	catch (...)
	{
		Alert(TMCString255("Unexpected error building mesh."));
	}
	return MC_S_OK;
}

MCCOMErr DEMPrim::GetRenderingFacetMesh(FacetMesh** outMesh)
{
	try
	{
		setCurrentFrame(fData.animate * static_cast<real32>(fElevation.GetElemCount() - 1), fData.interpolateFrames);
		
		if (lastPreviewMesh && fData.lPreviewSize == PREVIEW_FULL && lastPreviewFrame == getCurrentInterpolatedFrame())
		{
			lastPreviewMesh->AddRef();
			*outMesh = lastPreviewMesh;
			return MC_S_OK;
		}
		lastPreviewFrame = getCurrentInterpolatedFrame();

		TMCCountedPtr<IMCUnknown> progressKey;
		TMCString255 progressMsg;
		gResourceUtilities->GetIndString('3Din','DDEM',progressMsg, 130, 2);
		gShellUtilities->BeginProgress(progressMsg, &progressKey);

		FacetMesh::Create(outMesh);
		ThrowIfNil(*outMesh);

		unsigned long fullPolyCount = gcGetFullPolyCount(&fullMapInfo);
		unsigned long targetPolyCount;

		if (fData.fResolution == 1.0f) {
			targetPolyCount = fullPolyCount;
		}
		else {
			targetPolyCount = (float)fullPolyCount * fData.fResolution;
		}

		int meshType = GC_MESH_GRID_SMART;
		switch (fData.meshType) {
			case gcGridPointSampling:
				meshType = GC_MESH_GRID_POINT;
				break;
			case gcGridBilinearFiltering:
				meshType = GC_MESH_GRID_BILINEAR;
				break;
			case gcGridSmartSplit:
				meshType = GC_MESH_GRID_SMART;
				break;
			case gcAdaptive:
				meshType = GC_MESH_ADAPTIVE;
				break;
		}

		BuildMesh(*outMesh, fData.sizeX, fData.sizeY, fData.topElevation - fData.baseElevation, fData.baseElevation, meshType, targetPolyCount, fData.adaptiveError);

		gShellUtilities->EndProgress(progressKey);
	}
	catch (...)
	{
		Alert(TMCString255("Unexpected error building mesh."));
	}
	return MC_S_OK;

}

MCCOMErr DEMPrim::ExtensionDataChanged()
{
	lastPreviewMesh = NULL;
	if (fData.sResolution.Length() == 0)
	{
		TMCString255 widthstr, heightstr;
		widthstr.FromInt32(fullMapInfo.samples[GC_EAST]);
		heightstr.FromInt32(fullMapInfo.samples[GC_NORTH]);

		fData.sResolution = widthstr + TMCString255(" x ") + heightstr;
	}
	TMCString255 facets;
	unsigned long fullPolyCount = gcGetFullPolyCount(&fullMapInfo), targetPolyCount = (float)fullPolyCount * fData.fResolution;

	facets.FromInt32(targetPolyCount);

	fData.facets = "";
	
	if (fData.meshType == gcAdaptive)
		fData.facets = TMCString255("Up to ");
	
	fData.facets += facets + TMCString255(" facets");
	
	return MC_S_OK;
}

MCCOMErr DEMPrim::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
	TMCCountedPtr<I3DShObject> object;
	TMCCountedPtr<I3DShPrimitive> prim;
	
	//warnings label, lets grab one
	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if (!warnings) { 
		TMCCountedPtr<IMFPart> parent;
		//if we don't have a pointer to the 
		sourcePart->GetPartParent(&parent);
		if (parent != NULL) {
			parent->FindChildPartByID(&warnings, 'FACE');
			}
		}
	return MC_S_OK;
}

void DEMPrim::updateFacets() {
	if (warnings && lastPreviewMesh && fData.lPreviewSize == PREVIEW_FULL) {
		TMCString255 facets;
		facets.FromInt32(lastPreviewMesh->fFacets.GetElemCount());

		fData.facets = facets + TMCString255(" facets");

		warnings->SetValue((void *)&fData.facets 
				, kStringValueType
				, true
				, false);
		warnings = NULL;
		}
	}

MCCOMErr DEMPrim::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	return ElevationMesh::Read(stream, readUnknown, privData);

}
MCCOMErr DEMPrim::Write(IShTokenStream* stream)
{

	return ElevationMesh::Write(stream);
}
