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
#include "PublicUtilities.h"
#include "AGrPrim.h"
#include "AnythingGroovesDLL.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "I3DShInstance.h"
#include "MCCountedPtrArray.h"
#include "IAGrPrim.h"
#include "cAGrPrimPlane.h"
#include "cAGrPrimSphere.h"
#include "cAGrPrimCone.h"
#include "cAGrPrimCube.h"
#include "cAGrPrimCylinder.h"
#include "IMFTextPopupPart.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "copyright.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_AGrPrim(R_CLSID_AGrPrim);
const MCGUID IID_AGrPrim(R_IID_AGrPrim);
#else
const MCGUID CLSID_AGrPrim={R_CLSID_AGrPrim};
const MCGUID IID_AGrPrim={R_IID_AGrPrim};
#endif

inline real32 sqr(real32 pfIn){
	return pfIn * pfIn;
	}

inline real32 sqr(const TVector3 &first,const TVector3 &second){
	return sqr(first.x - second.x) 
		+ sqr(first.y - second.y) 
		+ sqr(first.z - second.z);
	}


inline int32 intcompare(real32 in) {
	return in * 10000 + .5;
	}


MCCOMErr AGrPrim::QueryInterface(const MCIID& riid, void** ppvObj)
{

	if (MCIsEqualIID(riid, IID_AGrPrim))
	{
		TMCCountedGetHelper<AGrPrim> result(ppvObj);
		result = (AGrPrim*)this;
		return MC_S_OK;
	}
	return TBasicPrimitive::QueryInterface(riid, ppvObj);
}

// Constructor of the C++ Object :
AGrPrim::AGrPrim()
{
	fData.lPrimitive = PRIMITIVE_PLANE;
	fData.fStart  = 0;
	fData.fStop = .25;
	fData.lU = 128;
	fData.lV = 128;
	fData.bPreSmooth = false;
	fData.fSmoothAngle = 0;
	fData.lAnimate = 0;
	fData.bRefresh = false;
	fData.bAdaptiveMesh = false;
	fData.bEnabled = true;
	fData.lSplitMethod = SPLIT_SHORT;
	fData.sizeX = 20;
	fData.sizeY = 20;
	fData.sizeZ = 20;
	fData.bEmptyZero = false;
	fData.lPreviewSize = PREVIEW_32;
	fData.lObjectPicker = -1;
}
AGrPrim::~AGrPrim(){
	warnings = NULL;
	scene = NULL;
	}

int16 AGrPrim::GetResID()
{
	return 310;		// This is the view ID in the resource file
}

void* AGrPrim::GetExtensionDataBuffer()
{
  return &fData;					// used by the shell to set the new parameters
} 

// I3DExGeometricPrimitive methods
// -- Geometry calls

MCCOMErr AGrPrim::GetNbrLOD(int16& nbrLod)
{
	nbrLod = 2;
	return MC_S_OK;
}

MCCOMErr    AGrPrim::GetLOD	(int16 lodIndex, real& lod) {
	switch(lodIndex) {
		case 0: 
			lod = 0;
			break;
		case 1:
			lod = .000001f;
			break;
		}
	return MC_S_OK;
	}

