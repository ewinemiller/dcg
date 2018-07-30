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
#include "PublicUtilities.h"
#include "AnythingGrowsPrimitive.h"
#include "AnythingGrowsDLL.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "I3DShInstance.h"
#include "MCCountedPtrArray.h" 
#include "I3DShComponentOwner.h"
#include "IShComponent.h"
#include "I3dExPrimitive.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "dcgmeshutil.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"

#if (VERSIONNUMBER != 0x010000)
#include "I3DExVertexPrimitive.h"
#endif

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_AnythingGrowsPrimitive(R_CLSID_AnythingGrowsPrimitive);
const MCGUID IID_AnythingGrowsPrimitive(R_IID_AnythingGrowsPrimitive);
#else
const MCGUID CLSID_AnythingGrowsPrimitive={R_CLSID_AnythingGrowsPrimitive};
const MCGUID IID_AnythingGrowsPrimitive={R_IID_AnythingGrowsPrimitive};
#endif
// I3DExGeometricPrimitive methods
// -- Geometry calls

MCCOMErr AnythingGrowsPrimitive::Grow(uint32 lodIndex, 
				FacetMesh** outMesh, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode) {

	instance = NULL;
	int32 lSelectedDomain = -1;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
	TMCCountedPtr<I3DShTreeElement>	tree;
	TMCCountedPtr<I3DShPrimitive> primitive;
	TMCCountedPtr<FacetMesh> mesh;
	TMCCountedPtr<FacetMesh> originalMesh;
	TMCCountedPtr<I3DShObject> object;
	MCCOMErr retval = MC_S_OK;

	if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) {
		TMCCountedPtr<I3DShPrimitive> prim;
		primcomp->GetPrimitive(&prim);
		primcomp = NULL;
		if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) {
			TMCCountedPtrArray<I3DShInstance> myinstances;
			object->GetScene(&scene);
			if (object->GetInstancesCount() > 0){
				TMCCountedPtr<I3DShTreeElement>	myfirsttree;
				TMCCountedPtr<I3DShInstance> myfirstinstance;
				object->GetInstanceArray(myinstances);
				myfirstinstance = myinstances[0];
				if (myfirstinstance->QueryInterface(IID_I3DShTreeElement, (void**)&myfirsttree)== MC_S_OK)  
				{
					fTransform = myfirsttree->GetGlobalTransform3D(kCurrentFrame);
				}
			}

			object = NULL;
			}
		prim = NULL;
		}

	if (fData.sObjectName.Length() > 0) {
		GetTreeElementByName(tree, fData.sObjectName);
		if (!tree)
		{
			fData.Warnings = "Bad Base Object name";
			UpdateWarnings();
			return MC_S_OK;
		}

		if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
		{
			refinstance = instance;
			instance->GetDeformed3DObject(&object);
			ThrowIfNil(object);
			lBaseSpaceCount = object->GetUVSpaceCount();
			if (fData.sDomain.Length() > 0)
			{
				for (uint32 ldomainindex = 0; ldomainindex < lBaseSpaceCount; ldomainindex++) 
				{
					UVSpaceInfo uvSpaceInfo;				
					object->GetUVSpace(ldomainindex, &uvSpaceInfo);
					if (fData.sDomain == uvSpaceInfo.fName)
					{
						lSelectedDomain = ldomainindex;
						ldomainindex = lBaseSpaceCount;
					}
				}
			}

			object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
			ThrowIfNil(primitive);
			
			primitive->GetFMesh(0.00025000001f, &mesh);
			//primitive->GetFMesh(0, &mesh);
			ThrowIfNil(mesh);

			if (fData.bBoneMode)
			{
				TMCCountedPtr<I3DShObject> baseobject;
				TMCCountedPtr<I3DShObject> newobject;
				TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
				if (instance->Get3DObject(&baseobject)==MC_S_OK) {
					if (getIsFigurePrimitive(object))
					{
						TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
						TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
						TMCCountedPtr<IShComponent> comp;
						TMCCountedPtr<IMCUnknown> extPrimitive;

						object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
						ThrowIfNil(extprimitive);
						extprimitive->GetPrimitiveComponent(&comp);
						ThrowIfNil(comp);
						comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
						ThrowIfNil(primcomp);
						primcomp->GetExternalPrimitive((IMCUnknown**)&extPrimitive);
						object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
						ThrowIfNil(primitive);
						if (extPrimitive->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive)==MC_S_OK) 
						{
							vertexPrimitive->Hydrate();
							vertexPrimitive->GetFacetMesh(0, false, eModelingMeshWithUV, &originalMesh);
							if (
								(originalMesh->fFacets.GetElemCount() != mesh->fFacets.GetElemCount())
								)
							{
								originalMesh = NULL;
							}
							else
							{
								//facets are in a different order than the getfacetmesh
								//so close those facets
								originalMesh->fFacets.SetElemCount(0);
								originalMesh->fFacets.CopyElems(mesh->fFacets, 0, mesh->fFacets.GetElemCount(), 0);
								//create a fake instance to tack this object on to
								TMCCountedPtr<I3DShFacetMeshPrimitive> facetMeshPrimitive;

								gShell3DUtilities->CreatePrimitiveByID(kFacetMeshPrimitiveID, &newobject, IID_I3DShFacetMeshPrimitive, (void**) &facetMeshPrimitive);

								TBBox3D bbox;

								originalMesh->CalcBBox(bbox);
								facetMeshPrimitive->SetNbrLOD(1); // Must be called before 'SetFacetMesh'
								facetMeshPrimitive->SetBoundingBox(bbox);
								
								facetMeshPrimitive->SetFacetMesh(0, 0, originalMesh);

								gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&refinstance);	
								ThrowIfNil(refinstance);
								refinstance->Set3DObject(newobject);

							}
						}

					}
					else
					{
						baseobject->Clone(&newobject, kNoAnim);
						ThrowIfNil(newobject);

						TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
						newobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);

						ThrowIfNil(extprimitive);
						TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
						TMCCountedPtr<IShComponent> comp;
						TMCCountedPtr<IMCUnknown> extPrimitive;
						extprimitive->GetPrimitiveComponent(&comp);
						comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
						ThrowIfNil(primcomp);
						primcomp->GetExternalPrimitive((IMCUnknown**)&extPrimitive);

						newobject->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
						ThrowIfNil(primitive);
						if (extPrimitive->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive)==MC_S_OK) 
						{
							removeBonesAndMorphs(vertexPrimitive);

							primitive->GetFMesh(0.00025000001f, &originalMesh);
							if (
								(originalMesh->fFacets.GetElemCount() != mesh->fFacets.GetElemCount())
								)
							{
								originalMesh = NULL;
							}
							else
							{
								//create a fake instance to tack this object on to
								gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&refinstance);	
								ThrowIfNil(refinstance);
								refinstance->Set3DObject(newobject);

							}
						}
					}
				}
			}
			else 
			if (fData.sReferenceObjectName.Length() > 0)
			{
				GetTreeElementByName(tree, fData.sReferenceObjectName);
				if (tree->QueryInterface(IID_I3DShInstance, (void**)&refinstance)== MC_S_OK) 
				{
					refinstance->GetDeformed3DObject(&object);
					ThrowIfNil(object);

					object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
					ThrowIfNil(primitive);
					
					primitive->GetFMesh(0.00025000001f, &originalMesh);
					ThrowIfNil(originalMesh);
					if (
						(originalMesh->fFacets.GetElemCount() != mesh->fFacets.GetElemCount())
						)
					{
						originalMesh = NULL;
					}
				}

			}
			if (lodIndex == 1) 
			{
				int32 renderhairs = fData.lNumHair;
				switch (fData.lPreviewMode)
				{
					case pmUltraLow: fData.lNumHair /= 1000; break;
					case pmLow: fData.lNumHair /= 100; break;
					case pmMedium: fData.lNumHair /= 10; break;
					case pmHigh: fData.lNumHair /= 5; break;
				}
				if (originalMesh != NULL)
				{
					retval = GrowHair(0, mesh, originalMesh, outMesh, lSelectedDomain, 
						strands, 
						generationMode); 
				}
				else
				{
					retval = GrowHair(0, mesh, mesh, outMesh, lSelectedDomain, 
						strands, 
						generationMode); 
				}
				fData.lNumHair = renderhairs;
			}
			else
			{
				if (originalMesh != NULL)
				{
					retval = GrowHair(0, mesh, originalMesh, outMesh, lSelectedDomain, 
						strands, 
						generationMode); 
				}
				else
				{
					retval = GrowHair(0, mesh, mesh, outMesh, lSelectedDomain, 
						strands, 
						generationMode); 
				}
			}

		}
	}
	else
	{
		fData.Warnings = "No Base Object selected";
		UpdateWarnings();
		return MC_S_OK;
	}
	UpdateWarnings();

	return retval;
}

