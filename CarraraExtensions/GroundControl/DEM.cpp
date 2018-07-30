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
#include "DEM.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShShader.h"
#if (VERSIONNUMBER >= 0x030000)
#include "I3DShMasterGroup.h"
#endif
#include "IShChannel.h"
#include "DEMDLL.h"
#include "IComponentAnim.h"
#include "I3DShComponentOwner.h"
#include "IShComponent.h"
#include "I3DExPrimitive.h"
#include "TerrainData.h"
#include "dcgdebug.h"
#include "DEMPrim.h"
#include <stdlib.h>
#include "IMFTextPopupPart.h" 
#include "MFPartMessages.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "commessages.h"
#include "GCBump.h"
#include "DCGSequencedFileHelper.h"
#include "float.h"

//#include "Windows.h"
const MCGUID CLSID_DEM(R_CLSID_DEM);
const MCGUID CLSID_r32(R_CLSID_r32);
const MCGUID CLSID_TER(R_CLSID_TER);
const MCGUID CLSID_HGT(R_CLSID_HGT);
const MCGUID CLSID_BIL(R_CLSID_BIL);
const MCGUID CLSID_PGM(R_CLSID_PGM);
const MCGUID CLSID_IMG(R_CLSID_IMG);
const MCGUID CLSID_ASC(R_CLSID_ASC);
const MCGUID CLSID_FLT(R_CLSID_FLT);
const MCGUID CLSID_PDS(R_CLSID_PDS);

const MCGUID CLSID_StandardExternalPrimitive(R_CLSID_StandardExternalPrimitive);

inline void SetChunkSize(TMCRect& chunk, int32 chunkx, int32 chunky, const TMCRect &bounds, int32 CHUNK_SIZE) {
	chunk.left = bounds.left + CHUNK_SIZE * chunkx;
	chunk.top = bounds.top + CHUNK_SIZE * chunky;
	if (bounds.left + CHUNK_SIZE * chunkx + CHUNK_SIZE > bounds.right) {
		chunk.right = bounds.right;
		}
	else {
		chunk.right = bounds.left + CHUNK_SIZE * chunkx + CHUNK_SIZE;
		}
	if (bounds.top + CHUNK_SIZE * chunky + CHUNK_SIZE > bounds.bottom) {
		chunk.bottom = bounds.bottom;
		}
	else {
		chunk.bottom = bounds.top + CHUNK_SIZE * chunky + CHUNK_SIZE;
		}
	}

DEM::DEM()
{

	fData.lImportAs = iaDEM;
	fData.sizeX = 20.0f;
	fData.sizeY = 20.0f;
	fData.sizeZ = 2.0f;
	fData.shading = gcsShader;
	fData.scaling = gcscaleReal + 4;
	fData.fResolution = 0.10f;
	fData.adaptiveError = 0.01f;
	fData.lookForSequenced = false;
	fData.meshType = gcGridSmartSplit;

	bDoTexture = false;

	cleanUp();
}


DEM::~DEM() 
{

}

void DEM::cleanUp()
{
	gcCleanUp(&fullMapInfo);
	if (fData.lImportAs != iaGenerator) 
	{
		//this object is no longer needed
		fElevation.SetElemCount(0);
	}
	fMasterGroup = NULL;
}

void DEM::buildBasicScene(I3DShScene *scene, I3DShTreeElement *fatherTree, TMCCountedPtr<I3DShTreeElement> &topTree)
{
	ThrowIfNil(scene);
	TMCCountedPtr<I3DShMasterGroup> masterGroup;
	scene->QueryInterface(IID_I3DShMasterGroup,(void**)&masterGroup);

	fMasterGroup = masterGroup;
	if (fatherTree == NULL)
	{
		TMCCountedPtr<I3DShGroup> 	topGroup;
		scene->CreateTreeRootIfNone();
		MCCOMErr err = scene->GetTreeRoot(&topGroup);
		topGroup->QueryInterface(IID_I3DShTreeElement,(void**)  &topTree);
	}
	else
	{
		TMCCountedPtr<I3DShGroup> 	topGroup;
		MCCOMErr err = scene->GetTreeRoot(&topGroup);
		topGroup->QueryInterface(IID_I3DShTreeElement,(void**)  &topTree);
	}

	scene->CreateRenderingCameraIfNone('coni', (fatherTree == NULL));	// Create a conical rendering camera if none, and a Distant light if we not DEMorting in an existing scene
}
//todo:add an event handler to enable/disable UI elements with options that don't make sense together
//instead of just doing what works

