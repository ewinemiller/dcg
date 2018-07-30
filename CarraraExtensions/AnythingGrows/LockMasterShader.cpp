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
#include "math.h"
#include "LockMasterShader.h"
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
#include "AnythingGrowsDeformer.h"
#include "AnythingGrowsPrimitive.h"
#include "I3DShObject.h"

#if (VERSIONNUMBER >= 0x030000)
#include "I3DShMasterGroup.h"

#endif
#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_LockMasterShader(R_CLSID_LockMasterShader);
#else
const MCGUID CLSID_LockMasterShader={R_CLSID_LockMasterShader};
#endif
extern IChangeManager*	gChangeManager;

// Constructor of the C++ Object :

LockMasterShader::LockMasterShader()
{
	// Data initialisation

	// Protected variables
	fSceneDocument = NULL;
	fTree = NULL;
	fScene = NULL;
	fSelection = NULL;
}
  
#if (VERSIONNUMBER >= 0x030000)
void LockMasterShader::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
         TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);
         result = new GrowsMenuPreparer;
}


#endif

// TBasicSceneCommand methods :

#if (VERSIONNUMBER >= 0x030000)
boolean GrowsMenuPreparer::SelfPrepareMenu(ISceneDocument* sceneDocument)
#else
boolean LockMasterShader::SelfPrepareMenus(ISceneDocument* sceneDocument)
#endif
{
	if (sceneDocument) {
		return true;
		}
	return false;
}

MCCOMErr LockMasterShader::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;
	sceneDocument -> GetSceneSelection(&fSelection);
	fSelection -> Clone(&fCloneSelection);

	return MC_S_OK;
}


MCCOMErr LockMasterShader::Prepare()
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

boolean LockMasterShader::Do()
{
	ClearTree(fTree);
	DoTree(fTree);

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	fSceneDocument -> GetSceneSelection(&fSelection);
	fSelection->ClearSelection();

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
	return true;
}

boolean LockMasterShader::CanUndo()
{
	return false;
}

boolean LockMasterShader::Undo()
{
	return false;
}

boolean LockMasterShader::Redo()
{
	return false;
}

void LockMasterShader::ClearTree(TMCCountedPtr<I3DShTreeElement> tree) {
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShTreeElement> workingtree;
	TMCCountedPtr<I3DShTreeElement> oldfather;

	TMCString1023 sName;
	TMCString255 sStarter = "Master shader holder for AGro";
	for (tree->GetFirst(&workingtree);workingtree!=NULL;){
		if (workingtree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
			workingtree->GetName(sName);
			sName.SubString(0, 29);
			if (sName == sStarter) {
				TMCCountedPtr<I3DShTreeElement> nextitem;
				workingtree->GetRight(&nextitem);
				workingtree->Unlink(&oldfather);
				workingtree = nextitem;
				}
			else {
				ClearTree(workingtree);
				workingtree->GetRight(&workingtree);
				}
			}
		else {
			ClearTree(workingtree);
			workingtree->GetRight(&workingtree);
		}
		}
	}

void LockMasterShader::DoTree(TMCCountedPtr<I3DShTreeElement> tree) {
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShTreeElement> workingtree;
	for (tree->GetFirst(&workingtree);workingtree!=NULL;workingtree->GetRight(&workingtree)){
		if (workingtree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
			DoTree(workingtree);
			DoItem(workingtree, instance);
			}
		else {
			DoTree(workingtree);
			}
	} 
	}