MCCOMErr AnythingGrowsPrimitive::GetFacetMesh (uint32 lodIndex, FacetMesh** outMesh) {
	//TMCCountedPtr<AnythingGrowsStrands> strands;
	//TMCCountedCreateHelper<AnythingGrowsStrands > result(&strands);
	//result = new AnythingGrowsStrands ();

	//Grow(0, NULL, &strands, GenerateStrands);;

	FacetMesh::Create(outMesh);
	return Grow(lodIndex, outMesh, NULL, GenerateMesh);
}

// Give the boundary Box
void AnythingGrowsPrimitive::GetBoundingBox(TBBox3D& bbox){
		if (bInGetBoundingBox) return;
		bInGetBoundingBox = true;
	try {
		TMCCountedPtr<I3DShTreeElement>	tree;
		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShPrimitive> primitive;
		TMCCountedPtr<FacetMesh> mesh;
		TMCCountedPtr<I3DShObject> object;
		MCCOMErr retval = MC_S_OK;

		bbox.fMin.x = -fData.fLength; 
		bbox.fMin.y = -fData.fLength;
		bbox.fMin.z = -fData.fLength;
		bbox.fMax.x = fData.fLength;
		bbox.fMax.y = fData.fLength;
		bbox.fMax.z = fData.fLength;

		setScene();

		if (fData.sObjectName.Length() > 0) {
			GetTreeElementByName(tree, fData.sObjectName);
			if (!tree)
			{
				fData.Warnings = "bad name";
				UpdateWarnings();
				return;
			}
			fTransform = tree->GetGlobalTransform3D(kCurrentFrame);

			if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
			{
				instance->GetDeformed3DObject(&object);
				ThrowIfNil(object);

				object->GetBoundingBox(bbox);
				bbox.fMin.x -= fData.fLength;
				bbox.fMin.y -= fData.fLength;
				bbox.fMin.z -= fData.fLength;
				bbox.fMax.x += fData.fLength;
				bbox.fMax.y += fData.fLength;
				bbox.fMax.z += fData.fLength;

			}
		}
		//add in the tip object's size
		if (fData.sTipObjectName.Length() > 0) {
			TMCCountedPtr<I3DShObject> object;
			TMCCountedPtr<I3DShTreeElement>	tree;
			TMCCountedPtr<I3DShInstance> instance;
			GetTreeElementByName(tree, fData.sTipObjectName);
			if (!tree)
			{
				fData.Warnings = "bad name";
				UpdateWarnings();
				return;
			} 
			else if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
			{
				TBBox3D tipbox;

				instance->GetDeformed3DObject(&object);
				ThrowIfNil(object);

				object->GetBoundingBox(tipbox);
				real32 tipsize
						, xsize = tipbox.fMax.x - tipbox.fMin.x
						, ysize = tipbox.fMax.y - tipbox.fMin.y
						, zsize = tipbox.fMax.z - tipbox.fMin.z;
				if (xsize >= ysize && xsize >= zsize)
				{
					tipsize = xsize;
				}
				else if (ysize >= xsize && ysize >= zsize)
				{
					tipsize = ysize;
				}
				else
				{
					tipsize = zsize;
				}
				tipsize *= fData.fTipScale;

				bbox.fMin.x -= tipsize;
				bbox.fMin.y -= tipsize;
				bbox.fMin.z -= tipsize;
				bbox.fMax.x += tipsize;
				bbox.fMax.y += tipsize;
				bbox.fMax.z += tipsize;
			}
		}//end tip object length
		bInGetBoundingBox = false;
	}
	catch(...) {
		fData.Warnings = "unknown error";
		UpdateWarnings();
		bInGetBoundingBox = false;
		return;
	}
}