MCCOMErr AGrPrim::GetFacetMesh (uint32 lodIndex, FacetMesh** outMesh)
{
	IAGrPrim* agrprim = NULL;

	try {
		AGrPrimData tempdata = fData;
		if (lodIndex == 1) {
			switch (fData.lPreviewSize) {
				case PREVIEW_16: 
					tempdata.lU = tempdata.lV = 16;
					break;			
				case PREVIEW_32: 
					tempdata.lU = tempdata.lV = 32;
					break;			
				case PREVIEW_64: 
					tempdata.lU = tempdata.lV = 64;
					break;			
				case PREVIEW_128:  
					tempdata.lU = tempdata.lV = 128;
					break;	
				}
			}
		TMCCountedPtr<I3DShMasterShader> mastershader;
		TMCCountedPtr<I3DShScene> scene;
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShPrimitive> prim;
		TMCCountedPtr<I3DShShader> shader;
		DCGFacetMeshAccumulator newaccu;
		//DCGFacetMeshAccumulator* accu;

		switch (fData.lPrimitive) {
			case PRIMITIVE_PLANE:
				agrprim = new cAGrPrimPlane(tempdata);
				break;
			case PRIMITIVE_SPHERE:
				agrprim = new cAGrPrimSphere(tempdata);
				break;
			case PRIMITIVE_CONE:
				agrprim = new cAGrPrimCone(tempdata);
				break;
			case PRIMITIVE_CUBE:
				agrprim = new cAGrPrimCube(tempdata);
				break;
			case PRIMITIVE_CYLINDER:
				agrprim = new cAGrPrimCylinder(tempdata);
				break;
			}

		//reset warning stuff
		fData.Warnings = "No warnings.";
		if (!fData.bEnabled) {
			fData.Warnings =  "Disabled.";
			UpdateWarnings();
			agrprim->DoBasicMesh(outMesh);
			return MC_S_OK;
			}

		if (fData.MasterShader == kNullString) {
			fData.Warnings =  "Select shader.";
			UpdateWarnings();
			agrprim->DoBasicMesh(outMesh);
			return MC_S_OK;
			}

		//find the object I'm deforming so I can use it's shader
		if (QueryInterface(IID_I3DShPrimitiveComponent, reinterpret_cast<void**>(&primcomp)) == MC_S_OK) {
			primcomp->GetPrimitive(&prim);
			primcomp = NULL;
			if (prim->QueryInterface(IID_I3DShObject, reinterpret_cast<void**>(&object)) == MC_S_OK) {
				object->GetScene(&scene);
				object = NULL;
				}
			prim = NULL;
			}

		scene->GetMasterShaderByName(&mastershader, fData.MasterShader);
		if ((I3DShMasterShader*)mastershader == NULL) {
			fData.Warnings =  "No match for shader name.";
			UpdateWarnings();
			agrprim->DoBasicMesh(outMesh);
			return MC_S_OK;
			}

		mastershader->GetShader(&shader);
			
		//apply the deformation
		agrprim->DoMesh(&newaccu, shader);

		delete agrprim;

		if (fData.fSmoothAngle > 0) {
			newaccu.CalculateNormals(fData.fSmoothAngle);
			}
		//else
		//{
		//	newaccu.CalculateNormals(1);
		//}
		
		newaccu.MakeFacetMesh(outMesh);
		
		//clean up
		shader = NULL;
		scene = NULL;

		//update the UI
		UpdateWarnings();

		//shader = NULL;
		return MC_S_OK;
	}
	catch(...) {
		//clean up
		if (agrprim != NULL) {
			delete agrprim;
			}
		
		switch (fData.lPrimitive) {
			case PRIMITIVE_PLANE:
				agrprim = new cAGrPrimPlane(fData);
				break;
			case PRIMITIVE_SPHERE:
				agrprim = new cAGrPrimSphere(fData);
				break;
			case PRIMITIVE_CONE:
				agrprim = new cAGrPrimCone(fData);
				break;
			case PRIMITIVE_CUBE:
				agrprim = new cAGrPrimCube(fData);
				break;
			case PRIMITIVE_CYLINDER:
				agrprim = new cAGrPrimCylinder(fData);
				break;
			}

		agrprim->DoBasicMesh(outMesh);

		delete agrprim;

		fData.Warnings = "Insufficient memory to do deformation.";
		UpdateWarnings();

		return MC_S_OK;
		}
	return MC_S_OK;
}

// Give the boundary Box
void AGrPrim::GetBoundingBox(TBBox3D& bbox){
	IAGrPrim* agrprim = NULL;
	switch (fData.lPrimitive) {
		case PRIMITIVE_PLANE:
			agrprim = new cAGrPrimPlane(fData);
			break;
		case PRIMITIVE_SPHERE:
			agrprim = new cAGrPrimSphere(fData);
			break;
		case PRIMITIVE_CONE:
			agrprim = new cAGrPrimCone(fData);
			break;
		case PRIMITIVE_CUBE:
			agrprim = new cAGrPrimCube(fData);
			break;
		case PRIMITIVE_CYLINDER:
			agrprim = new cAGrPrimCylinder(fData);
			break;
		}

	agrprim->GetBoundingBox (bbox);
	
	delete agrprim;
}
  
