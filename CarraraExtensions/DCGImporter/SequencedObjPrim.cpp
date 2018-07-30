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
#include "SequencedObjPrim.h"
#include "DCGDebug.h"
#include "I3DShScene.h"
#include "I3DShTreeElement.h"
#include "MCPtr.h"
#include "ActionsParams.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShGroup.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "MFPartMessages.h"
#include "DCGSequencedFileHelper.h"
#include "I3DShShader.h"
#include "ISceneDocument.h"
#include "ISceneSelection.h"
#include "IChangeManagement.h"
#include "MCCountedPtrHelper.h"
#include "DCGImporterDLL.h"
#include "stdio.h"

const MCGUID CLSID_SequencedObjPrim(R_CLSID_SequencedObjPrim);
#define R_CLSID_ObjImporter 0x14115c5c,0x03ba,0x11d1,0x82,0x76,0x00,0xAA,0x00,0xc2,0x8e,0x8d
const MCGUID CLSID_ObjImporter(R_CLSID_ObjImporter);
const MCGUID IID_SequencedObjPrim(R_IID_SequencedObjPrim);





// Constructor of the C++ Object
SequencedObjPrim::SequencedObjPrim()
{
	lastPreviewFrame = currentFrame = -1;
	fData.animate = 0;
	fData.previewMode = pmCurrentFrame;
	fData.mapObjectYToCarraraZ = true;
	fData.conversion = 1.0f;
	fData.smoothing = 30;
	fData.objectType = FACET_MESH;
	fData.uvBetween0And1 = true;

	lastCaller = lcUnknown;
	inImport = false;

	//this must be created early. Create in GetRenderingFacetMesh causes exception when DoImport called
	gComponentUtilities->CoCreateInstance(CLSID_ObjImporter, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DExImportFilter, reinterpret_cast<void**>(&objImporter));

}

SequencedObjPrim::~SequencedObjPrim()
{
}

MCCOMErr SequencedObjPrim::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_SequencedObjPrim))
	{
		TMCCountedGetHelper<SequencedObjPrim> result(ppvObj);
		result = (SequencedObjPrim*)this;
		return MC_S_OK;
	}
	else
		return TBasicPrimitive::QueryInterface(riid, ppvObj);	
}

uint32 SequencedObjPrim::AddRef()
{
	return TBasicPrimitive::AddRef();
}

void SequencedObjPrim::GetBoundingBox(TBBox3D& bbox) 
{
	if (inImport)
	{
		loadDataFromStub();
	}
	else if (gShell3DUtilities->GetCurrentRoom() == kStoryBoardRoomID)
	{
		loadIfNeeded(flBoundingBox);
	}
	else
	{
		loadIfNeeded(flUnknown);
	}
	if (boundingBoxes[currentFrame].Valid())
	{
		bbox = boundingBoxes[currentFrame];
	}
}

uint32 SequencedObjPrim::GetUVSpaceCount() 
{
	return spaceInfo.GetElemCount();
}

MCCOMErr SequencedObjPrim::GetUVSpace( uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo) 
{
	if (uvSpaceID < spaceInfo.GetElemCount())
	{
		uvSpaceInfo->fID = spaceInfo[uvSpaceID].fID;
		uvSpaceInfo->fName = spaceInfo[uvSpaceID].fName;
		uvSpaceInfo->fWraparound[0] = spaceInfo[uvSpaceID].fWraparound[0];
		uvSpaceInfo->fWraparound[1] = spaceInfo[uvSpaceID].fWraparound[1];
	}
	return MC_S_OK;

}

 MCCOMErr SequencedObjPrim::GetNbrLOD(int16& nbrLod)
{
	nbrLod = 1;
	return MC_S_OK;

};

MCCOMErr SequencedObjPrim::GetLOD(int16 lodIndex, real& lod)
{
	lod = 1;
	return MC_S_OK;
};



