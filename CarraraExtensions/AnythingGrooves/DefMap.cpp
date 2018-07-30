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
#include "DefMap.h"
#include "MCCountedPtrHelper.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#include "I3DShUtilities.h"
#include "IShChannel.h"
#include "IMFMenu.h"
#include "shadertypes.h"
#include "COM3DUtilities.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "I3dShObject.h"
#include "AnythingGroovesDLL.h"
#include "IMFTextPopupPart.h" 
#include "MFPartMessages.h"
#include "commessages.h"
#include "I3DShTreeElement.h"
#include "copyright.h"


#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_DefMap(R_CLSID_DefMap);
const MCGUID IID_DefMap(R_IID_DefMap);
#else
const MCGUID CLSID_DefMap={R_CLSID_DefMap};
const MCGUID IID_DefMap={R_IID_DefMap};
#endif

inline real32 sqr(real32 pfIn){
	return pfIn * pfIn;
	}

inline real32 sqr(const TVector3 &first,const TVector3 &second){
	return sqr(first.x - second.x) 
		+ sqr(first.y - second.y) 
		+ sqr(first.z - second.z);
	}
inline real32 intcompare(const real32 &in) {
	return floorf(in * 100000 + .5);
	}
inline real32 filter(const real32 &in) {
	return intcompare(in) / (real32)100000;
	}

MCCOMErr DefMap::QueryInterface(const MCIID& riid, void** ppvObj)
{

	if (MCIsEqualIID(riid, IID_DefMap))
	{
		TMCCountedGetHelper<DefMap> result(ppvObj);
		result = (DefMap*)this;
		return MC_S_OK;
	}
	return TBasicDeformModifier::QueryInterface(riid, ppvObj);
}


int16 DefMap::GetResID()
{
	return 308;		// This is the view ID in the resource file
}

// Constructor / Destructor of the C++ Object :
DefMap::DefMap() {
  
	// Data initialisation :
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
	fData.lObjectPicker = -1;
	//fFlags.SetMasked(TModifierFlags::kDisableBackFacing);
	fFlags.SetMasked(TModifierFlags::kDeformModifier);
	shadingIn.fCurrentCompletionMask = 0;
}
  
void* DefMap::GetExtensionDataBuffer()
{
  return &fData;					// used by the shell to set the new parameters
}
MCCOMErr  DefMap::ExtensionDataChanged()
{
	if (fData.lObjectPicker != -1)  {
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

	if (fData.lU != 0) {
		fUVInterval.x = 1.0 / fData.lU;
		}
	else {
		fUVInterval.x = 0;
		}
	if (fData.lV != 0) {
		fUVInterval.y = 1.0 / fData.lV;
		}
	else {
		fUVInterval.y = 0;
		}
	return MC_S_OK;
}

void DefMap::Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger>	result(res);

	DefMap* theClone = new DefMap();
	ThrowIfNil(theClone);
	theClone->SetControllingUnknown(pUnkOuter);
	CopyData(theClone);

	result = (IExDataExchanger*) theClone;
}


void DefMap::CopyData(DefMap* dest) const
{
	dest->fData = fData;
	if (scene != NULL) {
		dest->scene = scene;
		}
}
  
void DefMap::SetBoundingBox(const TBBox3D& bbox)
{
}

MCCOMErr DefMap::DeformPoint(const TVector3& point, TVector3& result)
{
  
  return MC_S_OK;
}
void DefMap::BuildEndpointsU(const TVertex3D pt[3]){
	//build the endpoints for the U lines
	int32 VOrder[3] = {0,0,0};
	DefPoint* Vpt[3];
	Vpt[0] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
	Vpt[1] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
	Vpt[2] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));

	real32 fFarAlong;
	real32 fMinV, fMaxV, V20, V21, V10;
	bool bMidHigh;
	int32 V;

	SortPointsV(pt, &VOrder[0]);

	Vpt[0]->vertex  = pt[VOrder[0]];
	Vpt[1]->vertex  = pt[VOrder[1]];
	Vpt[2]->vertex = pt[VOrder[2]];


	fFarAlong = (Vpt[1]->vertex.fUV.y - Vpt[0]->vertex.fUV.y) / (Vpt[2]->vertex.fUV.y - Vpt[0]->vertex.fUV.y);
	bMidHigh = ((Vpt[0]->vertex.fUV.x + fFarAlong * (Vpt[2]->vertex.fUV.x - Vpt[0]->vertex.fUV.x)) < Vpt[1]->vertex.fUV.x);

	fMinV = Vpt[0]->vertex.fUV.y * fData.lV;
	fMaxV = Vpt[2]->vertex.fUV.y * fData.lV;
	
	if (Vpt[2]->vertex.fUV.y - Vpt[0]->vertex.fUV.y == 0) {
		V20 = 0;
		}
	else {
		V20 = 1 / (Vpt[2]->vertex.fUV.y - Vpt[0]->vertex.fUV.y);
		}

	if (Vpt[2]->vertex.fUV.y - Vpt[1]->vertex.fUV.y == 0) {
		V21 = 0;
		}
	else {
		V21 = 1 / (Vpt[2]->vertex.fUV.y - Vpt[1]->vertex.fUV.y);
		}

	if (Vpt[1]->vertex.fUV.y - Vpt[0]->vertex.fUV.y == 0) {
		V10 = 0;
		}
	else {
		V10 = 1 / (Vpt[1]->vertex.fUV.y - Vpt[0]->vertex.fUV.y);
		}

	for (V = ceil(fMinV); V <= floor(fMaxV); V++) {
		DefPoint* LeftPoint;
		DefPoint* RightPoint;
		RightPoint = Deformed.GetRight(V);
		LeftPoint = Deformed.GetLeft(V);

		if (bMidHigh == true) {
			if (Vpt[0]->vertex.fUV.y == Vpt[1]->vertex.fUV.y){
				fFarAlong = (V * fUVInterval.y - Vpt[1]->vertex.fUV.y) * V21;
				FillPoint(fFarAlong, Vpt[1]->vertex , Vpt[2]->vertex, RightPoint->vertex);
				}
			else if (Vpt[2]->vertex.fUV.y == Vpt[1]->vertex.fUV.y){
				fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y) * V10;
				FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[1]->vertex, RightPoint->vertex);
				}
			else if(V * fUVInterval.y <= Vpt[1]->vertex.fUV.y) {
				fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y)  * V10;
				FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[1]->vertex, RightPoint->vertex);
				}
			else {
				fFarAlong = (V * fUVInterval.y - Vpt[1]->vertex.fUV.y) * V21;
				FillPoint(fFarAlong, Vpt[1]->vertex, Vpt[2]->vertex, RightPoint->vertex);
				}
			fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y) * V20;
			FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[2]->vertex, LeftPoint->vertex);
			}
		else {
			if (Vpt[0]->vertex.fUV.y == Vpt[1]->vertex.fUV.y){
				fFarAlong = (V * fUVInterval.y - Vpt[1]->vertex.fUV.y) * V21;
				FillPoint(fFarAlong, Vpt[1]->vertex, Vpt[2]->vertex, LeftPoint->vertex);
				}
			else if (Vpt[2]->vertex.fUV.y == Vpt[1]->vertex.fUV.y){
				fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y) * V10;
				FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[1]->vertex, LeftPoint->vertex);
				}
			else if (V * fUVInterval.y <= Vpt[1]->vertex.fUV.y) {
				fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y) * V10;
				FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[1]->vertex, LeftPoint->vertex);
				}
			else {
				fFarAlong = (V * fUVInterval.y - Vpt[1]->vertex.fUV.y) * V21;
				FillPoint(fFarAlong, Vpt[1]->vertex, Vpt[2]->vertex, LeftPoint->vertex);
				}
			fFarAlong = (V * fUVInterval.y - Vpt[0]->vertex.fUV.y) * V20;
			FillPoint(fFarAlong, Vpt[0]->vertex, Vpt[2]->vertex, RightPoint->vertex);
			}
		if (fData.lSplitMethod == SPLIT_SMART) {
			RightPoint->original = RightPoint->vertex;
			LeftPoint->original = LeftPoint->original;
			}
		DeformPoint(LeftPoint);
		LeftPoint->bHasData = true;
		DeformPoint(RightPoint);
		RightPoint->bHasData = true;
		}	
	MCfree(Vpt[0]);
	MCfree(Vpt[1]);
	MCfree(Vpt[2]);
	}

void DefMap::ApplyDeformation(const TFacet3D* aFacet) 
{
	shadingIn.fUVSpaceID = aFacet->fUVSpace;
	
	TVertex3D Vpt[3];

	real32 fFarAlong;

	UOrder[0] = UOrder[1] = UOrder[2] = 0;

	int32 U, UBlockStop, UBlockStart, midU;

	TMCColorRGB color;
	//throw out facets with bad UV coordinates
	if ((intcompare(aFacet->fVertices[0].fUV.x) == intcompare(aFacet->fVertices[1].fUV.x)) 
		&& (intcompare(aFacet->fVertices[1].fUV.x) == intcompare(aFacet->fVertices[2].fUV.x))){
		mlBadFacets++;
		return;
		}
	if ((intcompare(aFacet->fVertices[0].fUV.y) == intcompare(aFacet->fVertices[1].fUV.y)) 
		&& (intcompare(aFacet->fVertices[1].fUV.y) == intcompare(aFacet->fVertices[2].fUV.y))){
		mlBadFacets++;
		return;
		}
	
	//figure out the order
	if ((aFacet->fVertices[2].fUV.x < aFacet->fVertices[0].fUV.x)
		&&(aFacet->fVertices[2].fUV.x < aFacet->fVertices[1].fUV.x)){
		UOrder[0] = 2;
		}
	else 
	if ((aFacet->fVertices[1].fUV.x <= aFacet->fVertices[0].fUV.x)
		&&(aFacet->fVertices[1].fUV.x <= aFacet->fVertices[2].fUV.x)) {
		UOrder[0] = 1;
		}

	if ((aFacet->fVertices[1].fUV.x > aFacet->fVertices[0].fUV.x)
		&&(aFacet->fVertices[1].fUV.x > aFacet->fVertices[2].fUV.x)){
		UOrder[2] = 1;
		}
	else 
	if ((aFacet->fVertices[2].fUV.x >= aFacet->fVertices[0].fUV.x)
		&&(aFacet->fVertices[2].fUV.x >= aFacet->fVertices[1].fUV.x)) {
		UOrder[2] = 2;
		}

	if ((UOrder[0] != 0)&&(UOrder[2] != 0)) {
		UOrder[1] = 0;
		}
	else if((UOrder[0] != 1)&&(UOrder[2] != 1)) {
		UOrder[1] = 1;
		}
	else if((UOrder[0] != 2)&&(UOrder[2] != 2)) {
		UOrder[1] = 2;
		}
	Upt[0] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
	Upt[1] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
	Upt[2] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));

	Upt[0]->vertex = aFacet->fVertices[UOrder[0]];
	Upt[1]->vertex = aFacet->fVertices[UOrder[1]];
	Upt[2]->vertex = aFacet->fVertices[UOrder[2]];

	Upt[0]->original = Upt[0]->vertex;
	Upt[1]->original = Upt[1]->vertex;
	Upt[2]->original = Upt[2]->vertex;

	fFarAlong = (Upt[1]->vertex.fUV.x - Upt[0]->vertex.fUV.x) 
			/ (Upt[2]->vertex.fUV.x - Upt[0]->vertex.fUV.x);
	bMidHigh = ((Upt[0]->vertex.fUV.y + fFarAlong * (Upt[2]->vertex.fUV.y - Upt[0]->vertex.fUV.y)) 
		< Upt[1]->vertex.fUV.y);

	if (bMidHigh) {
		//mid point is high
		AddOrder[0] = UOrder[0];
		AddOrder[1] = UOrder[1];
		AddOrder[2] = UOrder[2];
		}
	else {
		//mid point is low
		AddOrder[0] = UOrder[0];
		AddOrder[1] = UOrder[2];
		AddOrder[2] = UOrder[1];
		}


	fU[0] = Upt[0]->vertex.fUV.x * fData.lU;
	fU[1] = Upt[1]->vertex.fUV.x * fData.lU;
	fU[2] = Upt[2]->vertex.fUV.x * fData.lU;
	
	midU = intcompare(fU[1]);

	bFlat = true;

	//build the V and internal points
	SetUpGrid();

	//build the U end points
	BuildEndpointsU(aFacet->fVertices);

	DeformPoint(Upt[0]);
	DeformPoint(Upt[1]);
	DeformPoint(Upt[2]);

	if (bFlat)
	{
		if (bMidHigh) {
			AddFacet(Upt[0]
				, Upt[1], Upt[2]);
			}
		else {
			AddFacet(Upt[0]
				, Upt[2], Upt[1]);
			}
		MCfree(Upt[0]);
		MCfree(Upt[1]);
		MCfree(Upt[2]);
		return;
	}

	UBlockStart = ceil(fU[0]);
	UBlockStart += (intcompare(ceil(fU[0])) == intcompare(fU[0]));

	UBlockStop = floor(fU[2]);
	UBlockStop -= (intcompare(floor(fU[2])) == intcompare(fU[2]));

	if (
		(intcompare(Upt[1]->vertex.fUV.x * fData.lU)<=intcompare(UBlockStart))
		&&(intcompare(Upt[2]->vertex.fUV.x * fData.lU)<=intcompare(UBlockStart))
		) {
		/*Upt[0] always <= UBlockStart*/
		//everything in the same U line
		/*   1     |    
             | \   |
	         |   \ |
	         0----2|
		*/
		if (bMidHigh) {
			DoTriangle(UBlockStart - 1, Upt[0]
				, Upt[1], Upt[2]);
			}
		else {
			DoTriangle(UBlockStart - 1, Upt[0]
				, Upt[2], Upt[1]);
			}
		return;
		}
	else
	if (intcompare(Upt[1]->vertex.fUV.x * fData.lU)<intcompare(UBlockStart)) {
		//Upt[0] and Upt[1] below UBlockStart
		/*   1  |    
             | \|
	         |  |
	         0--|
			    |*/
		DoLeftTriangle(UBlockStart - 1, true);
		}
	else{
		//everything in different U lines
		/*      |    
			   /|
			  / |
			 0--|
				|*/
		DoLeftTriangle(UBlockStart - 1, false);
		}
	U = UBlockStart;
	//spin through the loop and create facets for each section
	/*		 | |  
             |/|
	         | |
	         |-|
			 |*/  
	while (U + 1 <= UBlockStop) {
		ApplyDeformationToUStrip(U
			//is this the midpoint
			, ((midU > intcompare(U))
				&&(midU < intcompare(U + 1))));
		U++;
		}

	if ((intcompare(fU[1]) > intcompare(U))
		&&(intcompare(fU[2]) > intcompare(U))) {
		//Upt[1] and Upt[2] are after last slice
		/*   | 1  
             |/|
	         | |
	         |-2
			 |*/  
		DoRightTriangle(U, true);
		}
	else
	if (intcompare(fU[2]) > intcompare(U)){
		//everything in different U lines
		/*   |    
			 |\
			 | \
			 |--2
			 |*/
		DoRightTriangle(U, false);
		}
	MCfree(Upt[0]);
	MCfree(Upt[1]);
	MCfree(Upt[2]);
}

