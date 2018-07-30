/*  Project Gemini - plug-in for Carrara
    Copyright (C) 2002 Eric Winemiller

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
#include "copyright.h"
#include "math.h"
#include "GeminiCommand.h"
#include "I3DShGroup.h"
#include "I3DShModifier.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#include "I3DShUtilities.h"
#include "com3dutilities.h"
#include "IComponentAnim.h"
#include "I3DShComponentOwner.h"
#include "I3DShObject.h"
#include "IShComponent.h"
#include "I3DShShader.h"
#include "I3DExPrimitive.h"
#include "GeminiDLL.h"
#include "I3DExVertexPrimitive.h"

#include "GeminiDeformer.h"
#include "copyright.h"
#if (VERSIONNUMBER >= 0x030000)
#include "I3DShMasterGroup.h"
#endif
#if (VERSIONNUMBER >= 0x050000)
#include "COMSafeUtilities.h"
#endif

#include "IMFDocument.h"
#include "I3dShModule.h"

extern IChangeManager*	gChangeManager;

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GeminiCommand(R_CLSID_GeminiCommand);
#else
const MCGUID CLSID_GeminiCommand={R_CLSID_GeminiCommand};
#endif

enum Keep {kXPositive = 0, kXNegative = 1, kYPositive = 2, kYNegative = 3, kZPositive = 4, kZNegative = 5};

// Constructor of the C++ Object :

GeminiCommandData	LastChanges;
boolean	bLastChangesInitialized = false;

void SetDefaults(GeminiCommandData &fData) {
	fData.lKeep = kZNegative;
	fData.fMargin = 0.10f;
	fData.bKeepOriginal = true;
	}


GeminiCommand::GeminiCommand()
{
	// Data initialisation
	SetDefaults(fData);
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
void GeminiCommand::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
         TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);
         result = new GeminiMenuPreparer;
}


#endif

// TBasicSceneCommand methods :
#if (VERSIONNUMBER >= 0x030000)
boolean GeminiMenuPreparer::SelfPrepareMenu(ISceneDocument* sceneDocument)
#else
boolean GeminiCommand::SelfPrepareMenus(ISceneDocument* sceneDocument)
#endif
{
	//enable only if we find just one primitive
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

			if (instance->GetInstanceKind() == 1/*primitive*/)
			{
				return true;
			}
		}
		
	}
	return false;
}

MCCOMErr GeminiCommand::Init(ISceneDocument* sceneDocument)
{
	//save off the stuff we will need later
	fSceneDocument = sceneDocument;
	sceneDocument -> GetSceneSelection(&fSelection);
	fSelection -> Clone(&fCloneSelection);

	//grab the settings from last round
	fData = LastChanges;

	//if there is a gemini on this item then override the last settings with the current gemini settings
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShModifier> modifier;
	TMCCountedPtr<GeminiDeformer> gemini;
	uint32 lModifierCount;
	uint32 lCurrentModifier;

	TTreeSelectionIterator iter(fSelection);
	tree = iter.First();
	lModifierCount = tree->GetModifiersCount();
	for(lCurrentModifier = 0; lCurrentModifier < lModifierCount; lCurrentModifier++)
	{
		if (tree->GetModifierByIndex(&modifier, lCurrentModifier) == MC_S_OK)
		{
			if (modifier->QueryInterface(IID_GeminiDeformer,(void**)&gemini)==MC_S_OK) 
			{
				GeminiData* geminidata;
				geminidata = static_cast<GeminiData*>(gemini->GetExtensionDataBuffer());
				fData.lKeep = geminidata->lKeep;
				fData.fMargin = geminidata->fMargin;
			}
		}
	}

	return MC_S_OK;
}


MCCOMErr GeminiCommand::Prepare()
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

