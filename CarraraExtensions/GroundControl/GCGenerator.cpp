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
#include "GCGenerator.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "MFPartMessages.h"
#include "COMUtilities.h"
#include "IMCFile.h"
#include "IShUtilities.h"
#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GCGenerator(R_CLSID_GCGenerator);
#else
const MCGUID CLSID_GCGenerator={R_CLSID_GCGenerator};
#endif

void GCGenerator::Filter(TMCArray<real>& heightField, TVector2& heightBound,
								 const TIndex2& size, const TVector2& cellSize)

{

	real32 heightRange = fData.fTop - fData.fBottom;
	heightBound.x = fData.fBottom;
	heightBound.y = fData.fTop;

	if (fElevation.GetElemCount() > 0 && fElevation[0] != NULL)
	{
		gcSetExternalBuffer(&fullMapInfo, fElevation[getCurrentFrame()]->getDataPointer(0));

		TMCArray<real32> northLines;
		TMCArray<real32> eastLines;
		uint32 east, north;
		real32 oneoverEast = 1.0f / static_cast<real32>(size.y - 1);
		real32 oneoverNorth = 1.0f / static_cast<real32>(size.x - 1);

		northLines.SetElemCount(size.x);
		eastLines.SetElemCount(size.y);
		//calculate slices
		for (east = 0; east < size.y; east++){
			eastLines[east] = east * oneoverEast;
			}

		for (north = 0; north < size.x; north++) {
			northLines[north] = north * oneoverNorth;
			}

		for (east = 0; east < size.y; east++){
			for (north = 0; north < size.x; north++) {
				float elevation = 
					gcGetElevationBySample(&fullMapInfo, fData.lFiltering, 1.0f - eastLines[east], 1.0f - northLines[north]);
				heightField[east * size.x + north] = fData.fBottom + elevation * heightRange;;
			}
		}
	}//end elevation not null
}


MCCOMErr GCGenerator::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	if ((sourceID == 'OPEN')&&(message == EMFPartMessage::kMsg_PartValueChanged)) 
	{
		TMCCountedPtr<IMCFile> file;
		gFileUtilities->CreateIMCFile(&file);
		IDType fileformat = 0, typeselection = 0;
		TMCArray<IDType> filetypes;
		TMCString255 filedialogtitle;
		gResourceUtilities->GetIndString('3Din','DDEM',filedialogtitle, 130, 3);
		filetypes.SetElemCount(9);
		filetypes.SetElem(0, 'DDEM');
		filetypes.SetElem(1, 'GCWM');
		filetypes.SetElem(2, 'GCTG');
		filetypes.SetElem(3, 'GCBI');
		filetypes.SetElem(4, 'GCHG');
		filetypes.SetElem(5, 'GCPG');
		filetypes.SetElem(6, 'GCAA');
		filetypes.SetElem(7, 'GCGF');
		filetypes.SetElem(8, 'GCPD');
		if (MC_S_OK == gFileFormatUtilities->OpenFileDialog(filedialogtitle, file, filetypes, typeselection, fileformat))
		{
			DEM dem;
			static_cast<DEMImpData*>(dem.GetExtensionDataBuffer())->lImportAs = iaGenerator;
			static_cast<DEMImpData*>(dem.GetExtensionDataBuffer())->shading = gcsNone;
			TMCCountedPtr<I3DShScene> scenejunk;
			TMCCountedPtr<I3DShTreeElement> treejunk;
			dem.DoImport(file, scenejunk, treejunk);
			fElevation.SetElemCount(1);
			fElevation = dem.fElevation;
			fullMapInfo.samples[GC_EAST] = dem.fullMapInfo.samples[GC_EAST];
			fullMapInfo.samples[GC_NORTH] = dem.fullMapInfo.samples[GC_NORTH];
			fData.sDescription = dem.sDescription;
			fData.sResolution = dem.sResolution;
			fData.fBottom = 0;
			fData.fTop = 12000;
			fData.eastSize = 120000;
			fData.northSize = 120000;
			if (fData.bUseFileHeight && dem.fullMapInfo.hasRealSize == GC_TRUE) {
				//gc native meters, carrara native inches
				fData.eastSize = dem.fullMapInfo.size[GC_EAST] * 39.3701f;
				fData.northSize = dem.fullMapInfo.size[GC_NORTH] * 39.3701f;;
			}
			if (fData.bUseFileHeight && (dem.fullMapInfo.hasRealSize == GC_TRUE || dem.fullMapInfo.hasRealElevation == GC_TRUE)) {
				fData.fBottom = dem.fullMapInfo.minElevation * 39.3701f;
				fData.fTop = dem.fullMapInfo.maxElevation * 39.3701f;
			}
		}//file not null
	}
	return MC_S_OK;
}

void GCGenerator::Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger>	result(res);

	GCGenerator* theClone = new GCGenerator();
	ThrowIfNil(theClone);

	theClone->SetControllingUnknown(pUnkOuter);
	theClone->fData = fData;
	theClone->fullMapInfo.samples[GC_NORTH] = fullMapInfo.samples[GC_NORTH];
	theClone->fullMapInfo.samples[GC_EAST] = fullMapInfo.samples[GC_EAST];
	theClone->fElevation = fElevation;

	result = (IExDataExchanger*) theClone;

}
MCCOMErr GCGenerator::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}
	else
		return TBasicTerrainFilter::QueryInterface(riid, ppvObj);	
}
uint32 GCGenerator::AddRef()
{
	return TBasicTerrainFilter::AddRef();
}

MCCOMErr GCGenerator::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	return ElevationMesh::Read(stream, readUnknown, privData);
}

MCCOMErr GCGenerator::Write(IShTokenStream* stream)
{
	return ElevationMesh::Write(stream);
}

