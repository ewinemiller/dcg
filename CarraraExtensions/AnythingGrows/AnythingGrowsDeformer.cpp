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
#include "AnythingGrowsDeformer.h"
#include "AnythingGrowsDLL.h"
#include "I3DShComponentOwner.h"
#include "IShComponent.h"
#include "I3dExPrimitive.h"
#include "copyright.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "dcgmeshutil.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"

#if (VERSIONNUMBER != 0x010000)
#include "I3DExVertexPrimitive.h"
#endif
#include <stdlib.h>

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_AnythingGrowsDeformer(R_CLSID_AnythingGrowsDeformer);
const MCGUID IID_AnythingGrowsDeformer(R_IID_AnythingGrowsDeformer);
#else
const MCGUID CLSID_AnythingGrowsDeformer={R_CLSID_AnythingGrowsDeformer};
const MCGUID IID_AnythingGrowsDeformer={R_IID_AnythingGrowsDeformer};
#endif

void HidePart(TMCCountedPtr<IMFPart> sourcePart, IDType sourceID, IDType hideID)
{
	TMCCountedPtr<IMFPart> hidePart;
	if (sourceID == 0) {
		sourcePart->FindChildPartByID(&hidePart, hideID);
		}
	else {
		TMCCountedPtr<IMFPart> parentPart;
		sourcePart->GetPartParent(&parentPart);
		parentPart->FindChildPartByID(&hidePart, hideID);
		}
	if (hidePart != NULL)
	{
		hidePart->SetShown(false, true);
	}

}


MCCOMErr AnythingGrowsDeformer::QueryInterface(const MCIID& riid, void** ppvObj)
{
 
	if (MCIsEqualIID(riid, IID_AnythingGrowsDeformer))
	{ 
		TMCCountedGetHelper<AnythingGrowsDeformer> result(ppvObj);
		result = (AnythingGrowsDeformer*)this;
		return MC_S_OK;
	}
	return TBasicDeformModifier::QueryInterface(riid, ppvObj);
}
  