void AnythingGrowsPrimitive::setScene()
{
	if (!scene)
	{
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> prim;
		if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) {
			primcomp->GetPrimitive(&prim);
			primcomp = NULL;
			if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) {
				object->GetScene(&scene);
				object = NULL;
				}
			prim = NULL;
			}
	}
}


  
// -- Shading calls
uint32 AnythingGrowsPrimitive::GetUVSpaceCount	()
{
	TMCCountedPtr<I3DShObject> object;
	TMCCountedPtr<I3DShTreeElement>	tree;
	TMCCountedPtr<I3DShInstance> instance;

	uint32 UVSpaceCount = 1;

	lBaseSpaceCount = 0;
	lTipSpaceCount = 0;

	//setScene();

	//the UV space count is the sum of the base object
	//the strands, and the tip object
	if (scene != NULL)
	{
		if (fData.sObjectName.Length() > 0) {
			GetTreeElementByName(tree, fData.sObjectName);
			if (!tree)
			{
				fData.Warnings = "bad name";
				UpdateWarnings();
			} else
			if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
			{
				instance->GetDeformed3DObject(&object);
				ThrowIfNil(object);
				lBaseSpaceCount = object->GetUVSpaceCount();

			}
		}
		tree = NULL;
		instance = NULL;
		object = NULL;
		if (fData.sTipObjectName.Length() > 0) {
			GetTreeElementByName(tree, fData.sTipObjectName);
			if (!tree)
			{
				fData.Warnings = "bad name";
				UpdateWarnings();
			} else
			if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
			{
				instance->GetDeformed3DObject(&object);
				ThrowIfNil(object);
				lTipSpaceCount = object->GetUVSpaceCount();

			}
		}
		tree = NULL;
		instance = NULL;
		object = NULL;

		UVSpaceCount = UVSpaceCount + lBaseSpaceCount + lTipSpaceCount;
	}
	else if (fData.sObjectName.Length() > 0 || fData.sTipObjectName.Length() > 0) 
	{
		//if there are tip objects we'll wipe the shader assignments if we return 0 for them
		//just put a number there and it will get corrected later.
		UVSpaceCount = 50;
	}

	return UVSpaceCount;
}

