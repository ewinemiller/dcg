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
#include "SequencedObjImp.h"
#include "I3DShMasterGroup.h"

#include "I3DShScene.h"
#include "I3DShTreeElement.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShGroup.h"
#include "I3DShShader.h"

#include "I3DShInstance.h"
#include "comsafeutilities.h"
#include "IShUtilities.h"
#include "DCGSequencedFileHelper.h"
#include "SequencedObjPrim.h"
#include "MCPtr.h"

#include "DCGImporterDLL.h"

const MCGUID CLSID_SequencedObjImp ( R_CLSID_SequencedObjImp );
#define R_CLSID_ObjImporter 0x14115c5c,0x03ba,0x11d1,0x82,0x76,0x00,0xAA,0x00,0xc2,0x8e,0x8d
const MCGUID CLSID_ObjImporter(R_CLSID_ObjImporter);

SequencedObjImp::SequencedObjImp()
{
	fData.mapObjectYToCarraraZ = true;
	fData.conversion = 1.0f;
	fData.smoothing = 30;
	fData.objectType = FACET_MESH;
	fData.uvBetween0And1 = true;

	fMasterGroup = NULL;
}

SequencedObjImp::~SequencedObjImp() 
{
}

void SequencedObjImp::buildBasicScene(I3DShScene *scene, I3DShTreeElement *fatherTree, TMCCountedPtr<I3DShTreeElement> &topTree)
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

MCCOMErr SequencedObjImp::DoImport(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree)
{
	TMCCountedPtr<SequencedObjPrim> sequencedObjPrim;
	TMCDynamicString fileName;
	TMCDynamicString name;
	TMCCountedPtr<I3DShTreeElement> topTree;
	buildBasicScene(scene, fatherTree, topTree);
	
	//create my object
	TMCCountedPtr<I3DShObject> object;
	TMCCountedPtr<IExDataExchanger> dataexchanger;
	gShell3DUtilities->CreatePrimitiveByID('DISO', &object, IID_IExDataExchanger, reinterpret_cast<void**>(&dataexchanger));
	dataexchanger->QueryInterface(IID_SequencedObjPrim,reinterpret_cast<void**>(&sequencedObjPrim));

	file->GetFileFullPathName(fileName);
	DCGSequencedFileHelper::fillFileList(fileName, NULL, sequencedObjPrim->foundFiles);
	fileName = sequencedObjPrim->foundFiles[0];
	sequencedObjPrim->lastPreviewFrame = sequencedObjPrim->currentFrame = 0;
	sequencedObjPrim->boundingBoxes.SetElemCount(sequencedObjPrim->foundFiles.GetElemCount());

	buildName(fileName, name);
	object->SetName(name);

	sequencedObjPrim->fData.conversion = fData.conversion;
	sequencedObjPrim->fData.mapObjectYToCarraraZ = fData.mapObjectYToCarraraZ;
	sequencedObjPrim->fData.objectType = fData.objectType;
	sequencedObjPrim->fData.smoothing = fData.smoothing;
	sequencedObjPrim->fData.uvBetween0And1 = fData.uvBetween0And1;


	//build our temp scene to get the OBJ importer to do it's work
	TMCCountedPtr<I3DShScene> tempScene;
	gShell3DUtilities->CreateEmptyScene(&tempScene);

	TMCCountedPtr<I3DShTreeElement> tempTopTree;

	TMCCountedPtr<I3DShGroup> topGroup;
	tempScene->CreateTreeRootIfNone();
	MCCOMErr err = tempScene->GetTreeRoot(&topGroup);
	topGroup->QueryInterface(IID_I3DShTreeElement,reinterpret_cast<void**>(&tempTopTree));


	TMCCountedPtr<I3DExImportFilter> objImporter;
	TMCCountedPtr<I3DShObject> tempObject;
	TMCCountedPtr<I3DShPrimitive> tempPrimitive;

	gComponentUtilities->CoCreateInstance(CLSID_ObjImporter, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DExImportFilter, reinterpret_cast<void**>(&objImporter));

	TMCCountedPtr<IExDataExchanger> importerDataExchanger;
	objImporter->QueryInterface(IID_IExDataExchanger, (void**)&importerDataExchanger);
	
	ObjImporterData* importerData = reinterpret_cast<ObjImporterData*>(importerDataExchanger->GetExtensionDataBuffer());
	importerData->TrCo = fData.mapObjectYToCarraraZ;
	importerData->Noto = true;
	importerData->Conv = fData.conversion;
	importerData->Obj0 = fData.objectType;
	importerData->Smoo = fData.smoothing;
	importerData->UV01 = fData.uvBetween0And1;

	TMCCountedPtr<IMCFile> tempFile;
	gFileUtilities->CreateIMCFile(&tempFile);
	tempFile->SetWithFullPathName(fileName);

	boolean test = tempFile->Exists();

	TMCPtr<TImportInfo> importInfo;
	importInfo = NULL;
	
	objImporter->DoImport(file, tempScene, tempTopTree, importInfo);

	tempScene->Get3DObjectByIndex(&tempObject, 0);
	tempObject->QueryInterface(IID_I3DShPrimitive, reinterpret_cast<void**>(&tempPrimitive));

	tempPrimitive->GetRenderingFacetMesh(&sequencedObjPrim->mesh);
	sequencedObjPrim->mesh->CalcBBox(sequencedObjPrim->boundingBoxes[sequencedObjPrim->currentFrame]);

	sequencedObjPrim->spaceInfo.SetElemCount(tempObject->GetUVSpaceCount());

	for (uint32 spaceIndex = 0; spaceIndex < sequencedObjPrim->spaceInfo.GetElemCount(); spaceIndex++)
	{
		UVSpaceInfo tempSpaceInfo;

		tempObject->GetUVSpace(spaceIndex, &tempSpaceInfo);
		sequencedObjPrim->spaceInfo[spaceIndex] = tempSpaceInfo;
	}

	//attach the object to an instance
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShTreeElement> instanceTree;
	gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, reinterpret_cast<void**>(&instance));

	instance->Set3DObject(object);
	instance->QueryInterface(IID_I3DShTreeElement, (void**) &instanceTree);

	MCAssert(fMasterGroup);
	instanceTree->SetMasterGroup(fMasterGroup);
	instanceTree->SetName(name);

	topTree->InsertLast(instanceTree);

	synchShaders(tempScene, scene, instance);
	return MC_S_OK;
}

