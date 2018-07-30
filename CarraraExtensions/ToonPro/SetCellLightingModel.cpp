/*  Toon! Pro - plug-in for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#include "SetCellLightingModel.h"
#include "math.h"

#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif

#include "I3DShGroup.h"
#include "I3DShModifier.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#include "I3DShUtilities.h"
#include "com3dutilities.h"
#include "IComponentAnim.h"
#include "I3DShComponentOwner.h"
#include "IShComponent.h"
#include "I3DShShader.h"
#include "I3DShObject.h"
#include "I3DShInstance.h"
#include "MFPartMessages.h"
#include "IMFPart.h"


#if (VERSIONNUMBER >= 0x030000)
#include "I3DShMasterGroup.h"
#endif

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_SetCellLightingModel(R_CLSID_SetCellLightingModel);
#else
const MCGUID CLSID_SetCellLightingModel={R_CLSID_SetCellLightingModel};
#endif
extern IChangeManager*	gChangeManager;

SetCellLightingModelPublicData LastChanges;
boolean	bLastChangesInitialized = false;

void SetDefaults(SetCellLightingModelPublicData &fData) 
{
	fData.vec2ShadowHighlight.x = .50f;
	fData.vec2ShadowHighlight.y = .75f;
	fData.fShadowBrightness = .30f;
	fData.lLevels = 1;
}


SetCellLightingModel::SetCellLightingModel()
{
	// Data initialisation
	SetDefaults(fData);
	// fData variables
	if (!bLastChangesInitialized) {
		LastChanges = fData;
		bLastChangesInitialized = true;
		}

	// Protected variables
	fSceneDocument = NULL;
	fTree = NULL;
	fScene = NULL;
	fSelection = NULL;
}
  
#if (VERSIONNUMBER >= 0x030000)
void SetCellLightingModel::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
         TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);
         result = new SetCellLightingModelMenuPreparer;
}


#endif

// TBasicSceneCommand methods :

#if (VERSIONNUMBER >= 0x030000)
boolean SetCellLightingModelMenuPreparer::SelfPrepareMenu(ISceneDocument* sceneDocument)
#else
boolean SetCellLightingModel::SelfPrepareMenus(ISceneDocument* sceneDocument)
#endif
{
	TMCCountedPtr<ISceneSelection>		selection;
	//if (sceneDocument)
	{
		sceneDocument->GetSceneSelection(&selection);
		TTreeSelectionIterator iter(selection);
		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShTreeElement> tree;
		if (!iter.IsElemCountEqualTo(1)){
			return false;
			}
		tree=iter.First();
		if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
		{

			if (instance->GetInstanceKind() == I3DShInstance::kPrimitiveInstance)
			{
				return true;
			}
		}
		
	}
	return false;
}

MCCOMErr SetCellLightingModel::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;
	sceneDocument -> GetSceneSelection(&fSelection);
	fSelection -> Clone(&fCloneSelection);

	return MC_S_OK;
}


MCCOMErr SetCellLightingModel::Prepare()
{
	if (fSceneDocument)
	{
		fSceneDocument->GetScene(&fScene);

		fScene->GetTreePropertyChangeChannel(&fTreePropertyChannel);
		ThrowIfNil(fTreePropertyChannel);

		fSceneDocument->GetSceneSelectionChannel(&fSelectionChannel);
		ThrowIfNil(fSelectionChannel);
	}

	if (!fTree)
	{
		TMCCountedPtr<I3DShGroup> group;
		fScene->GetTreeRoot(&group);
		if (group->QueryInterface(IID_I3DShTreeElement, (void **) &fTree) != MC_S_OK)
			return MC_S_OK;
	}
	return MC_S_OK;
}

void SetCellLightingModel::ChangeLightingModel(I3DShMasterShader* masterShader)
{
	TMCCountedPtr<I3DShShader> shader;
	TMCCountedPtr<I3DShShader> cellShader;
	TMCCountedPtr<CelShader> cell;
	TMCCountedPtr<IShComponent> component;

	masterShader->GetShader(&shader);
	ThrowIfNil(shader);
	shader->QueryInterface(IID_Cel, (void**) &cell);
	//if there is already a cell lighting model, just update it's values
	if (cell)
	{
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->fShadowBrightness = fData.fShadowBrightness;
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->lLevels = fData.lLevels;
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->vec2ShadowHighlight = fData.vec2ShadowHighlight;
	}
	//otherwise set a new cell as the top most shader
	else
	{
		gComponentUtilities->CreateComponent(kRID_ShaderFamilyID, 'SPCE', &component);
		ThrowIfNil(component);
		component->QueryInterface(IID_I3DShShader, (void**) &cellShader);
		ThrowIfNil(cellShader);
		cellShader->QueryInterface(IID_Cel, (void**) &cell);
		ThrowIfNil(cell);

		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->fShadowBrightness = fData.fShadowBrightness;
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->lLevels = fData.lLevels;
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->vec2ShadowHighlight = fData.vec2ShadowHighlight;
		
		TMCCountedPtr<IShParameterComponent> parameterComponent;
		shader->QueryInterface(IID_IShParameterComponent, (void**)&parameterComponent);
		((CelPublicData*)(cell.fObject->GetExtensionDataBuffer()))->param = parameterComponent;

		cell.fObject->ExtensionDataChanged();
		masterShader->SetShader(cellShader, kWithAnim);
	}

}

boolean SetCellLightingModel::Do()
{
	LastChanges = fData;

	//do stuff
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShMasterShader> masterShader;

	TTreeSelectionIterator iter(fSelection);
	tree = iter.First();
	ThrowIfNil(tree);
	tree->QueryInterface(IID_I3DShInstance, (void**)&instance);
	ThrowIfNil(instance);
	instance->GetShader(&masterShader);
	ThrowIfNil(masterShader);
	ChangeLightingModel(masterShader);

	uint32 uvSpaceCount = instance->GetUVSpaceCount();
	for (uint32 uvSpaceIndex = 0; uvSpaceIndex < uvSpaceCount; uvSpaceIndex++)
	{
		instance->GetUVSpaceShader(uvSpaceIndex, &masterShader);
		if (masterShader)
		{
			ChangeLightingModel(masterShader);
		}
	}
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
	fSceneDocument -> GetSceneSelection(&fSelection);
	fSelection->ClearSelection();

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
	return true;
}

boolean SetCellLightingModel::CanUndo()
{
	return false;
}

boolean SetCellLightingModel::Undo()
{
	return false;
}

boolean SetCellLightingModel::Redo()
{
	return false;
}

int16 SetCellLightingModel::GetResID()

{
	return 400;
}	

void* SetCellLightingModel::GetExtensionDataBuffer()
{
	return &fData;;
}

MCCOMErr SetCellLightingModel::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart>	sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if (sourceID == 'LRUN' && message == EMFPartMessage::kMsg_PartValueChanged) 
	{
		fData = LastChanges;
	}
	else if(sourceID == 'DEFT' && message == EMFPartMessage::kMsg_PartValueChanged) 
	{
		SetDefaults(fData);
	}

	return MC_S_OK;
}