MCCOMErr AnythingGrowsPrimitive::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo)
{
	if (uvSpaceID == 0)
	{ 
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = false;
		uvSpaceInfo->fName = "Strands";
		return MC_S_OK;
	}
	else if (!scene)
	{
		//don't have a scene yet, just drop in a placeholder
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = false;
		uvSpaceInfo->fName = "Placeholder";
		return MC_S_OK;
	}
	else if  (uvSpaceID <= lBaseSpaceCount)
	{
		TMCString31 sBase = "Base ";
		uvSpaceInfo->fName.FromInt32(uvSpaceID - 1);
		uvSpaceInfo->fName = sBase + uvSpaceInfo->fName;
		
		//setScene();

		if (scene != NULL)
		{
			if (fData.sObjectName.Length() > 0) {
				TMCCountedPtr<I3DShObject> object;
				TMCCountedPtr<I3DShTreeElement>	tree;
				TMCCountedPtr<I3DShInstance> instance;
				GetTreeElementByName(tree, fData.sObjectName);
				if (!tree)
				{
					fData.Warnings = "bad name";
					UpdateWarnings();
				} else
				if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
				{
					instance->GetDeformed3DObject(&object);
					ThrowIfNil(object);
					return object->GetUVSpace(uvSpaceID - 1, uvSpaceInfo);

				}
			}
		}

		return MC_S_FALSE;
	}
	else if  (uvSpaceID <= lBaseSpaceCount + lTipSpaceCount)
	{
		TMCString31 sBase = "Tip ";
		uvSpaceInfo->fName.FromInt32(uvSpaceID - lBaseSpaceCount - 1);
		uvSpaceInfo->fName = sBase + uvSpaceInfo->fName;

		//setScene();

		if (scene != NULL)
		{
			if (fData.sTipObjectName.Length() > 0) {
				TMCCountedPtr<I3DShObject> object;
				TMCCountedPtr<I3DShTreeElement>	tree;
				TMCCountedPtr<I3DShInstance> instance;
				GetTreeElementByName(tree, fData.sTipObjectName);
				if (!tree)
				{
					fData.Warnings = "bad name";
					UpdateWarnings();
				} else
				if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
				{
					instance->GetDeformed3DObject(&object);
					ThrowIfNil(object);
					return object->GetUVSpace(uvSpaceID - lBaseSpaceCount - 1, uvSpaceInfo);

				}
			}
		}

		return MC_S_FALSE;
	}
	else
	{
		return MC_S_FALSE;
	}
}