MCCOMErr SequencedObjPrim::GetFacetMesh (uint32 lodIndex, FacetMesh** outMesh)
{
	lastCaller = lcGetFacetMesh;

	try
	{
		if (gShell3DUtilities->GetCurrentRoom() == kStoryBoardRoomID)
		{
			loadIfNeeded(flBoundingBox);
			if (boundingBoxes[currentFrame].Valid())
			{
				buildCubeMesh(outMesh, boundingBoxes[currentFrame]);
			}
			else
			{
				FacetMesh::Create(outMesh);
				//buildPlaceHolderMesh(outMesh);
			}
		}
		else if (fData.previewMode == pmBoundingBox)
		{
			loadIfNeeded(flUnknown);
			if (boundingBoxes[currentFrame].Valid())
			{
				buildCubeMesh(outMesh, boundingBoxes[currentFrame]);
			}
			else
			{
				FacetMesh::Create(outMesh);
				//buildPlaceHolderMesh(outMesh);
			}
		}
		else if (fData.previewMode == pmCurrentFrame)
		{
			loadIfNeeded(flUnknown);
			if (mesh)
			{
				mesh->AddRef();
				*outMesh = mesh;
			}
			else
			{
				FacetMesh::Create(outMesh);
				//buildPlaceHolderMesh(outMesh);
			}
		}
	}
	catch (...)
	{
		Alert(TMCString255("Unexpected error building mesh."));
	}
	return MC_S_OK;
}

MCCOMErr SequencedObjPrim::GetRenderingFacetMesh(FacetMesh** outMesh)
{
	lastCaller = lcGetRenderingFacetMesh;

	try
	{
		loadIfNeeded(flFullMesh);
		if (mesh)
		{
			mesh->AddRef();
			*outMesh = mesh;
		}
		else
		{
			//buildPlaceHolderMesh(outMesh);
			FacetMesh::Create(outMesh);
		}

	}
	catch (TMCException& exception)
	{
		//if it's not the exception thrown by the abort button
		//pop a message
		if (exception.GetError() != -2147417984) {
			Alert(TMCString255("Unexpected error building mesh."));
		}
		FacetMesh::Create(outMesh);
	}
	return MC_S_OK;

}

MCCOMErr SequencedObjPrim::ExtensionDataChanged()
{
	if (fData.conversion != lastfData.conversion ||
		fData.mapObjectYToCarraraZ != lastfData.mapObjectYToCarraraZ)
	{
		mesh = NULL;
		boundingBoxes.SetElemCount(0);
		boundingBoxes.SetElemCount(foundFiles.GetElemCount());
	}
	else if (fData.animate != lastfData.animate ||
		fData.smoothing != lastfData.smoothing ||
		fData.objectType != lastfData.objectType)
	{
		mesh = NULL;
	}
	lastfData = fData;
	return MC_S_OK;
}

void SequencedObjPrim::loadIfNeeded(ForceLoad forceLoad)
{
	if (foundFiles.GetElemCount() > 0)
	{
		currentFrame = getCalculatedFrame();

		if (forceLoad == flBoundingBox)
		{
			if (!boundingBoxes[currentFrame].Valid())
			{
				loadBoundingBox();
			}
		}
		else if (fData.previewMode == pmCurrentFrame || forceLoad == flFullMesh
			|| lastCaller == lcGetRenderingFacetMesh)
		{
			if (currentFrame != lastPreviewFrame
				|| !mesh)
			{
				lastPreviewFrame = currentFrame;
				loadOBJFile();
			}
		}
		else if (fData.previewMode == pmBoundingBox)
		{
			if (!boundingBoxes[currentFrame].Valid())
			{
				loadBoundingBox();
			}
		}
	}
}

MCCOMErr SequencedObjPrim::getFile(TMCDynamicString& fileName, IMCFile** file)
{
	fileName = foundFiles[currentFrame];
	TMCCountedGetHelper<IMCFile> result(file);
	TMCCountedPtr<IMCFile> newFile;
	gFileUtilities->CreateIMCFile(&newFile);
	newFile->SetWithFullPathName(fileName);

	if (!newFile->Exists())
	{

		TMCCountedPtr<IMCFile> file;
		gFileUtilities->CreateIMCFile(&file);
		file->SetWithFullPathName(fileName);
		IDType fileformat = 0, typeselection = 0;
		TMCArray<IDType> filetypes;
		TMCString255 filedialogtitle;
		gResourceUtilities->GetIndString(kRID_GeometricPrimitiveFamilyID,'DISO',filedialogtitle, 100, 1);
		filetypes.SetElemCount(1);
		filetypes.SetElem(0, 'DCOI');
		if (MC_S_OK == gFileFormatUtilities->OpenFileDialog(filedialogtitle, file, filetypes, typeselection, fileformat))
		{
			TMCDynamicString fullPathName;
			file->GetFileFullPathName(fullPathName);
			TMCClassArray<TMCDynamicString> newFoundFiles;
			DCGSequencedFileHelper::fillFileList(fullPathName, NULL, foundFiles);
			mesh = NULL;
			boundingBoxes.SetElemCount(0);
			boundingBoxes.SetElemCount(foundFiles.GetElemCount());

			currentFrame = getCalculatedFrame();
			fileName = foundFiles[currentFrame];
			gFileUtilities->CreateIMCFile(&newFile);
			newFile->SetWithFullPathName(fileName);
		}
		else
		{
			mesh = NULL;
			boundingBoxes.SetElemCount(0);
			boundingBoxes.SetElemCount(foundFiles.GetElemCount());
			return MC_S_FALSE;
		}
	}
	result = newFile;
	return MC_S_OK;

}