MCCOMErr AnythingGrowsDeformer::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh) 
{
	TMCCountedPtr<I3DShTreeElementMember> Member;
	TMCCountedPtr<FacetMesh> originalMesh;
	TMCCountedPtr<FacetMesh> inMesh;
	TMCCountedPtr<I3DShInstance> bonedInstance;

	inMesh = in;

	int32 lSelectedDomain = -1;

	if (TBasicModifier::QueryInterface(IID_I3DShTreeElementMember, (void**)&Member) == MC_S_OK) 
	{
		TMCCountedPtr<I3DShObject> object;

		Member->GetTreeElement(&tree);
		ThrowIfNil(tree);
#if (VERSIONNUMBER >= 0x040000 )	
		fTransform = tree->GetGlobalTransform3D(kCurrentFrame);
#else
		tree->GetGlobalTransform3D(fTransform);
#endif
		tree->GetScene(&scene);
		tree->QueryInterface(IID_I3DShInstance, (void**) &instance);
		if (instance)
		{
			//we're an object
			instance->Get3DObject(&object);

			ThrowIfNil(object);

			lBaseSpaceCount = object->GetUVSpaceCount();
		}
		else//we're a group
		{
			//if it's a group, they can't select UV space
		}
		refinstance = instance;

#if (VERSIONNUMBER != 0x010000)
		if (fData.bBoneMode)
		{
#if VERSIONNUMBER >= 0x060000
			if (getIsFigurePrimitive(object))
			{
				TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
				TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
				TMCCountedPtr<IShComponent> comp;
				TMCCountedPtr<IMCUnknown> extPrimitive;
				TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;

				object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
				ThrowIfNil(extprimitive);
				extprimitive->GetPrimitiveComponent(&comp);
				ThrowIfNil(comp);
				comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
				ThrowIfNil(primcomp);
				primcomp->GetExternalPrimitive((IMCUnknown**)&extPrimitive);
				if (extPrimitive->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive)==MC_S_OK) 
				{
					vertexPrimitive->Hydrate();
					vertexPrimitive->GetFacetMesh(0, false, eModelingMeshWithUV, &originalMesh);
					if (
						(originalMesh->fFacets.GetElemCount() != inMesh->fFacets.GetElemCount())
						)
					{
						originalMesh = NULL;
					}
					else
					{
						TMCCountedPtr<I3DShObject> newobject;
						//facets are in a different order than the getfacetmesh
						//so close those facets
						originalMesh->fFacets.SetElemCount(0);
						originalMesh->fFacets.CopyElems(inMesh->fFacets, 0, inMesh->fFacets.GetElemCount(), 0);
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
#endif
			{
				TMCCountedPtr<I3DShObject> newobject;
				TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
				TMCCountedPtr<I3DShPrimitive> primitive;
				TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
				TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
				TMCCountedPtr<IShComponent> comp;
				TMCCountedPtr<IMCUnknown> extPrimitive;

				object->Clone(&newobject, kNoAnim);
				ThrowIfNil(newobject);

				newobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
				ThrowIfNil(extprimitive);

				extprimitive->GetPrimitiveComponent(&comp);
				ThrowIfNil(comp);

				comp->QueryInterface(IID_I3DShPrimitiveComponent, (void**) &primcomp);
				ThrowIfNil(primcomp);

				primcomp->GetExternalPrimitive((IMCUnknown**)&extPrimitive);
				ThrowIfNil(extPrimitive);

				newobject->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
				ThrowIfNil(primitive);

				if (extPrimitive->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive)==MC_S_OK) 
				{

					removeBonesAndMorphs(vertexPrimitive);

					primitive->GetFMesh(lod, &originalMesh);
					if (originalMesh->fFacets.GetElemCount() != inMesh->fFacets.GetElemCount())
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
		}//end is bone mode
		else
#endif	
		if (fData.sReferenceObjectName.Length() > 0)
		{
			TMCCountedPtr<I3DShTreeElement>	reftree;
			TMCCountedPtr<I3DShPrimitive> refprimitive;
			TMCCountedPtr<I3DShObject> refobject;

			GetTreeElementByName(reftree, fData.sReferenceObjectName);
			//if it's a bad name we don't want to raise an error, just keep going.
			if (reftree != NULL)
			{
				reftree->QueryInterface(IID_I3DShInstance, (void**) &refinstance);
				ThrowIfNil(refinstance);

				refinstance->Get3DObject(&refobject);
				ThrowIfNil(refobject);

				refobject->QueryInterface(IID_I3DShPrimitive, (void**) &refprimitive);
				ThrowIfNil(refprimitive);
				
				refprimitive->GetFMesh(lod, &originalMesh);
				ThrowIfNil(originalMesh);

				if (originalMesh->fFacets.GetElemCount() != in->fFacets.GetElemCount())
				{
					originalMesh = NULL;
				}
			}

		}
			
		if (fData.sDomain.Length() > 0)
		{
			lSelectedDomain = 0;
				
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
		

	}
	MCCOMErr retVal;
	FacetMesh::Create(outMesh);

	if (originalMesh != NULL)
	{
		retVal = GrowHair(lod, inMesh, originalMesh, outMesh, lSelectedDomain, 
						NULL, 
						GenerateMesh); 
	}
	else
	{
		retVal = GrowHair(lod, inMesh, inMesh, outMesh, lSelectedDomain, 
						NULL, 
						GenerateMesh); 
	}

	bonedInstance = NULL;

	return retVal;

}



MCCOMErr AnythingGrowsDeformer::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	TMCCountedPtr<IMFPart> hidePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if ((sourceID == 'REFR')&&(message == EMFPartMessage::kMsg_PartValueChanged)) {
		fData.bRefresh = !fData.bRefresh;
		}

	if ((message == kMsg_CUIP_ComponentAttached)||((sourceID == 'ADVA')&&(message == EMFPartMessage::kMsg_PartValueChanged))) {
		TMCCountedPtr<IMFPart> popuppart;
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> prim;
		TMCCountedPtr<I3DShInstance> instance;

		HidePart(sourcePart, sourceID, 'RMSL');
		HidePart(sourcePart, sourceID, 'PREV');
		HidePart(sourcePart, sourceID, 'LINE');
		HidePart(sourcePart, sourceID, 'OBJS');
		HidePart(sourcePart, sourceID, 'OBJP');
		HidePart(sourcePart, sourceID, 'OBJL');

		//reference object list
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('R','E','F','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('R','E','F','P'));
			}
		if (tree != NULL) 
		{
			tree->QueryInterface(IID_I3DShInstance, (void**)&instance);
			if (popuppart != NULL)
			{
				FillObjectList(NULL, instance, popuppart);
			}
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
		if (tree != NULL) 
		{
			tree->QueryInterface(IID_I3DShInstance, (void**)&instance);
			if (popuppart != NULL)
			{
				FillObjectList(NULL, instance, popuppart);
			}
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
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lLengthPicker = -1;
		}
		//width popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('W','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('W','S','H','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lWidthPicker = -1;
		}
		//stiffness popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('S','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('S','S','H','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lStiffnessPicker = -1;
		}
		//x wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('X','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('X','S','H','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lXWigglePicker = -1;
		}
		//y wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Y','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Y','S','H','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);  
			fData.lYWigglePicker = -1;
		}
		//z wiggle popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Z','S','H','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Z','S','H','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lZWigglePicker = -1;
		}
		//x custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('X','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('X','C','U','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lXCustomVectorPicker = -1;
		}
		//y custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Y','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Y','C','U','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);  
			fData.lYCustomVectorPicker = -1;
		}
		//z custom shader popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('Z','C','U','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('Z','C','U','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lZCustomVectorPicker = -1;
		}
		//tip scale popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('T','S','I','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('T','S','I','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lTipScalePicker = -1;
		}
		//tip twist popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('T','T','W','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('T','T','W','P'));
			}
		if (popuppart != NULL)
		{
			FillShaderList(popuppart);
			fData.lTipTwistPicker = -1;
		}
		//domain popup
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('D','O','M','P'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('D','O','M','P'));
			}
		
		if (popuppart != NULL)
		{
			FillDomainList(instance, popuppart);
			fData.lDomainPicker = -1;
		}
	}

	return MC_S_OK;
}