MCCOMErr DefMap::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh) {
	try {
		TMCCountedPtr<FacetMesh> localmesh;
		DCGFacetMeshAccumulator newaccu;
		FacetMeshFacetIterator iter;
		TFacet3D aFacet;

		//reset warning stuff
		mlBadFacets = 0;
		fData.Warnings = "No warnings.";
		if (!fData.bEnabled) {
			fData.Warnings =  "Disabled.";
			in->Clone(outMesh);
			return MC_S_OK;
			}

		//normalize all UVs to 0 through 1
		int fUVElements = in->fuv.GetElemCount();
		
		//empty mesh we're out of here
		if (fUVElements == 0) {
			fData.Warnings =  "Empty mesh.";
			in->Clone(outMesh);
			return MC_S_OK;
			}

		if ((fData.lU > 1)||(fData.lV > 1))
		{
			//check for out of range UV coordinates, but doesn't matter if I'm doing just points
			real32 fMaxU, fMinU, fMaxV, fMinV;

			//start here
			fMinU = in->fuv[0].x;
			fMaxU = in->fuv[0].x;
			fMinV = in->fuv[0].y;
			fMaxV = in->fuv[0].y;
			for(int i = 1; i < fUVElements; i++) 
			{
				if (in->fuv[i].x < fMinU) {fMinU = in->fuv[i].x;};
				if (in->fuv[i].x > fMaxU) {fMaxU = in->fuv[i].x;};
				if (in->fuv[i].y < fMinV) {fMinV = in->fuv[i].y;};
				if (in->fuv[i].y > fMaxV) {fMaxV = in->fuv[i].y;};
			}
			//display a message if the UV coordinates are fubar
			if ((intcompare(fMaxU) > intcompare(2))||(intcompare(fMinU) < intcompare(-1))
				||(intcompare(fMaxV) > intcompare(2))||(intcompare(fMinV) < intcompare(-1))) 
			{
				fData.Warnings =  "Bad UV coordinates, Anything Grooves cannot be applied.";
				in->Clone(outMesh);
				return MC_S_OK;
			}
		}

		//find the object I'm deforming so I can use it's transform
		TMCCountedPtr<I3DShTreeElement>	tree;
		TMCCountedPtr<I3DShTreeElementMember> Member;
		TMCCountedPtr<I3DShMasterShader> mastershader;
		TMCCountedPtr<I3DShInstance> instance;
		TBasicModifier::QueryInterface(IID_I3DShTreeElementMember, (reinterpret_cast<void**>(&Member)));

		ThrowIfNil(Member);
        Member->GetTreeElement(&tree);
		ThrowIfNil(tree);
		tree->GetScene(&scene);
#if (VERSIONNUMBER >= 0x040000)
		fTransform = tree->GetGlobalTransform3D();
#else
		tree->GetGlobalTransform3D(fTransform);
#endif 

		if (fData.MasterShader == kNullString) {
			fData.Warnings =  "Select shader.";
			in->Clone(outMesh);
			return MC_S_OK;
			}

		scene->GetMasterShaderByName(&mastershader, fData.MasterShader);
		if (!mastershader) {
			fData.Warnings =  "No match for shader name.";
			in->Clone(outMesh);
			return MC_S_OK;
			}

		mastershader->GetShader(&shader);
		shader->GetShadingFlags(shadingflags);

		//presmooth if needed
		iter.Initialize(in);
		
		if (fData.bPreSmooth) {
			DCGFacetMeshAccumulator PresmoothAccu;
			for (iter.First(); iter.More(); iter.Next()) {
				TFacet3D aFacet = iter.GetFacet();
				PresmoothAccu.AccumulateFacet(&aFacet);
				}

			PresmoothAccu.CalculateNormals((float)179.99);
			PresmoothAccu.MakeFacetMesh(&localmesh);
			iter.Initialize(localmesh);
			}

			
		//apply the deformation
		accu = &newaccu;
		if ((fData.lU == 1)&&(fData.lV == 1))
		{
			accu->PrepareAccumulation(in->fFacets.GetElemCount());
			AddOrder[0] = 0;
			AddOrder[1] = 1;
			AddOrder[2]	= 2;
			Upt[0] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
			Upt[1] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
			Upt[2] = (DefPoint*)MCcalloc(1,sizeof(DefPoint));
			for (iter.First(); iter.More(); iter.Next()) 
			{
				TFacet3D aFacet = iter.GetFacet();
				shadingIn.fUVSpaceID = aFacet.fUVSpace;

				Upt[0]->vertex = aFacet.fVertices[0];
				Upt[1]->vertex = aFacet.fVertices[1];
				Upt[2]->vertex = aFacet.fVertices[2];

				DeformPoint(Upt[0]);
				DeformPoint(Upt[1]);
				DeformPoint(Upt[2]);

				AddFacet(Upt[0], Upt[1], Upt[2]);
			}
			MCfree(Upt[0]);
			MCfree(Upt[1]);
			MCfree(Upt[2]);
		}
		else
		{
			for (iter.First(); iter.More(); iter.Next()) 
			{
				TFacet3D aFacet = iter.GetFacet();
				ApplyDeformation (&aFacet);
			}
		}
		//get rid of local mesh if we used one
		localmesh = NULL;
		//get rid of the point array
		Deformed.Deallocate();

		//if nothing deformed let the user know
		if (accu->GetAccumulatedFacetsCount() == 0) {
			fData.Warnings = "No facets with good UV coordinates, Anything Grooves cannot be applied.";
			in->Clone(outMesh);
			return MC_S_OK;
			}

		if (fData.fSmoothAngle > 0 ) {
			accu->CalculateNormals(fData.fSmoothAngle);
			}

		accu->MakeFacetMesh(outMesh);

		if (mlBadFacets > 0) {
			TMCString255 temp = " facets deleted because of bad UV coordinates.";
			fData.Warnings.FromInt32(mlBadFacets);
			fData.Warnings = fData.Warnings + temp;
			}
		
		//clean up
		accu = NULL;

		//shader = NULL;
		return MC_S_OK;
	}
	catch(...) {
		//clean up
		accu = NULL;
		in->Clone(outMesh);
		return MC_S_OK;
		}
}

MCCOMErr DefMap::DeformBBox(const TBBox3D &in, TBBox3D &out) {
	real32 newbounds;
	if (fData.fStop > fData.fStart) {
		newbounds = fData.fStop;
		}
	else {
		newbounds = fData.fStart;
		}
	
	out = in;
	out.fMin.x -= newbounds;
	out.fMin.y -= newbounds;
	out.fMin.z -= newbounds;
	out.fMax.x += newbounds;
	out.fMax.y += newbounds;
	out.fMax.z += newbounds;
	return MC_S_OK;
}


void DefMap::FillPoint(const real32& fFarAlong,TVertex3D& pt1,TVertex3D& pt2, TVertex3D& result) {
	//do the UV
	result.fUV.x = pt1.fUV.x + fFarAlong * (pt2.fUV.x - pt1.fUV.x);
	result.fUV.y = pt1.fUV.y + fFarAlong * (pt2.fUV.y - pt1.fUV.y);


	//do the 3D point
	result.fVertex.x = pt1.fVertex.x + fFarAlong * (pt2.fVertex.x - pt1.fVertex.x);
	result.fVertex.y = pt1.fVertex.y + fFarAlong * (pt2.fVertex.y - pt1.fVertex.y);
	result.fVertex.z = pt1.fVertex.z + fFarAlong * (pt2.fVertex.z - pt1.fVertex.z);

	result.fNormal.x = pt1.fNormal.x + fFarAlong * (pt2.fNormal.x - pt1.fNormal.x);
	result.fNormal.y = pt1.fNormal.y + fFarAlong * (pt2.fNormal.y - pt1.fNormal.y);
	result.fNormal.z = pt1.fNormal.z + fFarAlong * (pt2.fNormal.z - pt1.fNormal.z);

	//result.fNormal.Normalize(result.fNormal);
	}

void DefMap::AddFacet(DefPoint* pt1,DefPoint* pt2,DefPoint* pt3) {
	if ((fData.bEmptyZero == true)
		&&(intcompare(pt1->displacement) == 0)
		&&(intcompare(pt2->displacement) == 0)
		&&(intcompare(pt3->displacement) == 0)) {
		return;
		}

	TFacet3D newfacet;

	newfacet.fUVSpace = shadingIn.fUVSpaceID;
	newfacet.fReserved = 0;
	
	switch (AddOrder[0]) {
		case 0: newfacet.fVertices[0] = pt1->vertex; break;
		case 1: newfacet.fVertices[0] = pt2->vertex; break;
		case 2: newfacet.fVertices[0] = pt3->vertex; break;
		}
	switch (AddOrder[1]) {
		case 0: newfacet.fVertices[1] = pt1->vertex; break;
		case 1: newfacet.fVertices[1] = pt2->vertex; break;
		case 2: newfacet.fVertices[1] = pt3->vertex; break;
		}
	switch (AddOrder[2]) {
		case 0: newfacet.fVertices[2] = pt1->vertex; break;
		case 1: newfacet.fVertices[2] = pt2->vertex; break;
		case 2: newfacet.fVertices[2] = pt3->vertex; break;
		}

	newfacet.fVertices[2].fNormal = 
		(newfacet.fVertices[1].fVertex - newfacet.fVertices[0].fVertex)
		^ (newfacet.fVertices[2].fVertex - newfacet.fVertices[1].fVertex);
	newfacet.fVertices[2].fNormal.Normalize(newfacet.fVertices[2].fNormal);
	newfacet.fVertices[0].fNormal = newfacet.fVertices[1].fNormal = newfacet.fVertices[2].fNormal;

	try {
		accu->AccumulateFacet(&newfacet);
		}
	catch(...) {
		fData.Warnings = "Insufficient memory to do deformation.";
		throw;
		}

	}

void DefMap::SortPointsV(const TVertex3D pt[3], int32* VOrder) {
	VOrder[0] = VOrder[1] = VOrder[2] = 0;

	if ((pt[2].fUV.y < pt[0].fUV.y)
		&&(pt[2].fUV.y < pt[1].fUV.y)){
		VOrder[0] = 2;
		}
	else 
	if ((pt[1].fUV.y <= pt[0].fUV.y)
		&&(pt[1].fUV.y <= pt[2].fUV.y)) {
		VOrder[0] = 1;
		}

	if ((pt[1].fUV.y > pt[0].fUV.y)
		&&(pt[1].fUV.y > pt[2].fUV.y)){
		VOrder[2] = 1;
		}
	else 
	if ((pt[2].fUV.y >= pt[0].fUV.y)
		&&(pt[2].fUV.y >= pt[1].fUV.y)) {
		VOrder[2] = 2;
		}

	if ((VOrder[0] != 0)&&(VOrder[2] != 0)) {
		VOrder[1] = 0;
		}
	else if((VOrder[0] != 1)&&(VOrder[2] != 1)) {
		VOrder[1] = 1;
		}
	else if((VOrder[0] != 2)&&(VOrder[2] != 2)) {
		VOrder[1] = 2;
		}
	}