void LockMasterShader::DoItem(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShInstance> instance) {
	if (instance->GetInstanceKind() == 1/*primitive*/){
		TMCCountedPtr<I3DShTreeElement> newtree;
		TMCCountedPtr<I3DShInstance> newinstance;
		TMCCountedPtr<I3DShObject> object;
		TMCString1023 sName;
		TMCString255 sStarter = "Master shader holder for AGro";
		TMCString255 sAGCount;
		TMCString255 sOn = " on ";
		uint32 modifiercount = tree->GetModifiersCount();
		uint32 i, AGrs = 0;
		TMCCountedPtr<I3DShModifier> modifier;
		TMCCountedPtr<I3DShMasterShader> mastershader;
		TMCCountedPtr<AnythingGrowsDeformer> agrodef;
		TMCCountedPtr<AnythingGrowsPrimitive> agroprim;
		if (instance->Get3DObject(&object)==MC_S_OK) {
			TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
			object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
			ThrowIfNil(extprimitive);
			TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
			TMCCountedPtr<IShComponent> comp;
			TMCCountedPtr<IMCUnknown> extPrimitive;
			extprimitive->GetPrimitiveComponent(&comp);
			comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
			ThrowIfNil(primcomp);
			primcomp->GetExternalPrimitive((IMCUnknown**)&extPrimitive);
			if (extPrimitive->QueryInterface(IID_AnythingGrowsPrimitive,(void**)&agroprim)==MC_S_OK) {
				AnythingGrowsData* fData = static_cast<struct AnythingGrowsData *>(agroprim->GetExtensionDataBuffer());
				
				fScene->GetMasterShaderByName(&mastershader, fData->MasterShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sStiffnessShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sXWiggleShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sYWiggleShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sZWiggleShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sTipScaleShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sTipTwistShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}

				fScene->GetMasterShaderByName(&mastershader, fData->sXCustomVectorShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sYCustomVectorShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sZCustomVectorShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}
				fScene->GetMasterShaderByName(&mastershader, fData->sWidthShader);
				if (mastershader != NULL) {
					AddLock(tree, mastershader, AGrs);
					mastershader = NULL;
					}

			}//end if agroprim
		
		}//end get 3d object

		//spin through each of the modifiers 
		//looking for an anything grooves
		for(i = 0; i < modifiercount; i++) {
			mastershader = NULL;
			if (tree->GetModifierByIndex(&modifier, i) == MC_S_OK) {
				if (modifier->QueryInterface(IID_AnythingGrowsDeformer,(void**)&agrodef)==MC_S_OK) {
					AnythingGrowsData* fData = static_cast<struct AnythingGrowsData *>(agrodef->GetExtensionDataBuffer());
	
					fScene->GetMasterShaderByName(&mastershader, fData->MasterShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sStiffnessShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sXWiggleShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sYWiggleShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sZWiggleShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sTipScaleShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sTipTwistShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sXCustomVectorShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sYCustomVectorShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sZCustomVectorShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
					fScene->GetMasterShaderByName(&mastershader, fData->sWidthShader);
					if (mastershader != NULL) {
						AddLock(tree, mastershader, AGrs);
						mastershader = NULL;
						}
				}//end is agrodef
			}//end get modifier
		}//end spin through modifiers
	}//end is primitive
	}//end function

void LockMasterShader::AddLock(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShMasterShader> mastershader, uint32& AGrs )
{
	TMCCountedPtr<I3DShTreeElement> newtree;
	TMCCountedPtr<I3DShInstance> newinstance;
	TMCCountedPtr<I3DShObject> object;
	TMCString1023 sName;
	TMCString255 sStarter = "Master shader holder for AGro";
	TMCString255 sAGCount;
	TMCString255 sOn = " on ";
	TVector3 scaling(.01f, .01f, .01f);
	scaling.x = (float).01;
	scaling.y = (float).01;
	scaling.z = (float).01;
	AGrs++;
	tree->GetName(sName);
	sAGCount.FromInt32(AGrs);
	sName = sStarter + sAGCount + sOn + sName;
	g3DDBActionServer->CreateObjectByKindAndID(&newtree
					,kRID_GeometricPrimitiveFamilyID, kPlanePrimitiveID, fScene);
	ThrowIfNil(newtree);
	if (newtree->QueryInterface(IID_I3DShInstance, (void**)&newinstance)== MC_S_OK) {
		newinstance->SetShader(mastershader);
		}
	newtree->SetName(sName);
#if (VERSIONNUMBER >= 0x030000)
	TMCCountedPtr<I3DShMasterGroup> masterGroup;
	fScene->QueryInterface(IID_I3DShMasterGroup,(void**)&masterGroup);
	newtree->SetMasterGroup(masterGroup);
#else
	newtree->SetScene(fScene);
#endif
	newtree->SetIsShown(false);
#if (VERSIONNUMBER < 0x050000)
	newtree->SetExists(false);
#endif
	newtree->SetXYZScaling(scaling);
	tree->InsertLast(newtree);
	tree->PostMoveChange();
}