MCCOMErr DEM::DoImport(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree)
{
	updateProgressBar pb;

	gcInitialize(&fullMapInfo);

	ThrowIfNil(file);

	bDoTexture = false;
	bRequiresTexture = false;
	try
	{
		TMCClassArray<TMCDynamicString> foundFiles;
		TMCString255 progressMsg;
		gResourceUtilities->GetIndString('3Din','DDEM',progressMsg, 130, 1);
		gShellUtilities->BeginProgress(progressMsg, &pb.progressKey);
		gcRegisterProgressCallback(&fullMapInfo, updateProgressBarCallback, &pb);
		
		float firstMinElevation, firstMaxElevation;

		file->GetFileFullPathName(fullPathName);
		if (fData.lookForSequenced && fData.lImportAs == iaDEM)
		{
			DCGSequencedFileHelper::fillFileList(fullPathName, NULL, foundFiles);
		}
		else
		{
			foundFiles.InsertElem(0, fullPathName);
		}

		TMCCountedPtr<I3DShTreeElement> topTree;

		if (fData.lImportAs != iaGenerator)
		{
			buildBasicScene(scene, fatherTree, topTree);
		}

		fElevation.SetElemCount(foundFiles.GetElemCount());
		elevationRange.SetElemCount(foundFiles.GetElemCount());

		for (uint32 fileIndex = 0; fileIndex < foundFiles.GetElemCount(); fileIndex++)
		{
			setCurrentFrame(fileIndex, false);
			
			pb.progressStart = static_cast<real32>(fileIndex) / static_cast<real32>(foundFiles.GetElemCount()); 
			pb.progressRun = static_cast<real32>(1) / static_cast<real32>(foundFiles.GetElemCount()); 
			readFile(fileIndex, foundFiles[fileIndex]);
			if (fileIndex == 0)
			{
				buildDescriptionAndName();
				firstMinElevation = fullMapInfo.minElevation;
				firstMaxElevation = fullMapInfo.maxElevation;
				if (fData.scaling >= gcscaleReal && (fullMapInfo.hasRealSize == GC_TRUE || fullMapInfo.hasRealElevation)) {
					real32 scale = 1.0f / pow(10.0f, fData.scaling - gcscaleReal);
					fullElevationRange.SetValues(fullMapInfo.minElevation * 39.3701f * scale, fullMapInfo.maxElevation * 39.3701f * scale);
				}
				else {
					fullElevationRange.SetValues(0, fData.sizeZ);
				}
				gcOverrideElevationRange(&fullMapInfo, fullMapInfo.minElevation, fullMapInfo.maxElevation);
				elevationRange[fileIndex].SetValues(0, 1.0f);
			}
			else  {
				elevationRange[fileIndex].SetValues( (fullMapInfo.minElevation - firstMinElevation) / (firstMaxElevation - firstMinElevation)
					, (fullMapInfo.maxElevation - firstMinElevation) / (firstMaxElevation - firstMinElevation));
			}
			gShellUtilities->SetProgressValue(pb.progressStart * 100.0f, pb.progressKey);
		}
		setCompleteElevationRange();
		gcDeregisterProgressCallback(&fullMapInfo);
		gShellUtilities->EndProgress(pb.progressKey);

		if (fData.lImportAs != iaGenerator) 
		{	
			buildAndShadeObject(scene, topTree);
		}
		cleanUp();
		
	}
	
	catch (TMCException& exception)
	{
		if (pb.progressKey) {
			gShellUtilities->EndProgress(pb.progressKey);
		}
		cleanUp();
		throw exception;
	}

	return MC_S_OK;
}



void DEM::buildDescriptionAndName()
{
	//build the name
	if (sDescription.Length() > 0) {
		name = sDescription;
	}
	else {
		TMCString15 sep;
		gFileUtilities->GetSeparator(sep);

		//name based on file name
		uint32 extensionpos = fullPathName.FindCharPosFromRight('.');
		uint32 slashpos = fullPathName.FindCharPosFromRight(sep[0]);
		if (extensionpos == 0)
		{
			extensionpos = fullPathName.Length() - 1;
		}
		name = fullPathName;
		name.SubString(slashpos + 1, extensionpos - slashpos - 1);
	}
}