MCCOMErr AnythingGrowsPrimitive::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	TMCString255 myname;
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
			parent->FindChildPartByID(&warnings, IDTYPE('W','A','R','N'));
			}
		}

	setScene();
	 
	if ((sourceID == 'LINE')&&(message == EMFPartMessage::kMsg_PartValueChanged )) 
	{
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> prim;
		TMCCountedPtr<I3DShTreeElement>	tree;
		TMCCountedPtrArray<I3DShInstance> instances;
		TTransform3D trans;
		if (fData.sObjectName.Length() > 0) 
		{
			GetTreeElementByName(tree, fData.sObjectName);
			if (tree != NULL)
			{

				trans = tree->GetGlobalTransform3D(kCurrentFrame);

				if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) 
				{
					primcomp->GetPrimitive(&prim);
					primcomp = NULL;
					if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) 
					{
						TMCCountedPtr<I3DShTreeElement>	mytree;
						object->GetInstanceArray(instances);
						instances[0]->QueryInterface(IID_I3DShTreeElement, (void**)&mytree);
						ThrowIfNil(mytree);
						mytree->SetGlobalTransform3D(trans);
						
					}
				}
			} 
		}
	}

	if ((message == kMsg_CUIP_ComponentAttached)||((sourceID == 'RMSL')&&(message == EMFPartMessage::kMsg_PartValueChanged ))) {
		TMCCountedPtr<IMFPart> popuppart;
		//object list
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('O','B','J','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('O','B','J','P'));
			}
		if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) {
			primcomp->GetPrimitive(&prim);
			primcomp = NULL;
			if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) {
 				FillObjectList(object, NULL, popuppart);
				object = NULL;
				}
			prim = NULL;
			}

		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('R','E','F','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('R','E','F','P'));
			}
		if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) {
			primcomp->GetPrimitive(&prim);
			primcomp = NULL;
			if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) {
 				FillObjectList(object, NULL, popuppart);
				object = NULL;
				}
			prim = NULL;
			}

		//tip object list
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('T','O','B','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('T','O','B','P'));
			}
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> prim;
		if (QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp) == MC_S_OK) {
			primcomp->GetPrimitive(&prim);
			primcomp = NULL;
			if (prim->QueryInterface(IID_I3DShObject, (void**)&object) == MC_S_OK) {
 				FillObjectList(object, NULL, popuppart);
				object = NULL;
				}
			prim = NULL;
			}

		//length popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('L','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('L','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lLengthPicker = -1;
		//width popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('W','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('W','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lWidthPicker = -1;
		//stiffness popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('S','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('S','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lStiffnessPicker = -1;

		//x wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('X','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('X','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lXWigglePicker = -1;

		//y wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Y','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Y','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lYWigglePicker = -1;

		//z wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Z','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Z','S','H','P'));
			}
		FillShaderList(popuppart);
		fData.lZWigglePicker = -1;

		//x custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('X','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('X','C','U','P'));
			}
		FillShaderList(popuppart);
		fData.lXCustomVectorPicker = -1;

		//y custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Y','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Y','C','U','P'));
			}
		FillShaderList(popuppart);  
		fData.lYCustomVectorPicker = -1;

		//z custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Z','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Z','C','U','P'));
			}
		FillShaderList(popuppart);
		fData.lZCustomVectorPicker = -1;

		//tip scale popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('T','S','I','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('T','S','I','P'));
			}
		FillShaderList(popuppart);
		fData.lTipScalePicker = -1;

		//tip twist popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('T','T','W','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('T','T','W','P'));
			}
		FillShaderList(popuppart);
		fData.lTipTwistPicker = -1;

		//domain popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('D','O','M','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('D','O','M','P'));
			}

		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShTreeElement>	tree;
		
		if (fData.sObjectName.Length() > 0) 
		{
			GetTreeElementByName(tree, fData.sObjectName);
			if (tree != NULL)
			{
				tree->QueryInterface(IID_I3DShInstance, (void**)&instance);
			} 
		}
		FillDomainList(instance, popuppart);
		fData.lDomainPicker = -1;
	}

	return MC_S_OK;
	}

void AnythingGrowsPrimitive::UpdateWarnings() {
	if (warnings != NULL) {
		warnings->SetValue((void *)&fData.Warnings 
				, kStringValueType
				, true
				, false);
		warnings = NULL;
		}
	}

TMCString255 AnythingGrowsPrimitive::GetMasterShaderName() {
	return fData.MasterShader;
	}

MCCOMErr AnythingGrowsPrimitive::GetStrands(AnythingGrowsStrands** strands) {
	TMCCountedCreateHelper<AnythingGrowsStrands > result(strands);
	result = new AnythingGrowsStrands ();

	return Grow(0, NULL, strands, GenerateStrands);;
}