boolean GeminiCommand::Do()
{
	LastChanges = fData;

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCString255 name;
	TMCString255 newname = "Gemini Version of ";

	TTreeSelectionIterator iter(fSelection);
	tree = iter.First();
	TMCCountedPtr<I3DShInstance> instance;
	if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) {
		TMCCountedPtr<I3DShTreeElement> newtree;
		TMCCountedPtr<I3DShTreeElement> parent;
		TMCCountedPtr<I3DShObject> newobject;
		TMCCountedPtr<I3DShFacetMeshPrimitive> facetMeshPrimitive;
		TMCCountedPtr<FacetMesh> mesh;
		TMCCountedPtr<FacetMesh> newmesh;
		TMCCountedPtr<FacetMesh> geminimesh;

		TMCCountedPtr<I3DShInstance> newinstance;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> primitive;
		GeminiDeformer gemini;
		GeminiData* geminidata;

		TMCCountedPtr<I3DShMasterShader> shader;
		TVector3 hotpoint;
		TTransform3D transform;
		TMCCountedPtr<I3DShScene> scene;
		TBBox3D boundingbox;

		//get the name
		tree->GetName(name);
		//if (fData.bKeepOriginal)
		//{
		//	newname = newname + name;
		//}
		//else
		//{
			newname = name;
		//}
		//get the mesh
		instance->GetDeformed3DObject(&object);
		
		object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
		ThrowIfNil(primitive);
		
		primitive->GetFMesh(.01f, &mesh);
		ThrowIfNil(mesh);

		mesh->Clone(&newmesh);
		ThrowIfNil(newmesh);

		
		geminidata = static_cast<GeminiData*>(gemini.GetExtensionDataBuffer());
		
		geminidata->bEnabled = true;
		geminidata->lKeep = fData.lKeep;
		geminidata->fMargin = fData.fMargin;

		gemini.DeformFacetMesh(0,newmesh, &geminimesh);
		
		newmesh = NULL;

		//create the new mesh
		gShell3DUtilities->CreatePrimitiveByID(kFacetMeshPrimitiveID, &newobject 
			, IID_I3DShFacetMeshPrimitive, (void**) &facetMeshPrimitive);
		ThrowIfNil(facetMeshPrimitive);

		geminimesh->CalcBBox(boundingbox);
		facetMeshPrimitive->SetNbrLOD(1);
		facetMeshPrimitive->SetBoundingBox(boundingbox);
		facetMeshPrimitive->SetFacetMesh(0, 0, geminimesh);

		//I3DShObject*  oldMaster= ...

		TMCCountedPtr<I3DShObject> newMaster;
		//Create new vertex object
		TMCCountedPtr<IMCUnknown> dummy;
		gShell3DUtilities->CreatePrimitiveByID('vmda', &newMaster, MCGUID_NULL, (void**) &dummy);

		//Import facet mesh in vertex prim
		TMCCountedPtr<I3DShExternalPrimitive> externalPrimitive;
		newMaster->QueryInterface(IID_I3DShExternalPrimitive, (void**)&externalPrimitive);
		if (MCVerify(externalPrimitive))
		{
				TMCCountedPtr<IShComponent> component;
				externalPrimitive->GetPrimitiveComponent(&component);
				if (MCVerify(component))
				{
						TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
						component->QueryInterface(IID_I3DExVertexPrimitive, (void**)&vertexPrimitive);
						if (MCVerify(vertexPrimitive))
						{
								vertexPrimitive->DoImportAlien(newobject, 10.0f, false, 0); //you may want to change the fidelity
								gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL,
									MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**) &newinstance);

								newinstance->QueryInterface(IID_I3DShTreeElement, (void**) &newtree);
								ThrowIfNil(newtree);

								newMaster->SetName(newname);
								newinstance->Set3DObject(newMaster);
								//copy shader
								instance->GetShader(&shader);
								newinstance->SetShader(shader);
								//copy transform
#if (VERSIONNUMBER >= 0x040000 )
								transform = tree->GetGlobalTransform3D();
#else
								tree->GetGlobalTransform3D(transform);
#endif
								newtree->SetGlobalTransform3D(transform);
								//copy scene
								tree->GetScene(&scene);
#if (VERSIONNUMBER >= 0x030000)
								TMCCountedPtr<I3DShMasterGroup> masterGroup;
								scene->QueryInterface(IID_I3DShMasterGroup,(void**)&masterGroup);
								newtree->SetMasterGroup(masterGroup);
#else
								newtree->SetScene(scene);
#endif
								//copy hotpoint
								tree->GetHotPoint(hotpoint);
								newtree->SetHotPoint(hotpoint);
								//insert it into the tree
								tree->GetTop(&parent);
								tree->InsertRight(newtree);
								if (fData.bKeepOriginal == false)
								{
									TMCCountedPtr<I3DShTreeElement> oldfather;
									tree->Unlink(&oldfather);
								}
								gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

								fSceneDocument -> GetSceneSelection(&fSelection);
								fSelection->ClearSelection();

								ISelectableObject* newselection = NULL;

								if (newtree->QueryInterface(IID_ISelectableObject, (void**)&newselection)== MC_S_OK) {
									newselection ->AddToSelection(fSelection, true);
									}

								gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

						}
				}
		}	
	

	}
	
	return true;
}

boolean GeminiCommand::CanUndo()
{
	return false;
}

boolean GeminiCommand::Undo()
{
	return false;
}

boolean GeminiCommand::Redo()
{
	return false;
}

void* GeminiCommand::GetExtensionDataBuffer()
{
	return &fData; // used by the shell to set the new parameters
}
  
short GeminiCommand::GetResID()
{
	return 320; // this is the view ID in the resource file.
}


// For the component chooser
MCCOMErr GeminiCommand::ExtensionDataChanged()
{
	return MC_S_OK;
}

MCCOMErr GeminiCommand::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart>	sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if ((sourceID == 'LRUN')&&(message == 3)) {
		fData = LastChanges;
		}
	else if((sourceID == 'DEFT')&&(message == 3)) {
		SetDefaults(fData);
		}
		
	return MC_S_OK;
	}