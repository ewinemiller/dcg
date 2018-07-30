/*  Shader Ops - plug-in for Carrara
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
//#include "math.h"
#include "GroupShader.h"
#include "I3DShGroup.h"
#include "I3DShModifier.h"
#include "IComponentAnim.h"
#include "IMFTextPopupPart.h" 
#include "I3DShComponentOwner.h"
#include "IShComponent.h"
#include "ShaderOpsDLL.h"
#include "I3DShShader.h"
#include "commessages.h"
#include "I3DShInstance.h"

extern IChangeManager*	gChangeManager;

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GroupShader(R_CLSID_GroupShader);
#else
const MCGUID CLSID_GroupShader={R_CLSID_GroupShader};
#endif


// Constructor of the C++ Object :

GroupShader::GroupShader()
{
	// Data initialisation
	fData.acShaderIndex = -1;
	
	// Protected variables
	fSceneDocument = NULL;
	fTree = NULL;
	fScene = NULL;
	fSelection = NULL;
}
  
void* GroupShader::GetExtensionDataBuffer()
{
	return &fData; // used by the shell to set the new parameters
}
  
short GroupShader::GetResID()
{
	return 204; // this is the view ID in the resource file.
}


// For the component chooser
MCCOMErr GroupShader::ExtensionDataChanged()
{
	return MC_S_OK;
}
                                                                            
#if (VERSIONNUMBER >= 0x030000)
void GroupShader::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
         TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);
         result = new GroupShaderMenuPreparer;
}


#endif

// TBasicSceneCommand methods :

#if (VERSIONNUMBER >= 0x030000)
boolean GroupShaderMenuPreparer::SelfPrepareMenu(ISceneDocument* sceneDocument)
#else
boolean GroupShader::SelfPrepareMenus(ISceneDocument* sceneDocument)
#endif

{
	//enable only if we find a group or light
	TMCCountedPtr<ISceneSelection>		selection;
	if (sceneDocument)	{
		sceneDocument->GetSceneSelection(&selection);
		TTreeSelectionIterator iter(selection);
		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShTreeElement> tree;

		uint32 lSelectedElements = iter.GetElemCount();
		uint32 i;
		for(tree=iter.First(), i = 0; i < lSelectedElements; tree=iter.Next(), i++){
			if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
				if (instance->GetInstanceKind() == 1){
					return true;
					}
			}
			else{
				//found a group, it's true
				return true;
				}
			}
		}
	return false;
}

MCCOMErr GroupShader::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;
	sceneDocument -> GetSceneSelection(&fSelection);
	fSelection -> Clone(&fCloneSelection);

	return MC_S_OK;


	}
MCCOMErr GroupShader::Prepare()
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

boolean GroupShader::Do()
{
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShInstance> instance;
	lShadersApplied = 0;
	TTreeSelectionIterator iter(fSelection);

	uint32 lSelectedElements = iter.GetElemCount();
	uint32 i;
	for(tree=iter.First(), i = 0; i < lSelectedElements; tree=iter.Next(), i++)
	{
		if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
			DoItem(tree, instance);
			}
		else {
			//it's a group, apply the settings to it's children
			DoTree(tree);
			}

		}
	TTreeSelectionIterator selectionIter(fSelection);
	selectionIter = TTreeSelectionIterator(fSelection);
	lSelectedElements = iter.GetElemCount();
	for(tree=iter.First(), i = 0; i < lSelectedElements; tree=iter.Next(), i++)
	{
		tree->PostMoveChange(); // here, we make sure that this tree element is in the triple buffer
	}

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	return true;
}

boolean GroupShader::CanUndo()
{
	return false;
}

boolean GroupShader::Undo()
{
	return false;
}

boolean GroupShader::Redo()
{
	return false;
}

void GroupShader::DoTree(TMCCountedPtr<I3DShTreeElement> tree) {
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShTreeElement> workingtree;
	for (tree->GetFirst(&workingtree);workingtree!=NULL;workingtree->GetRight(&workingtree)){
		if (workingtree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
			DoItem(workingtree, instance);
			}
		else {
			DoTree(workingtree);
			}
		}
	}

void GroupShader::DoItem(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShInstance> instance) 
{

	TMCCountedPtr<I3DShComponentOwner> comp;
	TMCCountedPtr<IComponentAnim> anim;
	TMCCountedPtr<IShParameterComponent> param;
	TMCCountedPtr<I3DShMasterShader> mastershader;

	TMCCountedPtr<I3DShMasterShader> currentshader;

	fScene->GetMasterShaderByIndex(&mastershader, fData.acShaderIndex);
	if (instance->GetInstanceKind() == 1)
	{
		instance->GetShader(&currentshader);
		if (currentshader != mastershader)
		{
			instance->SetShader(mastershader);
			lShadersApplied++;
		}

	}//it's an object
}

MCCOMErr GroupShader::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart>	sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if (message == kMsg_CUIP_ComponentAttached)
	{
		TMCCountedPtr<IMFPart> popuppart;
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('M','S','H','D'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('M','S','H','D'));
			}
		FillShaderList(popuppart);
		fData.acShaderIndex = -1;
	
	}
		
	return MC_S_OK;
	}

void GroupShader::FillShaderList (TMCCountedPtr<IMFPart> popuppart)
{
	TMCCountedPtr<IMFTextPopupPart> popup;
	TMCString255 name = "(Clear)";

	popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
	popup->RemoveAll();

	if (fScene != NULL) 
	{
		TMCCountedPtr<I3DShMasterShader> mastershader;
		uint32 numshaders = fScene->GetMasterShadersCount();
		uint32 nummenu = 0;
		for (uint32 shaderindex = 0; shaderindex < numshaders; shaderindex++) 
		{
			fScene->GetMasterShaderByIndex(&mastershader, shaderindex);
			mastershader->GetName(name);
			mastershader = NULL;
			popup->AppendMenuItem(name);
			popup->SetItemActionNumber(nummenu, shaderindex);
			nummenu++;
		}
	} 
}