void SequencedObjPrim::loadBoundingBox() 
{
	TMCDynamicString fileName;
	TMCCountedPtr<IMCFile> file;
	if (MC_S_OK != getFile(fileName, &file))
	{
		return;
	}

	TMCCountedPtr<IMCUnknown> progressKey;
	TMCString255 progressMsg;
	gResourceUtilities->GetIndString(kRID_GeometricPrimitiveFamilyID,'DISO',progressMsg, 100, 2);
	gShellUtilities->BeginProgress(progressMsg, &progressKey);


	uint32 fileLength;
	file->Open();
	file->GetSize(fileLength);
	file->Close();

	mesh = NULL;

	TMCfstream* stream = NULL;
	TBBox3D& boundingBox = boundingBoxes[currentFrame];
	TVector3 point;	
	
	stream = new TMCfstream(fileName.StrGet(), kAnsiTranscoder, TMCiostream::in);
	
	TMCDynamicString line;
	TMCString15 vertexLine = "v";
	TMCString15 nan = "nan";

	TMCDynamicString token;
	boolean firstPoint = true;

	uint32 lineCount = 0;

	while (!stream->eof())
	{
		lineCount++;
		(*stream)>>token;
		stream->getLine(line);
		if (token.IsEqualNotCaseSensitive(vertexLine))
		{
			int32 position = stream->tellg();
			if (lineCount%500 == 0) 
			{
				gShellUtilities->SetProgressValue0to1( static_cast<real32>(position)/static_cast<real32>(fileLength), progressKey);	
			}
			if (line.SearchSBStr(nan) == -1)
			{
				if (fData.mapObjectYToCarraraZ)
				{
					sscanf(line.StrGet(), "%f %f %f", &point.x, &point.z, &point.y);
				}
				else
				{
					sscanf(line.StrGet(), "%f %f %f", &point.x, &point.y, &point.z);
				}
				point.x = -point.x;

				point*=fData.conversion;

				if (firstPoint) 
				{
					boundingBox.SetMax(point);
					boundingBox.SetMin(point);
					firstPoint = false;
				}
				else
				{
					boundingBox.AddPoint(point);
				}
			}
		}
	}
	delete stream;

	gShellUtilities->EndProgress(progressKey);
}

void SequencedObjPrim::loadOBJFile() 
{
	TMCDynamicString fileName;
	TMCCountedPtr<IMCFile> file;
	if (MC_S_OK != getFile(fileName, &file))
	{
		return;
	}


	TMCCountedPtr<IExDataExchanger> dataExchanger;
	objImporter->QueryInterface(IID_IExDataExchanger, reinterpret_cast<void**>(&dataExchanger));
	
	ObjImporterData* importerData = reinterpret_cast<ObjImporterData*>(dataExchanger->GetExtensionDataBuffer());
	importerData->TrCo = fData.mapObjectYToCarraraZ;
	importerData->Noto = true;
	importerData->Conv = fData.conversion;
	importerData->Obj0 = fData.objectType;
	importerData->Smoo = fData.smoothing;
	importerData->UV01 = fData.uvBetween0And1;

	gShell3DUtilities->CreateEmptyScene(&sceneStub);

	TMCCountedPtr<I3DShTreeElement> topTree;

	TMCCountedPtr<I3DShGroup> 	topGroup;
	sceneStub->CreateTreeRootIfNone();
	MCCOMErr err = sceneStub->GetTreeRoot(&topGroup);
	topGroup->QueryInterface(IID_I3DShTreeElement,reinterpret_cast<void**>(&topTree));


	TMCPtr<TImportInfo> importInfo;
	importInfo = NULL;
	
	inImport = true;
	stubDataLoaded = false;
	objImporter->DoImport(file, sceneStub, topTree, importInfo);
	inImport = false;
	loadDataFromStub();
	sceneStub = NULL;
}

void SequencedObjPrim::loadDataFromStub()
{
	if (stubDataLoaded == false)
	{
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> primitive;

		if (sceneStub->Get3DObjectsCount() > 0)
		{
			sceneStub->Get3DObjectByIndex(&object, 0);
			object->QueryInterface(IID_I3DShPrimitive, reinterpret_cast<void**>(&primitive));
			ThrowIfNil(primitive);

			primitive->GetRenderingFacetMesh(&mesh);
			mesh->CalcBBox(boundingBoxes[currentFrame]);
		}
		else
		{
			mesh = NULL;
		}
		stubDataLoaded = true;
	}
}

