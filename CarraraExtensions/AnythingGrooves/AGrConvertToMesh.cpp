/*  Anything Grooves - plug-in for Carrara
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
#include "AGrConvertToMesh.h"
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
//#include "DefMap.h"
#include "I3DExPrimitive.h"
#if (VERSIONNUMBER >= 0x030000)
#include "I3DShMasterGroup.h"
#endif
#if (VERSIONNUMBER >= 0x050000)
#include "COMSafeUtilities.h"
#endif

extern IChangeManager*	gChangeManager;
#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_AGrConvertToMesh(R_CLSID_AGrConvertToMesh);
#else
const MCGUID CLSID_AGrConvertToMesh={R_CLSID_AGrConvertToMesh};
#endif

// Constructor of the C++ Object :

ConvertToMesh::ConvertToMesh()
{
	// Data initialisation

	// Protected variables
	fSceneDocument = NULL;
	fTree = NULL;
	fScene = NULL;
	fSelection = NULL;
}
  
#if (VERSIONNUMBER >= 0x030000)
void ConvertToMesh::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
         TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);
         result = new ConvertToMeshMenuPreparer;
}


#endif

// TBasicSceneCommand methods :

#if (VERSIONNUMBER >= 0x030000)
boolean ConvertToMeshMenuPreparer::SelfPrepareMenu(ISceneDocument* sceneDocument)
#else
boolean ConvertToMesh::SelfPrepareMenus(ISceneDocument* sceneDocument)
#endif
{
	//enable only if we find just one primitive
	TMCCountedPtr<ISceneSelection>		selection;
	if (sceneDocument)
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

MCCOMErr ConvertToMesh::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;
	sceneDocument -> GetSceneSelection(&fSelection);
	fSelection -> Clone(&fCloneSelection);

	return MC_S_OK;
}


MCCOMErr ConvertToMesh::Prepare()
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

boolean ConvertToMesh::Do()
{
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCString255 name;
	TMCString255 newname = "Mesh Version of ";

	TTreeSelectionIterator iter(fSelection);
	tree=iter.First();
	TMCCountedPtr<I3DShInstance> instance;
	if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
	{
		TMCCountedPtr<I3DShTreeElement> newtree;
		TMCCountedPtr<I3DShTreeElement> parent;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShObject> newobject;
		TMCCountedPtr<I3DShFacetMeshPrimitive> facetMeshPrimitive;
		TMCCountedPtr<FacetMesh> mesh;
		TMCCountedPtr<FacetMesh> newmesh;

		TMCCountedPtr<I3DShInstance> newinstance;
		TMCCountedPtr<I3DShPrimitive> primitive;

		TMCCountedPtr<I3DShMasterShader> shader;
		TVector3 hotpoint;
		TTransform3D transform;
		TMCCountedPtr<I3DShScene> scene;
		TBBox3D boundingbox;

		//get the name
		tree->GetName(name);
		newname = newname + name;
		//get the mesh
		instance->GetDeformed3DObject(&object);
		
		object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
		ThrowIfNil(primitive);
		
		primitive->GetFMesh(.01f, &mesh);
		ThrowIfNil(mesh);

		mesh->Clone(&newmesh);
		ThrowIfNil(newmesh);

		//create the new mesh
		gShell3DUtilities->CreatePrimitiveByID(kFacetMeshPrimitiveID, &newobject 
			, IID_I3DShFacetMeshPrimitive, (void**) &facetMeshPrimitive);
		ThrowIfNil(facetMeshPrimitive);
		newobject->SetName(newname);

		newmesh->CalcBBox(boundingbox);
		facetMeshPrimitive->SetNbrLOD(1);
		facetMeshPrimitive->SetBoundingBox(boundingbox);
		facetMeshPrimitive->SetFacetMesh(0, 0, newmesh);

		gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL,
			MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**) &newinstance);

		newinstance->QueryInterface(IID_I3DShTreeElement, (void**) &newtree);
		ThrowIfNil(newtree);

		newinstance->Set3DObject(newobject);
		//copy shader
		instance->GetShader(&shader);
		newinstance->SetShader(shader);
		//copy transform
#if (VERSIONNUMBER >= 0x040000)
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
	}

	
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	fSceneDocument -> GetSceneSelection(&fSelection);
	fSelection->ClearSelection();
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
	return true;
}

boolean ConvertToMesh::CanUndo()
{
	return false;
}

boolean ConvertToMesh::Undo()
{
	return false;
}

boolean ConvertToMesh::Redo()
{
	return false;
}