void DEM::buildAndShadeObject(I3DShScene* scene, I3DShTreeElement* topTree)
{
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShTreeElement> instanceTree;
	TMCCountedPtr<I3DShObject> shObject;
	TMCCountedPtr<IExDataExchanger> dataexchanger;
	TMCCountedPtr<I3DShFacetMeshPrimitive> facetMeshPrimitive;
	switch (fData.lImportAs)
	{
		case iaDEM:
			gShell3DUtilities->CreatePrimitiveByID('DEMP', &shObject, IID_IExDataExchanger, (void**) &dataexchanger);
			break;
		case iaMesh:
			gShell3DUtilities->CreatePrimitiveByID(kFacetMeshPrimitiveID, &shObject, IID_I3DShFacetMeshPrimitive, (void**) &facetMeshPrimitive);
			break;
	}

	TMCCountedPtr<I3DShShader> multiShader;
	TMCCountedPtr<I3DShShader> colorShader;
	TMCCountedPtr<I3DShShader> highlightShader;
	TMCCountedPtr<I3DShShader> bumpShader;
	TMCCountedPtr<I3DShShader> shininessShader;
	TMCCountedPtr<I3DShShader> reflectionShader;
	TMCCountedPtr<I3DShShader> transparencyShader;
	TMCCountedPtr<I3DShMasterShader> masterShader;

	//build the shader for the object
	gShell3DUtilities->CreateValueShader(0.25f, &highlightShader);
	gShell3DUtilities->CreateValueShader(0.20f, &shininessShader);
	gShell3DUtilities->CreateValueShader(0.0, &reflectionShader);
	gShell3DUtilities->CreateValueShader(0.0, &transparencyShader);
	if (fData.shading == gcsNone)
	{
		TMCColorRGB inColor(1, 0, 0);
		gShell3DUtilities->CreateColorShader(inColor, &colorShader);
		gShell3DUtilities->CreateMultiChannelShader(
						colorShader, highlightShader, shininessShader, bumpShader, reflectionShader,
						transparencyShader,	NULL, NULL, &multiShader);
	}
	else if (fData.shading == gcsShader && fData.lImportAs == iaDEM)
	{
		TMCCountedPtr<IShComponent> component;
		gComponentUtilities->CreateComponent(kRID_ShaderFamilyID, 'GCSH', &component);
		ThrowIfNil(component);
		component->QueryInterface(IID_I3DShShader, (void**) &colorShader);
		gComponentUtilities->CreateComponent(kRID_ShaderFamilyID, 'GCSH', &component);
		ThrowIfNil(component);
		component->QueryInterface(IID_I3DShShader, (void**) &bumpShader);
		gShell3DUtilities->CreateMultiChannelShader(
					colorShader, highlightShader, shininessShader, bumpShader, reflectionShader,
					transparencyShader,	NULL, NULL, &multiShader, NULL, 1.0f);
	}			
	else
	{
		MapParams inMapParams;
		inMapParams.fBrightness = 1;
		inMapParams.fFlipHoriz = false;
		inMapParams.fFlipVert = true;
		inMapParams.fNinetyDegreeCount = 0;

		gShell3DUtilities->CreateTextureMapShader(texture, inMapParams,	&colorShader);
		gShell3DUtilities->CreateTextureMapShader(texture, inMapParams,	&bumpShader);
		gShell3DUtilities->CreateMultiChannelShader(
						colorShader, highlightShader, shininessShader, bumpShader, reflectionShader,
						transparencyShader,	NULL, NULL, &multiShader);
	}

	gShell3DUtilities->CreateMasterShader(multiShader, scene, &masterShader);

	shObject->SetName(name);
	masterShader->SetName(name);

	//set all the primitive's properties
	switch (fData.lImportAs)
	{
		case iaDEM:
			{
				DEMPrimData* PrimData = static_cast<DEMPrimData*>(dataexchanger->GetExtensionDataBuffer());
				PrimData->fResolution = fData.fResolution;
				PrimData->sizeX = importSize.x;
				PrimData->sizeY = importSize.y;
				PrimData->topElevation = fullElevationRange.y;
				PrimData->baseElevation = fullElevationRange.x;
				PrimData->sDescription = sDescription;
				PrimData->adaptiveError = fData.adaptiveError;
				PrimData->meshType = fData.meshType;
				TMCCountedPtr<DEMPrim> demprim;
				dataexchanger->QueryInterface(IID_DEMPrim,(void**)&demprim);
				demprim->fullMapInfo.samples[GC_NORTH] = fullMapInfo.samples[GC_NORTH];
				demprim->fullMapInfo.samples[GC_EAST] = fullMapInfo.samples[GC_EAST];
				demprim->fElevation = fElevation;
				demprim->elevationRange = elevationRange;
				demprim->setCompleteElevationRange();
				dataexchanger->ExtensionDataChanged();
			}
			break;
		case iaMesh:
			{
				TBBox3D bbox;
				bbox.fMin[0] = -importSize.x * 0.5;
				bbox.fMin[1] = -importSize.y * 0.5;
				bbox.fMin[2] = fullElevationRange.x;
				bbox.fMax[0] = -bbox.fMin[0];
				bbox.fMax[1] = -bbox.fMin[1];
				bbox.fMax[2] = fullElevationRange.y;
				facetMeshPrimitive->SetNbrLOD(1); // Must be called before 'SetFacetMesh'
				facetMeshPrimitive->SetBoundingBox(bbox);

				TMCCountedPtr<FacetMesh> theMesh;
				FacetMesh::Create(&theMesh);
				ThrowIfNil(theMesh);

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
				BuildMesh(theMesh, importSize.x, importSize.y, elevationRange[0].y - elevationRange[0].x, elevationRange[0].x, meshType, targetPolyCount, fData.adaptiveError );
		
				facetMeshPrimitive->SetFacetMesh(0, 0, theMesh);			
			}
			break;

	}

	gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL,
		MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**) &instance);

	instance->Set3DObject(shObject);
	instance->SetShader (masterShader);
	instance->QueryInterface(IID_I3DShTreeElement, (void**) &instanceTree);

	MCAssert(fMasterGroup);
	instanceTree->SetMasterGroup(fMasterGroup);
	instanceTree->SetName(name);

	topTree->InsertLast(instanceTree);

}