// -- Shading calls
uint32 AGrPrim::GetUVSpaceCount()
{
	IAGrPrim* agrprim = NULL;
	switch (fData.lPrimitive) {
		case PRIMITIVE_PLANE:
			agrprim = new cAGrPrimPlane(fData);
			break;
		case PRIMITIVE_SPHERE:
			agrprim = new cAGrPrimSphere(fData);
			break;
		case PRIMITIVE_CONE:
			agrprim = new cAGrPrimCone(fData);
			break;
		case PRIMITIVE_CUBE:
			agrprim = new cAGrPrimCube(fData);
			break;
		case PRIMITIVE_CYLINDER:
			agrprim = new cAGrPrimCylinder(fData);
			break;
		}

	return agrprim->GetUVSpaceCount();
	
	delete agrprim;
}


MCCOMErr AGrPrim::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo)
{
	IAGrPrim* agrprim = NULL;
	switch (fData.lPrimitive) {
		case PRIMITIVE_PLANE:
			agrprim = new cAGrPrimPlane(fData);
			break;
		case PRIMITIVE_SPHERE:
			agrprim = new cAGrPrimSphere(fData);
			break;
		case PRIMITIVE_CONE:
			agrprim = new cAGrPrimCone(fData);
			break;
		case PRIMITIVE_CUBE:
			agrprim = new cAGrPrimCube(fData);
			break;
		case PRIMITIVE_CYLINDER:
			agrprim = new cAGrPrimCylinder(fData);
			break;
		}

	return agrprim->GetUVSpace(uvSpaceID, uvSpaceInfo);
	
	delete agrprim;
}

boolean	AGrPrim::AutoSwitchToModeler() const 
{
	return true;
};

MCCOMErr AGrPrim::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	//warnings label, lets grab one
	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if ((IMFPart*)warnings == NULL) {
		TMCCountedPtr<IMFPart> parent;
		//if we don't have a pointer to the 
		sourcePart->GetPartParent(&parent);
		if (parent != NULL) {
			parent->FindChildPartByID(&warnings, IDTYPE('W','A','R','N'));
			}
		}
	
	if ((I3DShScene*)scene == NULL) {
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

	if ((message == kMsg_CUIP_ComponentAttached)||((sourceID == 'RMSL')&&(message == EMFPartMessage::kMsg_PartValueChanged ))) {
		TMCCountedPtr<IMFPart> popuppart;
		TMCCountedPtr<IMFTextPopupPart> popup;
		TMCString255 name;

		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, IDTYPE('O','P','I','C'));
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, IDTYPE('O','P','I','C'));
			}
		popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
		popup->RemoveAll();


		if (scene != NULL) {
			TMCCountedPtr<I3DShMasterShader> mastershader;
			uint32 numshaders = scene->GetMasterShadersCount();
			uint32 nummenu = 0;
			for (uint32 shaderindex = 0; shaderindex < numshaders; shaderindex++) {
				scene->GetMasterShaderByIndex(&mastershader, shaderindex);
				mastershader->GetName(name);
				mastershader = NULL;
				popup->AppendMenuItem(name);
				popup->SetItemActionNumber(nummenu, shaderindex);
				nummenu++;
				}
			}

		}

	return MC_S_OK;
	}

void AGrPrim::UpdateWarnings() {
	if (warnings != NULL) {
		warnings->SetValue((void *)&fData.Warnings 
				, kStringValueType
				, true
				, false);
		warnings = NULL;
		}
	}

TMCString255 AGrPrim::GetMasterShaderName() {
	return fData.MasterShader;
	}

MCCOMErr  AGrPrim::ExtensionDataChanged()
{
	if (fData.lObjectPicker == -2) {
		//do nothing
		fData.lObjectPicker = -1;
		}
	else if (fData.lObjectPicker != -1)  {
		if (scene != NULL) {
			TMCString255 name;
			TMCCountedPtr<I3DShMasterShader> mastershader;
			scene->GetMasterShaderByIndex(&mastershader, fData.lObjectPicker);
			mastershader->GetName(name);
			mastershader = NULL;
			fData.MasterShader = name;
						
			}
		fData.lObjectPicker = -1;
		}
	return MC_S_OK;
}