void SequencedObjPrim::Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger>	result(res);

	SequencedObjPrim* theClone = new SequencedObjPrim();
	ThrowIfNil(theClone);

	theClone->SetControllingUnknown(pUnkOuter);
	theClone->fData = fData;
	theClone->currentFrame = currentFrame;
	theClone->lastPreviewFrame = lastPreviewFrame;
	theClone->foundFiles = foundFiles;
	theClone->mesh = mesh;
	theClone->spaceInfo = spaceInfo;
	theClone->boundingBoxes = boundingBoxes;
	theClone->lastfData = lastfData;

	result = (IExDataExchanger*) theClone;
}

MCCOMErr SequencedObjPrim::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	// the keyword has been read
	int8 token[256];
	stream->GetBegin();
	uint32 spaceIndex = 0;
	uint32 fileIndex = 0;
	try
	{
		do
		{
			int16 err=stream->GetNextToken(token);
			//todo: update error handling
			//if(err!=0) throw TMCException(err,0);

			if (!stream->IsEndToken(token))
			{
				int32 keyword;

				stream->CompactAttribute(token, &keyword);

				switch (keyword)
				{
					case 'SPAC':
						spaceInfo.SetElemCount(stream->GetInt32Token());
						break;
					case 'FNMS':
						foundFiles.SetElemCount(stream->GetInt32Token());
						boundingBoxes.SetElemCount(foundFiles.GetElemCount());
						break;
					case 'SPIN':
						spaceIndex = stream->GetInt32Token();
						break;
					case 'SPID':
						spaceInfo[spaceIndex].fID = stream->GetInt32Token();
						break;
					case 'SNAM':
						spaceInfo[spaceIndex].fName = stream->GetStringToken();
						break;
					case 'WRP0':
						spaceInfo[spaceIndex].fWraparound[0] = stream->GetBooleanToken();
						break;
					case 'WRP1':
						spaceInfo[spaceIndex].fWraparound[1] = stream->GetBooleanToken();
						break;
					case 'FIIN':
						fileIndex = stream->GetInt32Token();
						break;
					case 'FNAM':
						foundFiles.SetElem(fileIndex, stream->GetStringToken());
						break;
					default:
						readUnknown(keyword, stream, privData);
				}

			}
		}
		while (!stream->IsEndToken(token));
	}
	catch (TMCException& exception)
	{
		throw exception;
	}
	return MC_S_OK;

}
MCCOMErr SequencedObjPrim::Write(IShTokenStream* stream)
{
	stream->Indent();
	stream->PutInt32Attribute('SPAC', spaceInfo.GetElemCount());
	stream->Indent();
	stream->PutInt32Attribute('FNMS', foundFiles.GetElemCount());
	for (uint32 spaceIndex = 0; spaceIndex < spaceInfo.GetElemCount(); spaceIndex++)
	{
		//uint32		 fID;				///< UV Space ID 
		//boolean 	 fWraparound[2];	///< Tells if the UV Space is closed in the U or V direction
		//TMCString31  fName;				///< UV Space Name

		stream->Indent();
		stream->PutInt32Attribute('SPIN', spaceIndex);
		stream->Indent();
		stream->PutInt32Attribute('SPID', spaceInfo[spaceIndex].fID);
		stream->Indent();
		stream->PutStringAttribute('SNAM', spaceInfo[spaceIndex].fName);
		stream->Indent();
		stream->PutBooleanAttribute('WRP0', spaceInfo[spaceIndex].fWraparound[0]);
		stream->Indent();
		stream->PutBooleanAttribute('WRP1', spaceInfo[spaceIndex].fWraparound[1]);
	}
	for (uint32 fileIndex = 0; fileIndex < foundFiles.GetElemCount(); fileIndex++)
	{
		stream->Indent();
		stream->PutInt32Attribute('FIIN', fileIndex);
		stream->Indent();
		stream->PutStringAttribute('FNAM', foundFiles[fileIndex]);

	}

	return MC_S_OK;
}

void SequencedObjPrim::buildPlaceHolderMesh(FacetMesh** outMesh)
{
	buildCubeMesh(outMesh, TBBox3D(TVector3(-3,-3,-3), TVector3(3,3,3)));
}