void DEM::readFile(uint32 frame, TMCDynamicString fileName)
{
	try
	{	
		if (gcCanOpenFile(fileName.StrGet(), &fullMapInfo)== GC_ERROR)
		{
			gcCleanUp(&fullMapInfo);
			return;
		}	

		TMCCountedPtr<CountedRealArray> currentFrameElevation;
		CountedRealArray::Create(&currentFrameElevation);
		fElevation.SetElem(frame, currentFrameElevation);
		fElevation[frame]->setDataElemCount(fullMapInfo.bufferSize);
		gcSetExternalBuffer(&fullMapInfo, fElevation[frame]->getDataPointer(0));
		
		if (gcOpenFile(fileName.StrGet(), &fullMapInfo)== GC_ERROR)
		{
			gcCleanUp(&fullMapInfo);
			return;
		}	
		gcNormalizeAndClipElevation(&fullMapInfo);		

		gcFillNoData(&fullMapInfo, GC_NO_DATA_PERIMETER_AVERAGE, 0);

		if (fullMapInfo.description) {
			sDescription.FromCPtr(fullMapInfo.description);
		}
		else {
			sDescription = "";
		}

		sResolution = "";

		importSize.x = fData.sizeX;
		importSize.y = fData.sizeY;

		if (fData.scaling >= gcscaleReal && fullMapInfo.hasRealSize == GC_TRUE) {
			real32 scale = 1.0f / pow(10.0f, fData.scaling - gcscaleReal);
			importSize.x = fullMapInfo.size[GC_NORTH] * 39.3701f * scale;
			importSize.y = fullMapInfo.size[GC_EAST] * 39.3701f * scale;
		}
		else if (fData.scaling == gcscaleProportion || fData.scaling >= gcscaleReal)
		{
			if (fullMapInfo.samples[GC_EAST] < fullMapInfo.samples[GC_NORTH])
			{
				importSize.y = importSize.x * fullMapInfo.samples[GC_EAST] / fullMapInfo.samples[GC_NORTH];
			}
			else
			{
				importSize.x = importSize.y * fullMapInfo.samples[GC_NORTH] / fullMapInfo.samples[GC_EAST];

			}
		}

		bRequiresTexture = (bRequiresTexture || (fData.shading == gcsTexture) || (fData.shading == gcsShader && fData.lImportAs != iaDEM));

		bDoTexture = bRequiresTexture;

		if (bDoTexture)
		{
			TMCArray<uint8> r;
			TMCArray<uint8> g;
			TMCArray<uint8> b;

			r.SetElemCount(fullMapInfo.bufferSize);
			g.SetElemCount(fullMapInfo.bufferSize);
			b.SetElemCount(fullMapInfo.bufferSize);

			for (uint32 east = 0; east < fullMapInfo.samples[GC_EAST]; east++) {
				for (uint32 north = 0; north < fullMapInfo.samples[GC_NORTH]; north++) {
					unsigned long index = gcGetElevationIndex(&fullMapInfo, east, north);
					float value = gcGetElevationByIndex(&fullMapInfo, index);
					value *= 255.0f;
					r[index] = value;
					g[index] = value;
					b[index] = value;
				}
			}

			TextureFromBuffers(r, g, b);
			r.ArrayFree();
			g.ArrayFree();
			b.ArrayFree();
		}


		bDoTexture = false;
		{
			TMCString255 widthstr, heightstr;
			widthstr.FromInt32(fullMapInfo.samples[GC_EAST]);
			heightstr.FromInt32(fullMapInfo.samples[GC_NORTH]);

			sResolution = widthstr + TMCString255(" x ") + heightstr;
		}


	}
	catch (TMCException& exception)
	{
		throw exception;
	}
}