void DefMap::DoLeftTriangle(int32 U
							, boolean bMidPoint){

DefPoint* left = Upt[0];
DefPoint* bottomright = Deformed.GetBottom(U + 1);
DefPoint* topright = Deformed.GetTop(U + 1);

int32 Shape = 0;
int32 V, VBlockStop, VBlockStart;
int32 lowV, highV;
int32 leftV = intcompare(left->vertex.fUV.y * fData.lV)
	, bottomrightV = intcompare(bottomright->vertex.fUV.y * fData.lV)
	, toprightV = intcompare(topright->vertex.fUV.y * fData.lV)
	, midV = intcompare(Upt[1]->vertex.fUV.y * fData.lV)
	, lowest, highest;
uint32 vsize = 1, usize = 1;

VBlockStart = floor(left->vertex.fUV.y * fData.lV);
lowest = leftV;
if (bottomrightV < intcompare(VBlockStart)) {
	VBlockStart = floor(filter(bottomright->vertex.fUV.y * fData.lV));
	lowest = bottomrightV;
	}
if ((bMidPoint)
	&&(midV < intcompare(VBlockStart))) {
	VBlockStart = floor(filter(Upt[1]->vertex.fUV.y * fData.lV));
	lowest = midV;
	}


VBlockStop = ceil(left->vertex.fUV.y * fData.lV);
highest = leftV;
if (toprightV > intcompare(VBlockStop)) {
	VBlockStop = ceil(filter(topright->vertex.fUV.y * fData.lV));
	highest = toprightV;
	}
if ((bMidPoint)
	&&(midV > intcompare(VBlockStop))) {
	VBlockStop = ceil(filter(Upt[1]->vertex.fUV.y * fData.lV));
	highest = midV;
	}


for (V = VBlockStart; V < VBlockStop; V++) {
	lowV = intcompare(V);
	highV = intcompare(V + 1);
	DefPoint* blocktr;
	DefPoint* blockbr;
	DefPoint* blockbl;
	DefPoint* blocktl;
	blocktr = Deformed.Point(U + 1, V + 1);
	blockbr = Deformed.Point(U + 1, V);
	blockbl = Deformed.Point(U, V);
	blocktl = Deformed.Point(U, V + 1);

	if ((blocktr != NULL)
		&&
		(blockbr != NULL)
		&&
		(blockbl != NULL)
		&&
		(blocktl != NULL)
		&&
		(blocktr->bHasData == true)
		&&
		(blockbr->bHasData == true)
		&&
		(blockbl->bHasData == true)
		&&
		(blocktl->bHasData == true)) {
		Shape = -1;
		}
	else {
		Shape = 0;
		if (leftV >= toprightV) {
			Shape += 1 * ((leftV > lowV)&&(leftV <= highV));
			}
		else if (leftV <= bottomrightV) {
			Shape += 1 * ((leftV >= lowV)&&(leftV < highV));
			}
		else {
			Shape += 1 * ((leftV > lowV)&&(leftV < highV));
			}
		Shape += 2 * ((toprightV > lowV)&&(toprightV <= highV));
		Shape += 4 * ((bottomrightV >= lowV)&&(bottomrightV < highV));
		if ((bMidPoint)) {
			if (bMidHigh) {
				Shape += 8 * ((midV > lowV)&&(midV <= highV));
				}
			else {
				Shape += 8 * ((midV >= lowV)&&(midV < highV));
				}
			}
		}

	switch (Shape) {
		case -1://mid: done
			if (fData.bAdaptiveMesh) {
				if(!Deformed.Point(U, V)->bFilled){
					GetBlockSize(U, V, usize, vsize, VBlockStop);
					AddBlock(U, V, usize, vsize);				
					}
				}
			else {
				AddBlock(U, V, 1, 1);
				}
			break;
		case 0://mid: done
			//make sure we don't have nothing because
			//we were out of range
			if ((highest > highV)&&(lowest < lowV)) {
				AddBlock(U, V, 1, 1);
				}
			break;
		case 1://mid: done
			if (bottomrightV > leftV) {
				AddFacet(left
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(midV < leftV)){
					if (leftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, left
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, left
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else
			if (toprightV < leftV) {
				AddFacet(left
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				if ((bMidPoint)&&(midV > leftV)) {
					if (leftV == highV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, left
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, left
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				if ((leftV == lowV)||(leftV == highV)) {
					//top edge 
					AddBlock(U, V, 1, 1);
					}
				else {//todo: change to AddQuad
					//in middle
					AddFacet(left
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(left
						, Deformed.GetRightMost(U, V + 1)
						, Deformed.GetRightMost(U, V));
					AddFacet(left
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				}
			break;
		case 2://mid: done
			if (leftV < toprightV) {
				//left point low
				AddFacet(topright
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				if ((bMidPoint)&&(midV > toprightV)) {
					if (toprightV == highV) {
						AddFacet(topright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(topright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
					//left point high
				if (toprightV == highV) {
					//topedge
					AddBlock(U, V, 1, 1);
					}
				else {//todo: change to AddQuad
					AddFacet(topright
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(topright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(topright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;
		case 3://mid: done
			if (leftV == lowV) {
				AddFacet(Deformed.GetRightMost(U, V)
					, left
					, topright);
				if ((bMidPoint)&&(bMidHigh)) {
					if (toprightV == highV) {
						AddFacet(topright
							, left
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(topright
							, left
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, left
					, topright);
				if ((bMidPoint)&&(bMidHigh)) {
					if ((leftV == highV)&&(toprightV == highV)) {
						//nothing to do
						}
					else if (leftV == highV) {
						AddFacet(topright
							, left
							, Deformed.GetLeftMost(U, V + 1));
						}
					else if (toprightV == highV) {
						AddFacet(topright
							, left
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(topright
							, left
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			break;
		case 4://mid: done
			if (leftV > bottomrightV) {
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(midV < bottomrightV)) {
					if (bottomrightV == lowV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, bottomright);
						}
					}
				}
			else {
				if (bottomrightV == lowV) {
					AddBlock(U, V, 1, 1);
					}
				else {
					if ((highV - bottomrightV) > (bottomrightV - lowV)) {
						//closer to bottom
						AddQuad(bottomright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1)); 
						AddFacet(Deformed.GetRightMost(U, V)
							,  Deformed.GetLeftMost(U, V)
							, bottomright);
						}
					else {
						//closer to top
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, bottomright); 
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, bottomright
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}
			break;
		case 5://mid: done
			if (leftV == highV) {
				AddFacet(bottomright
					, left
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(bMidHigh==false)) {
					if(bottomrightV == lowV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V)
							, left);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, bottomright);
						}
					}
				}
			else {
				AddQuad(bottomright
					, left
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(bMidHigh==false)) {
					if ((leftV == lowV)&&(bottomrightV == lowV)) {
						//nothing to do
						}
					else if (leftV == lowV){
						AddFacet(bottomright
							, Deformed.GetRightMost(U, V)
							, left);
						}
					else if (bottomrightV == lowV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V)
							, left);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V) 
							, Deformed.GetLeftMost(U, V)
							, left
							, bottomright);
						}
					}

				}
			break;
		case 6://mid: done
			if (leftV < bottomrightV) {
				if (bottomrightV == lowV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V)
						, topright);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topright
						, bottomright);
					}
				if ((bMidPoint)&&(midV > toprightV)) {
					if (toprightV == highV) {
						AddFacet(topright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(topright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}

					}
				}
			else {
				if (toprightV == highV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, topright);
					}
				else {
					AddQuad(topright
						, bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)); 
					}
				if ((bMidPoint)&&(midV < bottomrightV)) {
					if (bottomrightV == lowV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}
			break;
		case 7://mid: done
			AddFacet(left
				, topright
				, bottomright);
			if (bMidPoint) {
				if (bMidHigh) {
					AddQuad(topright
						, left
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)); 
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, left
						, bottomright);

					}
				}
			break;
		case 8://mid: done
			if ((leftV > midV)&&(bottomrightV > midV)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1));
				}
			else if ((toprightV < midV)&&(leftV < midV)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, Upt[1]);
				}
			else if ((midV == lowV)||(midV == highV)) {
				AddBlock(U, V, 1, 1);
				}
			else {
				if ((highV - midV) > (midV - lowV)) {
					//closer to bottom
					AddQuad(Deformed.GetRightMost(U, V)
						, Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]);
					}
				else {
					//closer to top
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, Upt[1]
						, Deformed.GetLeftMost(U, V + 1));
					}
				}
			break;
		case 9://mid: done
			if (bMidHigh) {
				FillTwoPointLeftHand(U, V, left, Upt[1]
					, (bottomrightV > leftV), (toprightV < leftV), (midV == highV), (leftV == lowV));
				}
			else {//bMidHigh == false
				FillTwoPointLeftHand(U, V, Upt[1], left
					, (bottomrightV > leftV), (toprightV < leftV), (leftV == highV), (midV == lowV));
				}
			break;
		case 10://mid: done
			if (leftV > toprightV) {
				if (bMidHigh) {
					AddQuad(topright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Upt[1]);
					AddFacet(topright
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					if (midV != highV) {
						AddFacet(Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				else {
					if (midV == highV) {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]
							, topright);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, topright);
						if (midV != lowV) {
							AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]);
							}
						}

					if (toprightV != highV) {
						AddFacet(topright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				if (midV == lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Upt[1]
						, topright);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]
						, topright);
					}
				}
			break;

		case 11://mid: done
			if (bMidHigh) {
				AddQuad(Deformed.GetRightMost(U, V)
					, left
					, Upt[1]
					, topright);
				if (leftV != lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, left);
					}
				}
			else {
				AddQuad(Deformed.GetRightMost(U, V)
					, Upt[1]
					, left
					, topright);
				if (midV != lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]);
					}
				}
			break;
		case 12://mid: done
			if (leftV < bottomrightV) {
				if (bMidHigh) {
					if (midV == lowV) {
						AddQuad(bottomright
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(bottomright
							, Deformed.GetLeftMost(U, V)
							, Upt[1]
							, Deformed.GetRightMost(U, V + 1));
						if (midV != highV) {
							AddFacet(Deformed.GetRightMost(U, V + 1)
								, Upt[1]
								, Deformed.GetLeftMost(U, V + 1));
							}
						}
					if (bottomrightV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomright);
						}
					}
				else {
					AddQuad(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, bottomright);
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					if (midV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]);
						}
					}
				}
			else {
				if (midV == highV) {
					AddFacet(bottomright
						, Upt[1]
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddQuad(bottomright
						, Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;

		case 13://mid: done
			if (bMidHigh) {
				AddQuad(bottomright
					, left
					, Upt[1]
					, Deformed.GetRightMost(U, V + 1));
				if (midV != highV) {
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, Upt[1]
						, Deformed.GetLeftMost(U, V + 1));
					}
				}
			else {
				AddQuad(bottomright
					, Upt[1]
					, left
					, Deformed.GetRightMost(U, V + 1));
				if (leftV != highV) {
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, left
						, Deformed.GetLeftMost(U, V + 1));
					}
				}
			break;

		case 14://mid: done
			if (leftV > toprightV) {
				if (bMidHigh) {
					AddQuad(topright
						, bottomright
						, Deformed.GetRightMost(U, V + 1)
						, Upt[1]);
					if (midV != highV) {
						AddFacet(Upt[1]
							, Deformed.GetRightMost(U, V + 1)
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else {
					if (midV == highV) {
						if (toprightV = highV) {
							AddFacet(bottomright
								, Upt[1]
								, topright);
							}
						else {
							AddQuad(bottomright
								, Upt[1]
								, Deformed.GetRightMost(U, V + 1)
								, topright);

							}
						}
					else {
						AddQuad(bottomright
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, topright);
						if (toprightV != highV) {
							AddFacet(topright
								, Deformed.GetRightMost(U, V + 1)
								, Deformed.GetLeftMost(U, V + 1));
							}
						}

					}
				}
			else {
				if (bMidHigh) {
					if (midV == lowV) {
						if (bottomrightV == lowV) {
							AddFacet(bottomright
								, Upt[1]
								, topright);
							}
						else {//bottomrightV != lowV
							AddQuad(bottomright
								, Deformed.GetRightMost(U, V)
								, Upt[1]
								, topright);
							}
						}
					else {//midV != lowV
						AddQuad(bottomright 
							, Deformed.GetLeftMost(U, V)
							, Upt[1]
							, topright);
						if (bottomrightV != lowV) {
							AddFacet(bottomright
								, Deformed.GetRightMost(U, V)
								, Deformed.GetLeftMost(U, V));
					
							}
						}
					}
				else {//Mid low
					AddQuad(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, topright
						, bottomright);
					if (midV != lowV) {
						AddFacet(Upt[1]
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));						}
					}
				}
			break;

		case 15://mid: done
			if (bMidHigh) {
				AddQuad(bottomright
					, left
					, Upt[1]
					, topright);
				}
			else {
				AddQuad(bottomright
					, Upt[1]
					, left
					, topright);
				}
			break;

		}
	}
	}
void DefMap::DoRightTriangle(int32 U
							 , boolean bMidPoint){
	
DefPoint* right = Upt[2];
DefPoint* bottomleft = Deformed.GetBottom(U);
DefPoint* topleft = Deformed.GetTop(U);
int32 Shape = 0;
int32 V, VBlockStop, VBlockStart;
int32 lowV, highV;
int32 rightV = intcompare(right->vertex.fUV.y * fData.lV)
	, bottomleftV = intcompare(bottomleft->vertex.fUV.y * fData.lV)
	, topleftV = intcompare(topleft->vertex.fUV.y * fData.lV)
	, midV = intcompare(Upt[1]->vertex.fUV.y * fData.lV)
	, lowest, highest;
uint32 vsize = 1, usize = 1;

VBlockStart = floor(right->vertex.fUV.y * fData.lV);
lowest = rightV;
if (bottomleftV < intcompare(VBlockStart)) {
	VBlockStart = floor(filter(bottomleft->vertex.fUV.y * fData.lV));
	lowest = bottomleftV;
	}
if ((bMidPoint)
	&&(midV < intcompare(VBlockStart))) {
	VBlockStart = floor(filter(Upt[1]->vertex.fUV.y * fData.lV));
	lowest = midV;
	}


VBlockStop = ceil(right->vertex.fUV.y * fData.lV);
highest = rightV;
if (topleftV > intcompare(VBlockStop)) {
	VBlockStop = ceil(filter(topleft->vertex.fUV.y * fData.lV));
	highest = topleftV;
	}
if ((bMidPoint)
	&&(midV > intcompare(VBlockStop))) {
	VBlockStop = ceil(filter(Upt[1]->vertex.fUV.y * fData.lV));
	highest = midV;
	}

for (V = VBlockStart; V < VBlockStop; V++) {
	lowV = intcompare(V);
	highV = intcompare(V + 1);
	DefPoint* blocktr;
	DefPoint* blockbr;
	DefPoint* blockbl;
	DefPoint* blocktl;
	blocktr = Deformed.Point(U + 1, V + 1);
	blockbr = Deformed.Point(U + 1, V);
	blockbl = Deformed.Point(U, V);
	blocktl = Deformed.Point(U, V + 1);

	if ((blocktr != NULL)
		&&
		(blockbr != NULL)
		&&
		(blockbl != NULL)
		&&
		(blocktl != NULL)
		&&
		(blocktr->bHasData == true)
		&&
		(blockbr->bHasData == true)
		&&
		(blockbl->bHasData == true)
		&&
		(blocktl->bHasData == true)) {
		Shape = -1;
		}
	else {
		Shape = 0;
		if (rightV >= topleftV) {
			Shape += 1 * ((rightV > lowV)&&(rightV <= highV));
			}
		else if (rightV <= bottomleftV) {
			Shape += 1 * ((rightV >= lowV)&&(rightV < highV));
			}
		else {
			Shape += 1 * ((rightV > lowV)&&(rightV < highV));
			}
		Shape += 2 * ((topleftV > lowV)&&(topleftV <= highV));
		Shape += 4 * ((bottomleftV >= lowV)&&(bottomleftV < highV));
		if ((bMidPoint)) {
			if (bMidHigh) {
				Shape += 8 * ((midV > lowV)&&(midV <= highV));
				}
			else {
				Shape += 8 * ((midV >= lowV)&&(midV < highV));
				}
			}
		}

	switch (Shape) {
		case -1:
			if (fData.bAdaptiveMesh) {
				if(!Deformed.Point(U, V)->bFilled){
					GetBlockSize(U, V, usize, vsize, VBlockStop);
					AddBlock(U, V, usize, vsize);				
					}
				}
			else {
				AddBlock(U, V, 1, 1);
				}
			break;

		case 0://mid: done
			//make sure we don't have nothing because
			//we were out of range
			if ((highest > highV)&&(lowest < lowV)) {
				AddBlock(U, V, 1, 1);
				}
			break;
		case 1://mid: done
			if (bottomleftV > rightV) {
				AddFacet(right
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(midV < rightV)){
					if (rightV == lowV) {
						AddFacet(right
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, right);
						}
					}
				}
			else 
			if (topleftV < rightV){
				AddFacet(right
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				if ((bMidPoint)&&(midV > rightV)) {
					if (rightV == highV) {
						AddFacet(right
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(right
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				if ((rightV == lowV)||(rightV == highV)) {
					AddBlock(U, V, 1, 1);
					}
				else {//todo: change to AddQuad
					AddFacet(right
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(right
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(right
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;
		case 2://mid: done
			if (rightV < topleftV) {
				AddFacet(topleft
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				if ((bMidPoint)&&(midV > topleftV)) {
					if (topleftV == highV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, topleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				if (topleftV == highV) {
					AddBlock(U, V, 1, 1);
					}
				else {
					if ((highV - topleftV)>(topleftV - lowV)) {
						//closer to bottom
						AddQuad(Deformed.GetRightMost(U, V)
							, topleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, topleft);
						}
					else {
						//closer to top
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}
			break;
		case 3://mid: done
			if (rightV == lowV) {
				AddFacet(right
					, Deformed.GetLeftMost(U, V)
					, topleft);
				if ((bMidPoint)&&(bMidHigh)) {
					if (topleftV == highV) {
						AddFacet(right
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(right
							, topleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else {
				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topleft
					, right);
				if ((bMidPoint)&&(bMidHigh)) {
					if ((rightV == highV)&&(topleftV == highV)) {
						//nothing to do
						}
					else if (rightV == highV) {
						AddFacet(right
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else if (topleftV == highV) {
						AddFacet(right
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddQuad(right
							, topleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			break;
		case 4://mid: done
			if (rightV > bottomleftV) {
				AddFacet(bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(midV < bottomleftV)) {
					if (bottomleftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, bottomleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, Deformed.GetRightMost(U, V + 1)); 
						}
					}
				}
			else {
				if (bottomleftV == lowV) {
					AddBlock(U, V, 1, 1);
					}
				else {
					if ((highV - bottomleftV) > (bottomleftV - lowV)) {
						//closer to bottom
						AddQuad(Deformed.GetRightMost(U, V)
							, bottomleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1)); 
						AddFacet(Deformed.GetRightMost(U, V)
							,  Deformed.GetLeftMost(U, V)
							, bottomleft);
						}
					else {
						//closer to top
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, Deformed.GetRightMost(U, V + 1)); 
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, bottomleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}

			break;
		case 5://mid: done
			if (rightV == highV) {
				AddFacet(right
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1));
				if ((bMidPoint)&&(bMidHigh==false)) {
					if (bottomleftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, bottomleft
							, right);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, right);
						}
					}
				}
			else {
				AddQuad(right
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				if ((bMidPoint)&&(bMidHigh==false)) {
					if ((rightV == lowV)&&(bottomleftV == lowV)) {
						//nothing to do
						}
					else if (rightV == lowV){
						AddFacet(right
							, Deformed.GetLeftMost(U, V) 
							, bottomleft);
						}
					else if (bottomleftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V) 
							, bottomleft
							, right);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V) 
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, right);
						}
					}
				}
			break;
		case 6://mid: done
			if (rightV < bottomleftV) {
				if (bottomleftV == lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, bottomleft
						, topleft);
					}
				else {
					AddQuad(bottomleft
						, topleft
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				if ((bMidPoint)&&(midV > topleftV)) {
					if (topleftV == highV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, topleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}

					}
				}
			else {
				if (topleftV == highV) {
					AddFacet(bottomleft
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddQuad(bottomleft
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				if ((bMidPoint)&&(midV < bottomleftV)) {
					if (bottomleftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, bottomleft
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			break;
		case 7://mid: done
			AddFacet(right
				, bottomleft
				, topleft);
			if (bMidPoint) {
				if (bMidHigh) {
					AddQuad(right
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)); 
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft
						, right);
					}
				}
			break;
		case 8://mid: done
			if ((rightV > midV)&&(bottomleftV > midV)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1));
				}
			else if ((topleftV < midV)&&(rightV < midV)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, Upt[1]);
				}
			else if ((midV == lowV)||(midV == highV)) {
				AddBlock(U, V, 1, 1);
				}
			else {
				if ((highV - midV) > (midV - lowV)) {
					//closer to bottom
					AddQuad(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]);
					}
				else {
					//closer to top
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Upt[1]);
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, Upt[1]
						, Deformed.GetLeftMost(U, V + 1));
					}
				}
			break;
		case 9://mid: done
			if (bMidHigh) {
				FillTwoPointRightHand(U, V, right, Upt[1]
					, (bottomleftV > rightV), (topleftV < rightV), (midV == highV), (rightV == lowV));
				}
			else {//bMidHigh == false
				FillTwoPointRightHand(U, V, Upt[1], right
					, (bottomleftV > rightV), (topleftV < rightV), (rightV == highV), (midV == lowV));
				}
			break;
		case 10://mid: done
			if (rightV > topleftV) {
				if (bMidHigh) {
					AddQuad(Deformed.GetRightMost(U, V)
						, topleft
						, Upt[1]
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topleft);
					if (midV != highV) {
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else {
					if (midV == highV) {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, topleft
							, Upt[1]);
						}
					else {
						AddQuad(Upt[1]
							, Deformed.GetLeftMost(U, V)
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						if (midV != lowV) {
							AddFacet(Deformed.GetRightMost(U, V)
								, Deformed.GetLeftMost(U, V)
								, Upt[1]);
							}
						}
						
					if (topleftV != highV) {
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}
			else {
				if (midV == lowV) {
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, topleft);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topleft
						, Upt[1]);
					}
				}
			break;

		case 11://mid: done
			if (bMidHigh) {
				AddQuad(right
					, Deformed.GetLeftMost(U, V)
					, topleft
					, Upt[1]);
				if (rightV != lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, right);
					}

				}
			else {
				AddQuad(Upt[1]
					, Deformed.GetLeftMost(U, V)
					, topleft
					, right);
				if (midV != lowV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Upt[1]);
					}
				}
			break;

		case 12://mid: done
			if (rightV < bottomleftV) {
				if (bMidHigh) {
					if (midV == lowV) {
						AddQuad(Upt[1]
							, bottomleft
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, bottomleft
							, Deformed.GetLeftMost(U, V + 1)
							, Upt[1]);
						if (midV != highV) {
							AddFacet(Upt[1]
								, Deformed.GetLeftMost(U, V + 1)
								, Deformed.GetRightMost(U, V + 1));
							}
						}
					if (bottomleftV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft);
						}
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Upt[1]
						, bottomleft
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					if (midV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]);
						}
					}
				}
			else {
				if (midV == highV) {
					AddFacet(bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Upt[1]);
					}
				else {
					AddQuad(Upt[1]
						, bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;

		case 13://mid: done
			if (bMidHigh) {
				AddQuad(right
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Upt[1]);
				if (midV != highV) {
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else {
				AddQuad(Upt[1]
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, right);
				if (rightV != highV) {
					AddFacet(right
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;

		case 14://mid: done
			if (rightV > topleftV) {
				if (bMidHigh) {
					AddQuad(Deformed.GetRightMost(U, V + 1)
						, bottomleft
						, topleft
						, Upt[1]);
					if (midV != highV) {
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else {
					if (midV == highV) {
						if (topleftV = highV) {
							AddFacet(Upt[1]
								, bottomleft
								, topleft);
							}
						else {
							AddQuad(Upt[1]
								, bottomleft
								, topleft
								, Deformed.GetLeftMost(U, V + 1));
							}
						}
					else {
						AddQuad(Upt[1]
							, bottomleft
							, topleft
							, Deformed.GetRightMost(U, V + 1));
						if (topleftV != highV) {
							AddFacet(Deformed.GetRightMost(U, V + 1)
								, topleft
								, Deformed.GetLeftMost(U, V + 1));
							}
						}

					}
				}
			else {
				if (bMidHigh) {
					if (midV == lowV) {
						if (bottomleftV == lowV) {
							AddFacet(Upt[1]
								, bottomleft
								, topleft);
							}
						else {//bottomleftV != lowV
							AddQuad(Upt[1]
								, Deformed.GetLeftMost(U, V)
								, bottomleft
								, topleft);
							}
						}
					else {//midV != lowV
						AddQuad(Deformed.GetRightMost(U, V)
							, bottomleft
							, topleft
							, Upt[1]);
						if (bottomleftV != lowV) {
							AddFacet(Deformed.GetRightMost(U, V)
								, Deformed.GetLeftMost(U, V)
								, bottomleft);
							}
						}
					}
				else {//Mid low
					AddQuad(Deformed.GetRightMost(U, V)
						, Upt[1]
						, bottomleft
						, topleft);
					if (midV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]);
						}
					}
				}
			break;

		case 15://mid: done
			if (bMidHigh) {
				AddQuad(right
					, bottomleft
					, topleft
					, Upt[1]);
				}
			else {
				AddQuad(Upt[1]
					, bottomleft
					, topleft
					, right);
				}
			break;
		}
	}

	}

void DefMap::DoTriangle(int32 U, DefPoint* pt1, DefPoint* pt2, DefPoint* pt3) {

int32 Shape = 0;
int32 V, VBlockStop, VBlockStart;
int32 lowV, highV;
int32 pt1V = intcompare(pt1->vertex.fUV.y * fData.lV)
	, pt2V = intcompare(pt2->vertex.fUV.y * fData.lV)
	, pt3V = intcompare(pt3->vertex.fUV.y * fData.lV)
	, lowest, highest;

VBlockStart = floor(pt1->vertex.fUV.y * fData.lV);
lowest = pt1V;
if (pt2V < intcompare(VBlockStart)) {
	VBlockStart = floor(filter(pt2->vertex.fUV.y * fData.lV));
	lowest = pt2V;
	}
if (pt3V < intcompare(VBlockStart)) {
	VBlockStart = floor(filter(pt3->vertex.fUV.y * fData.lV));
	lowest = pt3V;
	}


VBlockStop = ceil(pt1->vertex.fUV.y * fData.lV);
highest = pt1V;
if (pt2V > intcompare(VBlockStop)) {
	VBlockStop = ceil(filter(pt2->vertex.fUV.y * fData.lV));
	highest = pt2V;
	}
if (pt3V > intcompare(VBlockStop)) {
	VBlockStop = ceil(filter(pt3->vertex.fUV.y * fData.lV));
	highest = pt3V;
	}

for (V = VBlockStart; V < VBlockStop; V++) {
	lowV = intcompare(V);
	highV = intcompare(V + 1);
	Shape = 0;
	if (highest == pt1V) {
		Shape += 1 * ((pt1V > lowV)&&(pt1V <= highV));
		}
	else if (lowest == pt1V) {
		Shape += 1 * ((pt1V >= lowV)&&(pt1V < highV));
		}
	else {
		Shape += 1 * ((pt1V > lowV)&&(pt1V < highV));
		}
	if (highest == pt2V) {
		Shape += 2 * ((pt2V > lowV)&&(pt2V <= highV));
		}
	else if (lowest == pt2V) {
		Shape += 2 * ((pt2V >= lowV)&&(pt2V < highV));
		}
	else {
		Shape += 2 * ((pt2V > lowV)&&(pt2V < highV));
		}
	if (highest == pt3V) {
		Shape += 4 * ((pt3V > lowV)&&(pt3V <= highV));
		}
	else if (lowest == pt3V) {
		Shape += 4 * ((pt3V >= lowV)&&(pt3V < highV));
		}
	else {
		Shape += 4 * ((pt3V > lowV)&&(pt3V < highV));
		}

	switch (Shape) {
		case 0://mid: done
			//make sure we don't have nothing because
			//we were out of range
			if ((highest > highV)&&(lowest < lowV)) {
				AddBlock(U, V, 1, 1);
				}
			break;
		case 1:
			if ((pt2V< pt1V)&&(pt3V< pt1V)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, pt1);
				}
			else if ((pt2V > pt1V)&&(pt3V > pt1V)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, pt1
					, Deformed.GetLeftMost(U, V + 1));
				}
			else {
				float fFarAlong, fLineU;
				fFarAlong = (float)(pt1V - lowV) / (float)(highV - lowV);
				fLineU =  Deformed.GetLeftMost(U, V)->vertex.fUV.x
					+ fFarAlong * 
					( Deformed.GetLeftMost(U, V + 1)->vertex.fUV.x 
					-  Deformed.GetLeftMost(U, V)->vertex.fUV.x);
				if (fLineU < pt1->vertex.fUV.x) {
					FillSinglePointRightHand(U, V, pt1
						, ((highV - pt1V) > (pt1V - lowV))
						, (pt1V == highV), (pt1V == lowV));
					}
				else {
					FillSinglePointLeftHand(U, V, pt1
						, ((highV - pt1V) > (pt1V - lowV))
						, (pt1V == highV), (pt1V == lowV));
					}
				}
			break;
		case 2:
			if ((pt1V < pt2V)&&(pt3V< pt2V)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, pt2);
				}
			else if ((pt1V > pt2V)&&(pt3V > pt2V)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, pt2
					, Deformed.GetLeftMost(U, V + 1));
				}
			else {
				float fFarAlong, fLineU;
				fFarAlong = (float)(pt2V - lowV) / (float)(highV - lowV);
				fLineU =  Deformed.GetLeftMost(U, V)->vertex.fUV.x
					+ fFarAlong * 
					( Deformed.GetLeftMost(U, V + 1)->vertex.fUV.x 
					-  Deformed.GetLeftMost(U, V)->vertex.fUV.x);
				if (fLineU < pt2->vertex.fUV.x) {
					FillSinglePointRightHand(U, V, pt2
						, ((highV - pt2V) > (pt2V - lowV))
						, (pt2V == highV), (pt2V == lowV));
					}
				else {
					FillSinglePointLeftHand(U, V, pt2
						, ((highV - pt2V) > (pt2V - lowV))
						, (pt2V == highV), (pt2V == lowV));
					}
				}
			break;
		case 3:
			if (pt3V < pt1V) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, pt1
						, pt2);
				}
			else {
					AddQuad(pt1
						, pt2
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 4:
			if ((pt2V< pt3V)&&(pt1V< pt3V)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, pt3);
				}
			else if ((pt2V > pt3V)&&(pt1V > pt3V)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, pt3
					, Deformed.GetLeftMost(U, V + 1));
				}
			else {
				float fFarAlong, fLineU;
				fFarAlong = (float)(pt3V - lowV) / (float)(highV - lowV);
				fLineU =  Deformed.GetLeftMost(U, V)->vertex.fUV.x
					+ fFarAlong * 
					( Deformed.GetLeftMost(U, V + 1)->vertex.fUV.x 
					-  Deformed.GetLeftMost(U, V)->vertex.fUV.x);
				if (fLineU < pt3->vertex.fUV.x) {
					FillSinglePointRightHand(U, V, pt3
						, ((highV - pt3V) > (pt3V - lowV))
						, (pt3V == highV), (pt3V == lowV));
					}
				else {
					FillSinglePointLeftHand(U, V, pt3
						, ((highV - pt3V) > (pt3V - lowV))
						, (pt3V == highV), (pt3V == lowV));
					}
				}
			break;
		case 5:
			if (pt2V < pt1V) {
				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, pt3
					, pt1);
				}
			else {
				AddQuad(pt3
					, pt1
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 6:
			if (pt1V < pt3V) {
				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, pt2
					, pt3);
				}
			else {
				AddQuad(pt2
					, pt3
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 7:
			AddFacet(pt1, pt2, pt3);
			break;
		}
	}
	}

void DefMap::ApplyDeformationToUStrip(int32 U
			, boolean bMidPoint){

DefPoint* bottomright = Deformed.GetBottom(U + 1);
DefPoint* bottomleft = Deformed.GetBottom(U);
DefPoint* topleft = Deformed.GetTop(U);
DefPoint* topright = Deformed.GetTop(U + 1);

int32 Shape = 0;
int32 V, VBlockStop, VBlockStart;
int32 lowV, highV;
int32 bottomleftV = intcompare(bottomleft->vertex.fUV.y * fData.lV)
	, topleftV = intcompare(topleft->vertex.fUV.y * fData.lV)
	, bottomrightV = intcompare(bottomright->vertex.fUV.y * fData.lV)
	, toprightV = intcompare(topright->vertex.fUV.y * fData.lV)
	, midV = intcompare(Upt[1]->vertex.fUV.y * fData.lV)
	, lowest, highest;

uint32 vsize = 1, usize = 1;

VBlockStart = floor(bottomleft->vertex.fUV.y * fData.lV);
lowest = bottomleftV;
if (bottomrightV < intcompare(VBlockStart)) {
	VBlockStart = floor(filter(bottomright->vertex.fUV.y * fData.lV));
	lowest = bottomrightV;
	}
if ((bMidPoint)
	&&(midV < intcompare(VBlockStart))) {
	VBlockStart = floor(filter(Upt[1]->vertex.fUV.y * fData.lV));
	lowest = midV;
	}


VBlockStop = ceil(topleft->vertex.fUV.y * fData.lV);
highest = topleftV;
if (toprightV > intcompare(VBlockStop)) {
	VBlockStop = ceil(filter(topright->vertex.fUV.y * fData.lV));
	highest = toprightV;
	}
if ((bMidPoint)
	&&(midV > intcompare(VBlockStop))) {
	VBlockStop = ceil(filter(Upt[1]->vertex.fUV.y * fData.lV));
	highest = midV;
	}

for (V = VBlockStart; V < VBlockStop; V++) {
	DefPoint* blockbr = Deformed.Point(U + 1, V);
	DefPoint* blockbl = Deformed.Point(U, V);
	DefPoint* blocktl = Deformed.Point(U, V + 1);
	DefPoint* blocktr = Deformed.Point(U + 1, V + 1);
	lowV = intcompare(V);
	highV = intcompare(V + 1);

	if ((blocktr != NULL)
		&&
		(blockbr != NULL)
		&&
		(blockbl != NULL)
		&&
		(blocktl != NULL)
		&&
		(blockbr->bHasData == true)
		&&
		(blockbl->bHasData == true)
		&&
		(blocktl->bHasData == true)
		&&
		(blocktr->bHasData == true)) {
		Shape = -1;
		}
	else {
		Shape = 0;
		Shape += 1 * ((bottomleftV >= lowV)&&(bottomleftV < highV));
		Shape += 2 * ((bottomrightV >= lowV)&&(bottomrightV < highV));
		Shape += 4 * ((topleftV > lowV)&&(topleftV <= highV));
		Shape += 8 * ((toprightV > lowV)&&(toprightV <= highV));

		if ((bMidPoint)) {
			if (bMidHigh) {
				Shape += 16 * ((midV > lowV)&&(midV <= highV));
				}
			else {
				Shape += 16 * ((midV >= lowV)&&(midV < highV));
				}
			}
		}
	switch (Shape) {
		case -1:
			if (fData.bAdaptiveMesh) {
				if(!Deformed.Point(U, V)->bFilled){
					GetBlockSize(U, V, usize, vsize, VBlockStop);
					AddBlock(U, V, usize, vsize);				
					}
				}
			else {
				AddBlock(U, V, 1, 1);
				}
				break;
		case 0:
			//edge: done, nothing changes
			//make sure we don't have nothing because
			//we were out of range
			if ((highest > highV)&&(lowest < lowV)) {
				AddBlock(U, V, 1, 1);
				}
			break;
		case 1:
			//edge: done
			if (bottomrightV < bottomleftV) {
				if (bottomleftV == lowV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Deformed.GetLeftMost(U, V)
						, bottomleft
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}

				}
			else 
			if ((bMidPoint)&&(bMidHigh == false)){
				if (midV > bottomleftV) {
					AddFacet(Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						, bottomleft);
					}
				else if (bottomleftV == lowV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(bottomleft
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(bottomleft
						, Deformed.GetRightMost(U, V + 1)
						, Deformed.GetRightMost(U, V));
					AddFacet(bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else {
				AddFacet(Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1)
					, bottomleft);

				}
			break;
		case 2:
			//edge: done
			if (bottomleftV < bottomrightV) {
				if (bottomrightV == lowV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, bottomright);
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else 
			if ((bMidPoint)&&(bMidHigh == false)){
				if (midV > bottomrightV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				else if (bottomrightV == lowV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(bottomright
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else {
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 3:
			//edge: done
			AddQuad(bottomright
				, bottomleft
				, Deformed.GetLeftMost(U, V + 1)
				, Deformed.GetRightMost(U, V + 1));
			if ((bMidPoint)&&(bMidHigh == false)) {
				if ((bottomrightV == lowV)&&(bottomleftV == lowV)) {
					//do nothing we fill the chunck
					}
				else if (bottomrightV == lowV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					}
				else if (bottomleftV == lowV) {
					AddFacet(Deformed.GetRightMost(U, V )
						, bottomleft
						, bottomright);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V )
						, Deformed.GetLeftMost(U, V)
						, bottomleft
						, bottomright
						);
					}
				}
			break;
		case 4:
			//edge: done
			if (toprightV > topleftV) {
				if (topleftV == highV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topleft);
					AddFacet(Deformed.GetRightMost(U, V)
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else 
			if ((bMidPoint)&&(bMidHigh)){
				if (midV < topleftV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topleft);
					}
				else if (topleftV == highV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(topleft
						, Deformed.GetRightMost(U, V + 1)
						, Deformed.GetRightMost(U, V));
					AddFacet(topleft
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				}
			else {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topleft);
				}
			break;
		case 5:
			//edge: done
			if ((toprightV > topleftV)
				&&(bottomrightV > bottomleftV)) {
				if ((bMidPoint)&&(midV < lowV)) {
					AddQuad(Deformed.GetRightMost(U, V)
						, bottomleft
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					if ((topleftV == highV)
						&&(bottomleftV == lowV)) {
						//nothing more to do
						}
					else if (topleftV == highV){
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft);
						}
					else if (bottomleftV == lowV){
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft);
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else {
					if (topleftV == highV) {
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, bottomleft
							, topleft);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V + 1)
							, bottomleft
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				}
			else if ((toprightV < topleftV)
				&&(bottomrightV < bottomleftV)) {
				if ((bMidPoint)&&(midV > highV)) {
					AddQuad(Deformed.GetRightMost(U, V)
						, bottomleft
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					if ((topleftV == highV)
						&&(bottomleftV == lowV)) {
						//nothing more to do
						}
					else if (topleftV == highV){
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft);
						}
					else if (bottomleftV == lowV){
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft);
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, topleft
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else {
					if (bottomleftV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, bottomleft
							, topleft);
						}
					else {
						AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, bottomleft
							, topleft);
						}
					}
				}
			else {
				AddQuad(Deformed.GetRightMost(U, V)
					, bottomleft
					, topleft
					, Deformed.GetRightMost(U, V + 1));
				if ((topleftV == highV)&&(bottomleftV == lowV)) {
					//nothing more to do
					}
				else if (topleftV == highV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					}
				else if (bottomleftV == lowV) {
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					AddFacet(Deformed.GetRightMost(U, V + 1)
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					}
				}
			break;
		case 6:
			//edge: done
			AddQuad(bottomright
				, Deformed.GetLeftMost(U, V)
				, topleft
				, Deformed.GetRightMost(U, V + 1));
			if ((topleftV == highV)&&(bottomrightV == lowV)) {
				//nothing more to do
				}
			else if (topleftV == highV) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomright);
				}
			else if (bottomrightV == lowV) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, topleft
					, Deformed.GetLeftMost(U, V + 1));
				}
			else {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, topleft
					, Deformed.GetLeftMost(U, V + 1));
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomright);
				}
			break;
		case 7:
			//edge: done
			AddQuad(bottomright
				, bottomleft
				, topleft
				, Deformed.GetRightMost(U, V + 1));
			if (topleftV != highV) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, topleft
					, Deformed.GetLeftMost(U, V + 1));
				}
			if ((bMidPoint)&&(bMidHigh == false)) {
				if ((bottomleftV == lowV)&&(bottomrightV == lowV)) {
					//nothing left to do
					}
				else if (bottomleftV == lowV) {
					AddFacet(bottomright
						, Deformed.GetRightMost(U, V)
						, bottomleft);
					}
				else if (bottomrightV == lowV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft
						, bottomright);
					}
				}
			break;
		case 8:
			//edge: done
			if (topleftV > toprightV) {
				if (toprightV == highV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topright);
					AddFacet(Deformed.GetLeftMost(U, V)
						, Deformed.GetRightMost(U, V + 1)
						, topright);
					AddFacet(Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else 
			if ((bMidPoint)&&(bMidHigh)) {
				if (midV < toprightV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topright);
					}
				else if (toprightV == highV) {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1)
						);
					}
				else {
					AddFacet(topright
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(topright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(topright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else{
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topright);
				}
			break;
		case 9:
			//edge: done
			AddQuad(Deformed.GetRightMost(U, V)
				, bottomleft
				, Deformed.GetLeftMost(U, V + 1)
				, topright);
			if ((toprightV == highV)&&(bottomleftV == lowV)) {
				//nothing left to do
				}
			else if (toprightV == highV) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomleft);
				}
			else if (bottomleftV == lowV) {
				AddFacet(topright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			else {
				AddFacet(topright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomleft);
				}
			break;
		case 10:
			//edge: done
			if ((bottomleftV > bottomrightV)
				&&(topleftV > toprightV)) {
				if ((bMidPoint)&&(midV < lowV)) {
					AddQuad(bottomright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, topright);
					if ((toprightV == highV)
						&&(bottomrightV == lowV)) {
						//nothing more to do
						}
					else if (toprightV == highV) {
						AddFacet(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));
						}
					else if (bottomrightV == lowV) {
						AddFacet(topright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddFacet(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));
						AddFacet(topright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}

					}
				else {
					if (toprightV == highV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V + 1)
							, topright);
						}
					else {
						AddQuad(bottomright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1)
							, topright);
						}
					}
				}
			else if ((bottomleftV < bottomrightV)
				&&(topleftV < toprightV)) {
				if ((bMidPoint)&&(midV > highV)) {
					AddQuad(bottomright
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1)
						, topright);
					if ((toprightV == highV)
						&&(bottomrightV == lowV)) {
						//nothing more to do
						}
					else if (toprightV == highV) {
						AddFacet(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));
						}
					else if (bottomrightV == lowV) {
						AddFacet(topright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddFacet(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));
						AddFacet(topright
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				else {
					if (bottomrightV == lowV) {
						AddFacet(bottomright
							, Deformed.GetLeftMost(U, V)
							, topright);
						}
					else {
						AddQuad(bottomright
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, topright);
						}
					}
				}
			else {
				AddQuad(bottomright
					, Deformed.GetLeftMost(U, V)
					, Deformed.GetLeftMost(U, V + 1)
					, topright);
				if ((toprightV == highV)&&(bottomrightV == lowV)) {
					}
				else if (toprightV == highV) {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomright);
					}
				else if (bottomrightV == lowV) {
					AddFacet(topright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomright);
					AddFacet(topright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;
		case 11:
			//edge: done
			AddQuad(bottomright
				, bottomleft
				, Deformed.GetLeftMost(U, V + 1)
				, topright);
			if (toprightV != highV) {
				AddFacet(topright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			if ((bMidPoint)&&(bMidHigh == false)) {
				if ((bottomleftV == lowV)&&(bottomrightV == lowV)) {
					//nothing left to do
					}
				else if (bottomleftV == lowV) {
					AddFacet(bottomright
						, Deformed.GetRightMost(U, V)
						, bottomleft);
					}
				else if (bottomrightV == lowV) {
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft
						, bottomright);
					}
				}
			break;
		case 12:
			//edge: done 
			AddQuad(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, topleft
				, topright);
			if ((bMidPoint)&&(bMidHigh)) {
				if ((toprightV == highV)&&(topleftV == highV)) {
					}
				else if (toprightV == highV) {
					AddFacet(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					}
				else if (topleftV == highV) {
					AddFacet(topright
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddQuad(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}

			break;
		case 13:
			//edge: done
			AddQuad(Deformed.GetRightMost(U, V)
				, bottomleft
				, topleft
				, topright);
			if (bottomleftV != lowV) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomleft);
				}
			if ((bMidPoint)&&(bMidHigh)) {
				if ((topleftV == highV)&&(toprightV == highV)) {
					//nothing more to do
					}
				else if (topleftV == highV){
					AddFacet(topright
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					}
				else if (toprightV == highV){
					AddFacet(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					}
				else {
					AddQuad(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;
		case 14:
			//edge: done
			AddQuad(bottomright
				, Deformed.GetLeftMost(U, V)
				, topleft
				, topright);
			if (bottomrightV != lowV) {
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V)
					, Deformed.GetRightMost(U, V));
				}
			if ((bMidPoint)&&(bMidHigh)) {
				if ((topleftV == highV)&&(toprightV == highV)) {
					//nothing more to do
					}
				else if (topleftV == highV){
					AddFacet(topright
						, topleft
						, Deformed.GetRightMost(U, V + 1));
					}
				else if (toprightV == highV){
					AddFacet(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					}
				else {
					AddQuad(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			break;
		case 15:
			//edge: done, nothing to do
			AddQuad(bottomright
				, bottomleft
				, topleft
				, topright);
			if (bMidPoint) {
				if (bMidHigh) {
					AddQuad(topright
						, topleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddQuad(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, bottomleft
						, bottomright);
					}
				}
			break;
		case 16:
			//edge: done
			if ((toprightV < midV)&&(topleftV < midV)) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, Upt[1]);
				}
			else if ((bottomrightV > midV)&&(bottomleftV > midV)) {
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1));
				}
			else if (bMidHigh) {
				if (toprightV > midV) {
					AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]
							, Deformed.GetRightMost(U, V + 1));
					if (midV != highV) {
						AddFacet (Deformed.GetRightMost(U, V + 1)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1));
						}
					}
				else if (topleftV > midV) {
					AddQuad(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Upt[1]);
					if (midV != highV) {
						AddFacet(Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					}
				}
			else /*bMidHigh = false*/{
				if (bottomrightV < midV) {
					AddQuad(Deformed.GetRightMost(U, V)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
					if (midV != lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Upt[1]);
						}
					}
				else if (bottomleftV < midV) {
					AddQuad(Upt[1]
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
					if (midV != lowV) {
						AddFacet(Upt[1]
							, Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V));
						}
					}
				}

			break;
		case 17:
			//edge: done
			if (bottomrightV < bottomleftV) {
				if (bMidHigh) {
					FillTwoPoint(U, V, Upt[1], bottomleft
						, (midV == highV), (midV == lowV)
						, (bottomleftV == highV), (bottomleftV == lowV));
					}
				else {
					if (midV == lowV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Upt[1]
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetRightMost(U, V + 1));
						AddFacet(Deformed.GetLeftMost(U, V)
							, Upt[1]
							, Deformed.GetRightMost(U, V + 1));
						}
					AddFacet(Upt[1]
						, bottomleft
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(bottomleft
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else if (midV == highV) {
				AddFacet(bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Upt[1]);
				}
			else {
				AddQuad(Upt[1]
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 18:
			//edge: done
			if (bottomleftV < bottomrightV) {
				if (bMidHigh) {
					FillTwoPoint(U, V, bottomright, Upt[1]
						, (bottomrightV == highV), (bottomrightV == lowV)
						, (midV == highV), (midV == lowV));
					}
				else {
					if (midV == lowV) {
						AddFacet(Upt[1]
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					else {
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Upt[1]);
						AddFacet(Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V));
						}
					AddFacet(bottomright
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, bottomright);
					}
				}
			else if (midV == highV) {
				AddFacet(bottomright
					, Upt[1]
					, Deformed.GetRightMost(U, V + 1));
				}
			else {
				AddQuad(bottomright
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			break;
		case 19:
			//edge
			AddFacet(Upt[1]
				, bottomleft
				, Deformed.GetLeftMost(U, V + 1));
			AddFacet(bottomright
				, Upt[1]
				, Deformed.GetLeftMost(U, V + 1));
			AddFacet(Deformed.GetRightMost(U, V + 1)
				, bottomright
				, Deformed.GetLeftMost(U, V + 1));

			break;
		case 20:
			//edge: done
			if (toprightV > topleftV) {
				if (bMidHigh) {
					if (midV == highV) {
						AddFacet(Deformed.GetRightMost(U, V)
							, Upt[1]
							, Deformed.GetRightMost(U, V + 1));
						}
					else {
						AddFacet(Deformed.GetRightMost(U, V)
							, Upt[1]
							, Deformed.GetLeftMost(U, V + 1));
						AddFacet(Deformed.GetRightMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Deformed.GetRightMost(U, V + 1));
						}
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topleft);
					AddFacet(Deformed.GetRightMost(U, V)
						, topleft
						, Upt[1]);
					}
				else {
					FillTwoPoint(U, V, Upt[1], topleft
						, (midV == highV), (midV == lowV)
						, (topleftV == highV), (topleftV == lowV));
					}
				}
			else if (midV == lowV) {
				AddFacet(Upt[1]
					, Deformed.GetLeftMost(U, V)
					, topleft);
				}
			else {
				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topleft
					, Upt[1]);
				}
			break;
		case 21:
			//edge
			if ((toprightV > topleftV)
				&&(bottomrightV > bottomleftV)) {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V + 1)
						, bottomleft);
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					}
				else {
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					AddFacet(Upt[1]
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else if ((toprightV < topleftV)
				&&(bottomrightV < bottomleftV)) {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				else {
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					AddFacet(Upt[1]
						, topleft
						, Deformed.GetRightMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				}
			else {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V + 1)
						, Deformed.GetRightMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, bottomleft);
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					}
				else {
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V + 1)
						, Deformed.GetRightMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Upt[1]
						, topleft
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Upt[1]
						, bottomleft
						, topleft);
					}
				}
			break;
		case 22:
			//edge
			if (bMidHigh) {
				AddFacet(bottomright
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V)
					, topleft);
				AddFacet(bottomright
					, topleft
					, Upt[1]);
				AddFacet(bottomright
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1));
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				}
			else {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topleft);
				AddFacet(Upt[1]
					, Deformed.GetRightMost(U, V)
					, topleft);
				AddFacet(bottomright
					, Upt[1]
					, topleft);
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, bottomright
					, topleft);
				AddFacet(Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1)
					, topleft);
				}
			break;
		case 23:
			//edge
			if (bMidHigh) {
				AddFacet(bottomright
					, bottomleft
					, topleft);
				AddFacet(bottomright
					, topleft
					, Upt[1]);
				AddFacet(bottomright
					, Upt[1]
					, Deformed.GetLeftMost(U, V + 1));
				AddFacet(bottomright
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));

				}
			else {
				AddFacet(Upt[1]
					, bottomleft
					, topleft);
				AddFacet(Upt[1]
					, topleft
					, Deformed.GetLeftMost(U, V + 1));
				AddFacet(Upt[1]
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				AddFacet(Upt[1]
					, Deformed.GetRightMost(U, V + 1)
					, bottomright);
				}
			break;
		case 24:
			//edge: done
			if (topleftV > toprightV) {
				if (bMidHigh) {
					if (midV == highV) {
						AddFacet(Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1)
							, Upt[1]);
						}
					else {
						AddFacet(Upt[1]
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetRightMost(U, V + 1));
						AddFacet(Deformed.GetRightMost(U, V + 1)
							, Deformed.GetLeftMost(U, V)
							, Deformed.GetLeftMost(U, V + 1));
						}
					AddFacet(Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V)
						, topright);
					AddFacet(topright
						, Deformed.GetLeftMost(U, V)
						, Upt[1]);
					}
				else {
					FillTwoPoint(U, V, topright, Upt[1]
						, (toprightV == highV), (toprightV == lowV)
						, (midV == highV), (midV == lowV));
					}
				}
			else if (midV == lowV) {
				AddFacet(topright
					, Deformed.GetRightMost(U, V)
					, Upt[1]);
				}
			else {

				AddQuad(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, Upt[1]
					, topright);
				}
			break;
		case 25:
			//edge
			if (bMidHigh) {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, bottomleft);
				AddFacet(topright
					, Deformed.GetRightMost(U, V)
					, bottomleft);
				AddFacet(Upt[1]
					, topright
					, bottomleft);
				AddFacet(Deformed.GetRightMost(U, V + 1)
					, Upt[1]
					, bottomleft);
				AddFacet(Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1)
					, bottomleft);

				}
			else {
				AddFacet(Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V)
					, topright);
				AddFacet(Deformed.GetLeftMost(U, V)
					, Upt[1]
					, topright);
				AddFacet(Upt[1]
					, bottomleft
					, topright);
				AddFacet(bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, topright);
				AddFacet(Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1)
					, topright);
				}
			break;
		case 26:
			//edge
			if ((bottomleftV > bottomrightV)
				&&(topleftV > toprightV)) {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, bottomright
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V + 1)
						, topright);
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				}
			else if ((bottomleftV < bottomrightV)
				&&(topleftV < toprightV)) {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, bottomright
						, Deformed.GetRightMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				else {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, topright);
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				}
			else {
				if (bMidHigh) {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, bottomright
						, Deformed.GetRightMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					}
				else {
					AddFacet(Upt[1]
						, topright
						, bottomright);
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V + 1)
						, topright);
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V + 1)
						, Deformed.GetRightMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetLeftMost(U, V)
						, Deformed.GetLeftMost(U, V + 1));
					AddFacet(Upt[1]
						, Deformed.GetRightMost(U, V)
						, Deformed.GetLeftMost(U, V));
					}
				}
			break;
		case 27:
			//edge
			if (bMidHigh) {
				AddFacet(bottomleft
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				AddFacet(bottomleft
					, Deformed.GetRightMost(U, V + 1)
					, Upt[1]);
				AddFacet(bottomleft
					, Upt[1]
					, topright);
				AddFacet(bottomleft
					, topright
					, bottomright);
				}
			else {
				AddFacet(Upt[1]
					, bottomleft
					, Deformed.GetLeftMost(U, V + 1));
				AddFacet(Upt[1]
					, Deformed.GetLeftMost(U, V + 1)
					, Deformed.GetRightMost(U, V + 1));
				AddFacet(Upt[1]
					, Deformed.GetRightMost(U, V + 1)
					, topright);
				AddFacet(Upt[1]
					, topright
					, bottomright);

				}

			break;
		case 28:
			//edge
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, topright);
			AddFacet(topright
				, Deformed.GetLeftMost(U, V)
				, Upt[1]);
			AddFacet(Upt[1]
				, Deformed.GetLeftMost(U, V)
				, topleft);
			break;
		case 29:
			//edge
			if (bMidHigh) {
				AddFacet(Upt[1]
					, topright
					, Deformed.GetRightMost(U, V));
				AddFacet(Upt[1]
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				AddFacet(Upt[1]
					, Deformed.GetLeftMost(U, V)
					, bottomleft);
				AddFacet(Upt[1]
					, bottomleft
					, topleft);
				}
			else {
				AddFacet(topright
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				AddFacet(topright
					, Deformed.GetLeftMost(U, V)
					, Upt[1]);
				AddFacet(topright
					, Upt[1]
					, bottomleft);
				AddFacet(topright
					, bottomleft
					, topleft);
				}
			break;
		case 30:
			//edge
			if (bMidHigh) {
				AddFacet(Upt[1]
					, topright
					, bottomright);
				AddFacet(Upt[1]
					, bottomright
					, Deformed.GetRightMost(U, V));
				AddFacet(Upt[1]
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				AddFacet(Upt[1]
					, Deformed.GetLeftMost(U, V)
					, topleft);
				}
			else {
				AddFacet(topleft
					, topright
					, bottomright);
				AddFacet(topleft
					, bottomright
					, Upt[1]);
				AddFacet(topleft
					, Upt[1]
					, Deformed.GetRightMost(U, V));
				AddFacet(topleft
					, Deformed.GetRightMost(U, V)
					, Deformed.GetLeftMost(U, V));
				}
			break;
		case 31:
			//edge
			if (bMidHigh) {
				AddFacet(Upt[1]
					, topright
					, bottomright);
				AddFacet(Upt[1]
					, bottomright
					, bottomleft);
				AddFacet(Upt[1]
					, bottomleft
					, topleft);
				}
			else {
				AddFacet(Upt[1]
					, bottomleft
					, topleft);
				AddFacet(Upt[1]
					, topleft
					, topright);
				AddFacet(Upt[1]
					, topright
					, bottomright);
				}
			break;

		}//end switch
	}//end loop

	}//end ApplyDeformationToUStrip

void DefMap::AddQuad(DefPoint* bottomright, DefPoint* bottomleft
					 , DefPoint* topleft, DefPoint* topright
					 ) {
	switch(fData.lSplitMethod) {
		case SPLIT_SMART: {
			DefPoint originalbrtotl;
			DefPoint originalbltotr;
			DefPoint brtotl;
			DefPoint bltotr;
			FillPoint(.5, bottomright->original
				, topleft->original, originalbrtotl.vertex);
			DeformPoint(&originalbrtotl);
			FillPoint(.5, bottomleft->original
				, topright->original, originalbltotr.vertex);
			DeformPoint(&originalbltotr);
			FillPoint(.5, bottomright->vertex, topleft->vertex, brtotl.vertex);
			FillPoint(.5, bottomleft->vertex, topright->vertex, bltotr.vertex);
			if (sqr(bltotr.vertex.fVertex, originalbltotr.vertex.fVertex) 
				< sqr(brtotl.vertex.fVertex, originalbrtotl.vertex.fVertex)) {
				AddFacet(bottomright
					, bottomleft
					, topright);
				AddFacet(bottomleft
					, topleft
					, topright);
				}
			else {
				AddFacet(bottomright
					, bottomleft
					, topleft);
				AddFacet(bottomright
					, topleft
					, topright);
				}
			}
			break;
		case SPLIT_DUMB: 
			AddFacet(bottomright
				, bottomleft
				, topright);
			AddFacet(bottomleft
				, topleft
				, topright);
			 break;
		case SPLIT_SHORT: 
			if (sqr(bottomleft->vertex.fVertex, topright->vertex.fVertex) 
				< sqr(bottomright->vertex.fVertex, topleft->vertex.fVertex)) {
				AddFacet(bottomright
					, bottomleft
					, topright);
				AddFacet(bottomleft
					, topleft
					, topright);
				}
			else {
				AddFacet(bottomright
					, bottomleft
					, topleft);
				AddFacet(bottomright
					, topleft
					, topright);
				}
				  break;
		}

	}

void DefMap::SetUpGrid() {
	real32 fMinU, fMaxU, fMinV, fMaxV, U10, U21, U20, fFarAlong;
	int32 V, U;

	fMinU = fU[0];
	fMaxU = fU[2];

	fMinV = Upt[0]->vertex.fUV.y;
	fMaxV = Upt[0]->vertex.fUV.y;
	
	if (Upt[1]->vertex.fUV.y < fMinV) {
		fMinV = Upt[1]->vertex.fUV.y;
		}
	else if (Upt[1]->vertex.fUV.y > fMaxV) {
		fMaxV = Upt[1]->vertex.fUV.y;
		}
	
	if (Upt[2]->vertex.fUV.y < fMinV) {
		fMinV = Upt[2]->vertex.fUV.y;
		}
	else if (Upt[2]->vertex.fUV.y > fMaxV) {
		fMaxV = Upt[2]->vertex.fUV.y;
		}
	fMinV *= fData.lV;
	fMaxV *= fData.lV;
	
	Deformed.Allocate(fMinU, fMaxU, fMinV, fMaxV);

	if (Upt[2]->vertex.fUV.x - Upt[0]->vertex.fUV.x == 0) {
		U20 = 0;
		}
	else {
		U20 = 1 / (Upt[2]->vertex.fUV.x - Upt[0]->vertex.fUV.x);
		}
	if (Upt[2]->vertex.fUV.x - Upt[1]->vertex.fUV.x == 0) {
		U21 = 0;
		}
	else{
		U21 = 1 / (Upt[2]->vertex.fUV.x - Upt[1]->vertex.fUV.x);
		}
	if (Upt[1]->vertex.fUV.x - Upt[0]->vertex.fUV.x == 0) {
		U10 = 0;
		}
	else {
		U10 = 1 / (Upt[1]->vertex.fUV.x - Upt[0]->vertex.fUV.x);
		}
	
	//generate the interal points
	for (U = ceil(fMinU); U <= floor(fMaxU); U++) {
		real32 Vtopbottom;
		DefPoint* TopPoint;
		DefPoint* BottomPoint;
		TopPoint = Deformed.GetTop(U);
		BottomPoint = Deformed.GetBottom(U);

		if (bMidHigh == true) {
			if (Upt[0]->vertex.fUV.x == Upt[1]->vertex.fUV.x){
				fFarAlong = (U * fUVInterval.x - Upt[1]->vertex.fUV.x) * U21;
				FillPoint(fFarAlong, Upt[1]->vertex, Upt[2]->vertex, TopPoint->vertex );
				}
			else if (Upt[2]->vertex.fUV.x == Upt[1]->vertex.fUV.x){
				fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x) * U10;
				FillPoint(fFarAlong, Upt[0]->vertex, Upt[1]->vertex, TopPoint->vertex );
				}
			else if(U * fUVInterval.x <= Upt[1]->vertex.fUV.x) {
				fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x)  * U10;
				FillPoint(fFarAlong, Upt[0]->vertex, Upt[1]->vertex, TopPoint->vertex );
				}
			else {
				fFarAlong = (U * fUVInterval.x - Upt[1]->vertex.fUV.x) * U21;
				FillPoint(fFarAlong, Upt[1]->vertex, Upt[2]->vertex, TopPoint->vertex );
				}
			fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x) * U20;
			FillPoint(fFarAlong, Upt[0]->vertex, Upt[2]->vertex, BottomPoint->vertex );
			}
		else {
			if (Upt[0]->vertex.fUV.x == Upt[1]->vertex.fUV.x){
				fFarAlong = (U * fUVInterval.x - Upt[1]->vertex.fUV.x) * U21;
				FillPoint(fFarAlong, Upt[1]->vertex, Upt[2]->vertex, BottomPoint->vertex);
				}
			else if (Upt[2]->vertex.fUV.x == Upt[1]->vertex.fUV.x){
				fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x) * U10;
				FillPoint(fFarAlong, Upt[0]->vertex, Upt[1]->vertex, BottomPoint->vertex );
				}
			else if (U * fUVInterval.x <= Upt[1]->vertex.fUV.x) {
				fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x) * U10;
				FillPoint(fFarAlong, Upt[0]->vertex, Upt[1]->vertex, BottomPoint->vertex );
				}
			else {
				fFarAlong = (U * fUVInterval.x - Upt[1]->vertex.fUV.x) * U21;
				FillPoint(fFarAlong, Upt[1]->vertex, Upt[2]->vertex, BottomPoint->vertex );
				}
			fFarAlong = (U * fUVInterval.x - Upt[0]->vertex.fUV.x) * U20;
			FillPoint(fFarAlong, Upt[0]->vertex, Upt[2]->vertex, TopPoint->vertex );
			}
		if (TopPoint->vertex.fUV.y - BottomPoint->vertex.fUV.y == 0) {
			Vtopbottom = 0;
			}
		else {
			Vtopbottom = 1 / (TopPoint->vertex.fUV.y - BottomPoint->vertex.fUV.y);
			}


		for (V = ceil(BottomPoint->vertex.fUV.y * fData.lV); V <= floor(TopPoint->vertex.fUV.y * fData.lV); V++) {
			DefPoint* thePoint;

			thePoint = Deformed.Point(U,V);

			fFarAlong = (V * fUVInterval.y - BottomPoint->vertex.fUV.y) * Vtopbottom;
			FillPoint(fFarAlong, BottomPoint->vertex , TopPoint->vertex , thePoint->vertex);

			if (fData.lSplitMethod == SPLIT_SMART) {
				//save the original
				thePoint->original = thePoint->vertex;
				}

			DeformPoint(thePoint);
			thePoint->bHasData = true;
			}

		if (fData.lSplitMethod == SPLIT_SMART) {
			//save the original
			TopPoint->original = TopPoint->vertex;
			BottomPoint->original = BottomPoint->vertex;
			}

		//save the V end points
		DeformPoint(TopPoint);
		TopPoint->bHasData = true;
		DeformPoint(BottomPoint);
		BottomPoint->bHasData = true;

		}//end generate the internal points

	}

void DefMap::DeformPoint (DefPoint* thePoint) {

	boolean fullarea = false;
	real32 fValue;
	shadingIn.fUV = thePoint->vertex.fUV;
	shadingIn.fPointLoc = thePoint->vertex.fVertex;
	shadingIn.fNormalLoc = thePoint->vertex.fNormal;
	if (shadingflags.fNeedsNormal) {
		LocalToGlobalVector(fTransform, shadingIn.fNormalLoc, shadingIn.fGNormal);
		}
	if (shadingflags.fNeedsPoint) {
		LocalToGlobal(fTransform, shadingIn.fPointLoc, shadingIn.fPoint);
		}
	shader->GetValue(fValue, fullarea, shadingIn);
	thePoint->displacement = fValue;
	fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
	thePoint->vertex.fVertex +=thePoint->vertex.fNormal * fValue;
	if (fValue != 0) {
		bFlat = false;
		}
	}

MCCOMErr DefMap::HandleEvent(MessageID message, IMFResponder* source, void* data){
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if ((sourceID == 'REFR')&&(message == EMFPartMessage::kMsg_PartValueChanged)) {
		fData.bRefresh = !fData.bRefresh;
		}

	if ((message == kMsg_CUIP_ComponentAttached)||((sourceID == 'RMSL')&&(message == EMFPartMessage::kMsg_PartValueChanged))) {
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

TMCString255 DefMap::GetMasterShaderName() {
	return fData.MasterShader;
	}

void DefMap::FillTwoPointLeftHand(int32 U, int32 V
			, DefPoint* bottomleft, DefPoint* topleft
			, boolean bSwingsUp, boolean bSwingsDown
			, boolean bOnTopEdge, boolean bOnBottomEdge) {
	if (bSwingsUp) {
		if (bOnTopEdge) {
			AddFacet(Deformed.GetRightMost(U, V + 1)
				, bottomleft
				, topleft);
			}
		else {
			AddQuad(Deformed.GetRightMost(U, V + 1)
				, bottomleft
				, topleft
				, Deformed.GetLeftMost(U, V + 1));
			}
		}
	else if (bSwingsDown) {
		if (bOnBottomEdge) {
			AddFacet(Deformed.GetRightMost(U, V)
				, bottomleft
				, topleft);
			}
		else {
			AddQuad(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, bottomleft
				, topleft);
			}
		}
	else {
		AddQuad(Deformed.GetRightMost(U, V)
			, bottomleft
			, topleft
			, Deformed.GetRightMost(U, V + 1));
		if ((bOnBottomEdge)&&(bOnTopEdge)) {
			//nothing more to do
			}
		else if (bOnBottomEdge){
			AddFacet(Deformed.GetRightMost(U, V + 1)
				, topleft
				, Deformed.GetLeftMost(U, V + 1));
			}
		else if (bOnTopEdge){
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, bottomleft);
			}
		else {
			AddFacet(Deformed.GetRightMost(U, V + 1)
				, topleft
				, Deformed.GetLeftMost(U, V + 1));
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, bottomleft);
			}

		}
	}

void DefMap::FillTwoPointRightHand(int32 U, int32 V
			, DefPoint* bottomright, DefPoint* topright
			, boolean bSwingsUp, boolean bSwingsDown
			, boolean bOnTopEdge, boolean bOnBottomEdge) {
	if (bSwingsUp) {
		if (bOnTopEdge) {
			AddFacet(bottomright
				, Deformed.GetLeftMost(U, V + 1)
				, topright);
			}
		else {
			AddQuad(bottomright
				, Deformed.GetLeftMost(U, V + 1)
				, Deformed.GetRightMost(U, V + 1)
				, topright);
			}
		}
	else if (bSwingsDown) {
		if (bOnBottomEdge) {
			AddFacet(bottomright
				, Deformed.GetLeftMost(U, V)
				, topright);
			}
		else {
			AddQuad(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, topright
				, bottomright);
			}
		}
	else {
		AddQuad(bottomright
			, Deformed.GetLeftMost(U, V)
			, Deformed.GetLeftMost(U, V + 1)
			, topright);
		if ((bOnBottomEdge)&&(bOnTopEdge)) {
			//nothing more to do
			}
		else if (bOnBottomEdge){
			AddFacet(topright
				, Deformed.GetLeftMost(U, V + 1)
				, Deformed.GetRightMost(U, V + 1));
			}
		else if (bOnTopEdge){
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, bottomright);
			}
		else {
			AddFacet(topright
				, Deformed.GetLeftMost(U, V + 1)
				, Deformed.GetRightMost(U, V + 1));
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, bottomright);
			}

		}
	}

void DefMap::AddBlock(const uint32& u, const uint32& v
						   ,const  uint32& usize,const uint32& vsize) {
	AddQuad(Deformed.GetRightMost(u + usize - 1, v)
		, Deformed.GetLeftMost(u, v)
		, Deformed.GetLeftMost(u, v + vsize )
		, Deformed.GetRightMost(u + usize - 1, v + vsize)
		);
	}

void DefMap::FillTwoPoint(int32 U, int32 V
			, DefPoint* right, DefPoint* left
			, boolean bRightOnTopEdge, boolean bRightOnBottomEdge
			, boolean bLeftOnTopEdge, boolean bLeftOnBottomEdge){
	if ((bRightOnTopEdge)&&(bLeftOnTopEdge)) {
		//do nothing
		}
	else if (bRightOnTopEdge) {
		AddFacet(right
			, left
			, Deformed.GetLeftMost(U, V + 1));
		}
	else if (bLeftOnTopEdge) {
		AddFacet(right
			, left
			, Deformed.GetRightMost(U, V + 1));
		}
	else {
		AddQuad(right
			, left
			, Deformed.GetLeftMost(U, V + 1)
			, Deformed.GetRightMost(U, V + 1));
		}
	if ((bRightOnBottomEdge)&&(bLeftOnBottomEdge)) {
		//do nothing
		}
	else if (bRightOnBottomEdge) {
		AddFacet(right
			, Deformed.GetLeftMost(U, V)
			, left);
		}
	else if (bLeftOnBottomEdge) {
		AddFacet(Deformed.GetRightMost(U, V)
			, left
			, right);
		}
	else {
		AddQuad(Deformed.GetRightMost(U, V)
			, Deformed.GetLeftMost(U, V)
			, left
			, right);
		}
	}
void DefMap::FillSinglePointLeftHand(int32 U, int32 V
			, DefPoint* left
			, boolean bCloserToBottom
			, boolean bOnTopEdge, boolean bOnBottomEdge){
	if (bCloserToBottom) {
		//closer to bottom
		AddQuad(Deformed.GetRightMost(U, V)
			, left
			, Deformed.GetLeftMost(U, V + 1)
			, Deformed.GetRightMost(U, V + 1));
		if (!bOnBottomEdge) {
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, left);
			}
		}
	else {
		AddQuad(Deformed.GetRightMost(U, V)
			, Deformed.GetLeftMost(U, V)
			, left
			, Deformed.GetRightMost(U, V + 1));
		if (!bOnTopEdge) {
			AddFacet(Deformed.GetLeftMost(U, V + 1)
				, left
				, Deformed.GetRightMost(U, V + 1));
			}
		}
	}
void DefMap::FillSinglePointRightHand(int32 U, int32 V
			, DefPoint* right
			, boolean bCloserToBottom
			, boolean bOnTopEdge, boolean bOnBottomEdge){
	if (bCloserToBottom) {
		//closer to bottom
		AddQuad(right
			, Deformed.GetLeftMost(U, V)
			, Deformed.GetLeftMost(U, V + 1)
			, Deformed.GetRightMost(U, V + 1));
		if (!bOnBottomEdge) {
			AddFacet(Deformed.GetRightMost(U, V)
				, Deformed.GetLeftMost(U, V)
				, right);
			}
		}
	else {
		AddQuad(Deformed.GetRightMost(U, V)
			, Deformed.GetLeftMost(U, V)
			, Deformed.GetLeftMost(U, V + 1)
			, right);
		if (!bOnTopEdge) {
			AddFacet(right
				, Deformed.GetLeftMost(U, V + 1)
				, Deformed.GetRightMost(U, V + 1));
			}
		}
	}
int DefMap::AddPosition(int pt) {
	if (AddOrder[0] == pt) {
		return 0;
		}
	if (AddOrder[1] == pt) {
		return 1;
		}
	if (AddOrder[2] == pt) {
		return 2;
		}
	return -1;
	}

void DefMap::GetBlockSize (const uint32& u, const uint32& v
						   , uint32& usize, uint32& vsize,const uint32& maxv){

	TVector3 deltau, deltav, tempdeltau, tempdeltav;

	uint32 tempu, tempv, tempusize, tempvsize;
	boolean bDone = false, bCanStepU = true, bCanStepV = true, bTrySingleDirection = false;
	
	usize = 1;
	vsize = 1;

	deltau = Deformed.Point(u + 1, v)->vertex.fVertex - Deformed.Point(u, v)->vertex.fVertex;
	deltav = Deformed.Point(u, v + 1)->vertex.fVertex - Deformed.Point(u, v)->vertex.fVertex;
	
	deltau.x = intcompare(deltau.x);
	deltau.y = intcompare(deltau.y);
	deltau.z = intcompare(deltau.z);
	deltav.x = intcompare(deltav.x);
	deltav.y = intcompare(deltav.y);
	deltav.z = intcompare(deltav.z);

	//is the current block itself parallel, if not we can't go on
	tempdeltau = Deformed.Point(u + 1, v + 1)->vertex.fVertex - Deformed.Point(u, v + 1)->vertex.fVertex;
	tempdeltav = Deformed.Point(u + 1, v + 1)->vertex.fVertex - Deformed.Point(u + 1, v)->vertex.fVertex;


	if (
		(intcompare(tempdeltav.x) == deltav.x)
		&&
		(intcompare(tempdeltav.y) == deltav.y)
		&&
		(intcompare(tempdeltav.z) == deltav.z)
		&&
		(intcompare(tempdeltau.x) == deltau.x)
		&&
		(intcompare(tempdeltau.y) == deltau.y)
		&&
		(intcompare(tempdeltau.z) == deltau.z)
		) {
		

		while (bDone == false) {
			if (bCanStepU) {
				//see if we can take another step in the U direction
				if (u + usize >= fData.lU) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						DefPoint* checkpoint = Deformed.Point(u + usize + 1, tempv);
						if (//point is null
							(checkpoint == NULL)
							||
							//point isn't filled
							(!checkpoint->bHasData)
							||
							//or already filled
							((Deformed.Point(u + usize, tempv)->bFilled)
								&&(tempv != v + vsize))) {
							bCanStepU = false;
							//get us out of this loop
							tempv = v + vsize + 1;
							}
						else {
							tempdeltau = Deformed.Point(u + usize + 1, tempv)->vertex.fVertex 
								- Deformed.Point(u + usize, tempv)->vertex.fVertex;
							tempdeltau.x = intcompare(tempdeltau.x);
							tempdeltau.y = intcompare(tempdeltau.y);
							tempdeltau.z = intcompare(tempdeltau.z);
							if ((tempdeltau.x != deltau.x)
								||
								(tempdeltau.y != deltau.y)
								||
								(tempdeltau.z != deltau.z)){
								bCanStepU = false;
								//get us out of this loop
								tempv = v + vsize + 1;
								}	
							}
						}
					}
				}

			if (bCanStepV) {
				//see if we can take another step in the U direction
				if ((v + vsize >= fData.lV)||(v + vsize >= maxv)) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						DefPoint* checkpoint = Deformed.Point(tempu, v + vsize + 1);
						if (//point is null
							(checkpoint == NULL)
							||
							//point isn't filled
							(!checkpoint->bHasData)
							//or already filled
							||((Deformed.Point(tempu, v + vsize)->bFilled)
								&&(tempu != u + usize))){
							bCanStepV = false;
							tempu = u + usize + 1;
							}
						else {
							tempdeltav = Deformed.Point(tempu, v + vsize + 1)->vertex.fVertex 
								- Deformed.Point(tempu, v + vsize)->vertex.fVertex;
							tempdeltav.x = intcompare(tempdeltav.x);
							tempdeltav.y = intcompare(tempdeltav.y);
							tempdeltav.z = intcompare(tempdeltav.z);
							if ((tempdeltav.x != deltav.x)
								||
								(tempdeltav.y != deltav.y)
								||
								(tempdeltav.z != deltav.z)) {
								bCanStepV = false;
								tempu = u + usize + 1;
								}
							}
						}
					}
				}

			if ((bCanStepV)&&(bCanStepU)) {
				//we can go both directions, but can we do the little corner
				//if not we'll just go in the U direction
				if ((!Deformed.Point(u + usize + 1, v + vsize + 1)->bHasData)
					||
					(Deformed.Point(u + usize, v + vsize)->bFilled)){
					
					bTrySingleDirection = true;
					bDone = true;
					}
				else {
					tempdeltau = Deformed.Point(u + usize + 1, v + vsize + 1)->vertex.fVertex 
						- Deformed.Point(u + usize, v + vsize + 1)->vertex.fVertex;
					tempdeltau.x = intcompare(tempdeltau.x);
					tempdeltau.y = intcompare(tempdeltau.y);
					tempdeltau.z = intcompare(tempdeltau.z);
					tempdeltav = Deformed.Point(u + usize + 1, v + vsize + 1)->vertex.fVertex 
						- Deformed.Point(u + usize + 1, v + vsize)->vertex.fVertex;
					tempdeltav.x = intcompare(tempdeltav.x);
					tempdeltav.y = intcompare(tempdeltav.y);
					tempdeltav.z = intcompare(tempdeltav.z);
					if ((tempdeltav.x == deltav.x)
						&&
						(tempdeltav.y == deltav.y)
						&&
						(tempdeltav.z == deltav.z)
						&&
						(tempdeltau.x == deltau.x)
						&&
						(tempdeltau.y == deltau.y)
						&&
						(tempdeltau.z == deltau.z)) {
						vsize++;
						usize++;
						}
					else {
						bTrySingleDirection = true;
						bDone = true;
						}
					}
				}
			else if (bCanStepV) {
				vsize++;
				}
			else if (bCanStepU) {
				usize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		}
	
	if (bTrySingleDirection) {
		//first keep going in the U direction
		tempusize = usize + 1;
		bDone = false;
		while (bDone == false) {
			if (bCanStepU) {
				//see if we can take another step in the U direction
				if (u + tempusize >= fData.lU) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						DefPoint* checkpoint = Deformed.Point(u + tempusize + 1, tempv);
						if (//point is null
							(checkpoint == NULL)
							||
							//point isn't filled
							(!checkpoint->bHasData)
							||
							//or already filled
							((Deformed.Point(u + tempusize, tempv)->bFilled)
								&&(tempv != v + vsize))) {
							bCanStepU = false;
							//get us out of this loop
							tempv = v + vsize + 1;
							}
						else {
							tempdeltau = Deformed.Point(u + tempusize + 1, tempv)->vertex.fVertex 
								- Deformed.Point(u + tempusize, tempv)->vertex.fVertex;
							tempdeltau.x = intcompare(tempdeltau.x);
							tempdeltau.y = intcompare(tempdeltau.y);
							tempdeltau.z = intcompare(tempdeltau.z);
							if ((tempdeltau.x != deltau.x)
								||
								(tempdeltau.y != deltau.y)
								||
								(tempdeltau.z != deltau.z)){
								bCanStepU = false;
								//get us out of this loop
								tempv = v + vsize + 1;
								}	
							}
						}
					}
				}

			if (bCanStepU) {
				tempusize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		tempvsize = vsize + 1;
		bDone = false;
		//next keep going in the V direction
		while (bDone == false) {
			if (bCanStepV) {
				//see if we can take another step in the U direction
				if (v + tempvsize >= fData.lV) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						DefPoint* checkpoint = Deformed.Point(tempu, v + tempvsize + 1);
						if (//point is null
							(checkpoint == NULL)
							||
							//point isn't filled
							(!checkpoint->bHasData)
							//or already filled
							||((Deformed.Point(tempu, v + tempvsize)->bFilled)
								&&(tempu != u + usize))){
							bCanStepV = false;
							tempu = u + usize + 1;
							}
						else {
							tempdeltav = Deformed.Point(tempu, v + tempvsize + 1)->vertex.fVertex 
								- Deformed.Point(tempu, v + tempvsize)->vertex.fVertex;
							tempdeltav.x = intcompare(tempdeltav.x);
							tempdeltav.y = intcompare(tempdeltav.y);
							tempdeltav.z = intcompare(tempdeltav.z);
							if ((tempdeltav.x != deltav.x)
								||
								(tempdeltav.y != deltav.y)
								||
								(tempdeltav.z != deltav.z)) {
								bCanStepV = false;
								tempu = u + usize + 1;
								}
							}
						}
					}
				}

			if (bCanStepV) {
				tempvsize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		//which one gave us the biggest block
		if (usize * tempvsize > tempusize * vsize) {
			vsize = tempvsize;
			}
		else {
			usize = tempusize;
			}
		}//end try single

	//mark everything in this block as filled
	for (tempu = u; tempu < u + usize; tempu++) {
		for (tempv = v; tempv < v + vsize; tempv++) {
			Deformed.Point(tempu, tempv)->bFilled = true;
			}
		}
	}