void SequencedObjPrim::buildCubeMesh(FacetMesh** outMesh, const TBBox3D& boundingBox)
{
	FacetMesh::Create(outMesh);
	FacetMesh& mesh = (**outMesh);

	mesh.SetFacetsCount(12);
	mesh.SetVerticesCount(24);

	//bottom
	mesh.fVertices[0].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[1].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[2].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMin.z);
	mesh.fVertices[3].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMin.z);

	mesh.fNormals[0].SetValues(0, 0, -1);
	mesh.fNormals[1] = mesh.fNormals[2] = mesh.fNormals[3] = mesh.fNormals[0];

	mesh.fFacets[0].pt1 = 0; mesh.fFacets[0].pt2 = 2; mesh.fFacets[0].pt3 = 1;
	mesh.fFacets[1].pt1 = 0; mesh.fFacets[1].pt2 = 3; mesh.fFacets[1].pt3 = 2;

	//top
	mesh.fVertices[4].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMax.z);
	mesh.fVertices[5].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMax.z);
	mesh.fVertices[6].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMax.z);
	mesh.fVertices[7].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMax.z);

	mesh.fNormals[4].SetValues(0, 0, 1);
	mesh.fNormals[5] = mesh.fNormals[6] = mesh.fNormals[7] = mesh.fNormals[4];

	mesh.fFacets[2].pt1 = 4; mesh.fFacets[2].pt2 = 5; mesh.fFacets[2].pt3 = 6;
	mesh.fFacets[3].pt1 = 4; mesh.fFacets[3].pt2 = 6; mesh.fFacets[3].pt3 = 7;

	//-X
	mesh.fVertices[8].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[9].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMax.z);
	mesh.fVertices[10].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMax.z);
	mesh.fVertices[11].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMin.z);

	mesh.fNormals[8].SetValues(-1, 0, 0);
	mesh.fNormals[9] = mesh.fNormals[10] = mesh.fNormals[11] = mesh.fNormals[8];

	mesh.fFacets[4].pt1 = 8; mesh.fFacets[4].pt2 = 9; mesh.fFacets[4].pt3 = 10;
	mesh.fFacets[5].pt1 = 8; mesh.fFacets[5].pt2 = 10; mesh.fFacets[5].pt3 = 11;


	//X
	mesh.fVertices[12].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[13].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMax.z);
	mesh.fVertices[14].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMax.z);
	mesh.fVertices[15].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMin.z);

	mesh.fNormals[12].SetValues(-1, 0, 0);
	mesh.fNormals[13] = mesh.fNormals[14] = mesh.fNormals[15] = mesh.fNormals[12];

	mesh.fFacets[6].pt1 = 12; mesh.fFacets[6].pt2 = 14; mesh.fFacets[6].pt3 = 13;
	mesh.fFacets[7].pt1 = 12; mesh.fFacets[7].pt2 = 15; mesh.fFacets[7].pt3 = 14;

	//-y
	mesh.fVertices[16].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[17].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMin.z);
	mesh.fVertices[18].SetValues(boundingBox.fMax.x, boundingBox.fMin.y, boundingBox.fMax.z);
	mesh.fVertices[19].SetValues(boundingBox.fMin.x, boundingBox.fMin.y, boundingBox.fMax.z);

	mesh.fNormals[16].SetValues(0, -1, 0);
	mesh.fNormals[17] = mesh.fNormals[18] = mesh.fNormals[19] = mesh.fNormals[16];

	mesh.fFacets[8].pt1 = 16; mesh.fFacets[8].pt2 = 17; mesh.fFacets[8].pt3 = 18;
	mesh.fFacets[9].pt1 = 16; mesh.fFacets[9].pt2 = 18; mesh.fFacets[9].pt3 = 19;

	//y
	mesh.fVertices[20].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMin.z);
	mesh.fVertices[21].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMin.z);
	mesh.fVertices[22].SetValues(boundingBox.fMax.x, boundingBox.fMax.y, boundingBox.fMax.z);
	mesh.fVertices[23].SetValues(boundingBox.fMin.x, boundingBox.fMax.y, boundingBox.fMax.z);

	mesh.fNormals[20].SetValues(0, 1, 0);
	mesh.fNormals[21] = mesh.fNormals[22] = mesh.fNormals[23] = mesh.fNormals[20];

	mesh.fFacets[10].pt1 = 20; mesh.fFacets[10].pt2 = 22; mesh.fFacets[10].pt3 = 21;
	mesh.fFacets[11].pt1 = 20; mesh.fFacets[11].pt2 = 23; mesh.fFacets[11].pt3 = 22;

}