void DEM::TextureFromBuffers(
		TMCArray<uint8>& r, 	TMCArray<uint8>& g, TMCArray<uint8>& b)
{
	TMCCountedPtr<IShChannel> channel;
	IShChannelTileIterator* channeliter = NULL;
	const TChannelDataBucket *bucket;
	TMCRect bounds, chunk;
	int32 x, y, height, width,chunkx,chunky, CHUNK_SIZE, sizeH, sizeV;

	int32 ColorBinIndex;

	gComponentUtilities->CoCreateInstance(CLSID_TextureMap, NULL, MC_CLSCTX_INPROC_SERVER, IID_IShTextureMap, (void**) &texture);
	gShell3DUtilities->CreateChannel(&channel, fullMapInfo.samples[GC_NORTH] , fullMapInfo.samples[GC_EAST], 32);
	TChannelID newChannelID(TChannelID::eARGB_CHANNEL, 1);
	channel->SetID(newChannelID);
	channel->FillWithOnes();
	TextureMappingInfo tminfo;
	texture->GetTextureMappingInfo(tminfo);
	channel->GetBounds(bounds);
	sizeH = bounds.GetWidth();
	sizeV = bounds.GetHeight();
	texture->SetInternalMode(true);
	channel->GetChannelTileIterator(&channeliter, bounds);
	channeliter->First();
	channeliter->GetTile(&bucket, eTileRead);
	CHUNK_SIZE = bucket->Width();
	channeliter->UnGetTile(&bucket,true);
	channeliter->Release();

	for (chunkx = 0; chunkx < (sizeH + CHUNK_SIZE - 1) / CHUNK_SIZE; chunkx++) {
		for (chunky = 0; chunky < (sizeV + CHUNK_SIZE - 1) / CHUNK_SIZE; chunky++) {
			//set the boundaries for this chunk
			SetChunkSize(chunk,chunkx, chunky, bounds, CHUNK_SIZE);

			channel->GetTile(chunk, &bucket, eTileReadWrite);
			width=chunk.GetWidth();
			height=chunk.GetHeight();
			for (y=0;y<height;y++)	{
				
				TMCColorRGBA8 *color = (TMCColorRGBA8*)bucket->RowPtr32(y);
				for (x=0;x<width;x++){
						ColorBinIndex = (sizeV - (chunk.top + y) - 1) + (sizeH - (chunk.left + x) - 1) * sizeV;
						color[x].red = r[ColorBinIndex];
						color[x].blue = g[ColorBinIndex];
						color[x].green = b[ColorBinIndex];

					}//end x
				}//end y
			}//end chunky
		
		}//end chunkx

	//create the shader
	texture->SetChannel(channel);
}