void SequencedObjImp::buildName(const TMCDynamicString& fileName, TMCDynamicString& name)
{
	TMCString15 sep;
	gFileUtilities->GetSeparator(sep);

	//name based on file name
	uint32 extensionpos = fileName.FindCharPosFromRight('.');
	uint32 slashpos = fileName.FindCharPosFromRight(sep[0]);
	if (extensionpos == 0)
	{
		extensionpos = fileName.Length() - 1;
	}
	name = fileName;
	name.SubString(slashpos + 1, extensionpos - slashpos - 1);
}

void SequencedObjImp::synchShaders(I3DShScene* sourceScene, I3DShScene* destScene, I3DShInstance* destInstance)
{
	//make sure I have all the master shaders I need
	uint32 shaderCount = sourceScene->GetMasterShadersCount();

	for (uint32 shaderIndex = 0; shaderIndex < shaderCount; shaderIndex++)
	{
		TMCCountedPtr<I3DShMasterShader> sourceShader;
		TMCCountedPtr<I3DShMasterShader> destShader;
		TMCString1023 shaderName;
		sourceScene->GetMasterShaderByIndex(&sourceShader, shaderIndex);
		sourceShader->GetName(shaderName);

		destScene->GetMasterShaderByName(&destShader, shaderName);

		TMCCountedPtr<I3DShShader> shader;
		sourceShader->GetShader(&shader);
		gShell3DUtilities->CreateMasterShader(shader, destScene, &destShader);
		destShader->SetName(shaderName);
	}

	TMCCountedPtr<I3DShInstance> sourceInstance;
	sourceScene->GetInstanceByIndex(&sourceInstance, 0);
	uint32 spaceCount = sourceInstance->GetUVSpaceCount();

	//spin through the shading domains and set the shaders
	for (uint32 shadingDomainIndex = 0; shadingDomainIndex < spaceCount; shadingDomainIndex++)
	{
		TMCCountedPtr<I3DShMasterShader> sourceShader;
		TMCCountedPtr<I3DShMasterShader> destShader;
		TMCString1023 shaderName;

		sourceInstance->GetUVSpaceShader(shadingDomainIndex, &sourceShader);
		sourceShader->GetName(shaderName);

		destScene->GetMasterShaderByName(&destShader, shaderName);
		destInstance->SetUVSpaceShader(shadingDomainIndex, destShader);

	}
}


