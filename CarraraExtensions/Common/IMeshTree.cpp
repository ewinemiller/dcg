/*  Carrara plug-in utilities
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
#include "IMeshTree.h"
#include <math.h>
#include <stdlib.h>

IMeshTree::IMeshTree() {
		parent = NULL;
		FacetCount = 0;
		CurrentFacet = 0;
		Facets = NULL;
		center.x = 0;
		center.y = 0;
		center.z = 0;
		flatNodeCount = 0;
	}

IMeshTree::~IMeshTree() 
{
	if (Facets != NULL) 
	{
		MCfree(Facets);
	}
}

void IMeshTree::FindClosestNode(IMeshTree** node, const TVector3& point, meshTreeCallLocalStorage& callLocalStorage)
{
	callLocalStorage.closestDistance = FPOSINF;
	if (callLocalStorage.distance.GetElemCount() != flatNodeCount)
	{
		callLocalStorage.distance.SetElemCount(flatNodeCount);
	}
	real32 fTemp;
	for(uint32 i = 0; i < flatNodeCount; i++) 
	{
		callLocalStorage.distance[i] = sqrt(sqr(point, flatNodeList[i]->center));
		fTemp = callLocalStorage.distance[i] + flatNodeList[i]->radius;
		if (fTemp < callLocalStorage.closestDistance) 
		{
			callLocalStorage.closestDistance = fTemp;
			*node = (IMeshTree*)flatNodeList[i];
		}
	}
}

void IMeshTree::CalcBoundingBoxes(FacetMesh* mesh) {
	//calculate the real bounding box and center 
	for(uint32 i = 0; i<flatNodeCount;i++) 
	{
		TBBox3D bbox;
		IMeshTree* node = flatNodeList[i];
		for (uint32 lFacet = 0; lFacet < node->FacetCount; lFacet++) 
		{
			bbox.AddPoint(mesh->fVertices[mesh->fFacets[node->Facets[lFacet].FacetNumber].pt1]);
			bbox.AddPoint(mesh->fVertices[mesh->fFacets[node->Facets[lFacet].FacetNumber].pt2]);
			bbox.AddPoint(mesh->fVertices[mesh->fFacets[node->Facets[lFacet].FacetNumber].pt3]);
			
		}
		bbox.GetCenter(node->center);

		node->radius = sqrt(sqr(node->center, bbox.fMax));
	}

}

MCCOMErr IMeshTree::enumSiblings(IMeshTree* node, IMeshTree** sibling, meshTreeCallLocalStorage& callLocalStorage){
	while (callLocalStorage.currentIndex < flatNodeCount) {
		if ((callLocalStorage.distance[callLocalStorage.currentIndex]
			- flatNodeList[callLocalStorage.currentIndex]->radius
			<= callLocalStorage.closestDistance)
			&&(flatNodeList[callLocalStorage.currentIndex] != node)){
			
			*sibling = flatNodeList[callLocalStorage.currentIndex];
			callLocalStorage.currentIndex+=1;
			return MC_S_OK;
			}
		callLocalStorage.currentIndex+=1;
		}
	callLocalStorage.currentIndex = 0;
	return MC_S_FALSE;
	}


void IMeshTree::SetBoundingBox(TBBox3D NewBox) {
	bbox = NewBox;
	bbox.GetCenter(center);
	}

void IMeshTree::LoadItem(TVector3 pt1, TVector3 pt2, TVector3 pt3, long plFacet, MeshTreeItem& Item)
{
	Item.FacetNumber = plFacet;

	//create flat triangle
	FillFlat(pt1,pt2,pt3, Item.flatpt2y[0], Item.Rotation3D[0]);
	Item.flatpoint.x = pt3.x;
	Item.flatpoint.y = pt3.y;
	FillFlat(pt2,pt3,pt1, Item.flatpt2y[1], Item.Rotation2D[0]);
	FillFlat(pt3,pt1,pt2, Item.flatpt2y[2], Item.Rotation2D[1]);

}

void IMeshTree::FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3, real32& flatpt2y, DCGTransform3D& Transform){
	
	
	TTransform3D TempRotation;
	TTransform3D Rotation;

	Rotation.fRotationAndScale.Reset();
	TVector3 temppt1, temppt2;

	DCGTriangle Flat;
	Flat.pt1 = pt1; Flat.pt2 = pt2; Flat.pt3 = pt3;
	real32 theta, sintheta, costheta;
	real32 d12, dSin, x, y, d13;
	
	Transform.fTranslation[0] = pt1.x;
	Transform.fTranslation[1]= pt1.y;
	Transform.fTranslation[2] = pt1.z;
	Flat.pt2.x -= pt1.x;	Flat.pt2.y -= pt1.y;	Flat.pt2.z -= pt1.z;
	Flat.pt3.x -= pt1.x;	Flat.pt3.y -= pt1.y;	Flat.pt3.z -= pt1.z;
	Flat.pt1.x = Flat.pt1.y = Flat.pt1.z = 0;
	//Rotate around the Z axis
	temppt1 = Flat.pt1; temppt2 = Flat.pt2;
	temppt1.z = temppt2.z = 0;
	d12 = distance(temppt1,temppt2);
	if (d12 == 0)
	{
		theta = 0;
	}
	else
	{
		dSin = temppt2.x/d12;
		if (dSin > 1) {
			dSin = 1;
			}
		else if(dSin < -1) {
			dSin = -1;
			}
		theta = asin(dSin);
	}
	if (Flat.pt2.y < 0) {
		theta = PI - theta;
		}
	sintheta = sin(theta);
	costheta = cos(theta);
	x = Flat.pt2.x * costheta - Flat.pt2.y * sintheta;
	Flat.pt2.y = Flat.pt2.x * sintheta + Flat.pt2.y * costheta;
	Flat.pt2.x = x;
	x = Flat.pt3.x * costheta - Flat.pt3.y * sintheta;
	Flat.pt3.y = Flat.pt3.x * sintheta + Flat.pt3.y * costheta;
	Flat.pt3.x = x;
	
	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = - sintheta;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = sintheta;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = 1;


	Rotation.fRotationAndScale = TempRotation.fRotationAndScale * Rotation.fRotationAndScale;
	//Rotate around the X axis
	d12 = distance(Flat.pt1,Flat.pt2);
	if (d12 == 0)
	{
		theta = 0;
	}
	else
	{
		dSin = Flat.pt2.z/d12;
		if (dSin > 1) {
			dSin = 1;
			}
		else if(dSin < -1) {
			dSin = -1;
			}

		theta = -asin(dSin);
	}
	if (Flat.pt2.y < 0) {
		theta = PI - theta;
		}
	sintheta = sin(theta);
	costheta = cos(theta);
	y = Flat.pt2.y * costheta - Flat.pt2.z * sintheta;
	Flat.pt2.z = Flat.pt2.y * sintheta + Flat.pt2.z * costheta;
	Flat.pt2.y = y;
	y = Flat.pt3.y * costheta - Flat.pt3.z * sintheta;
	Flat.pt3.z = Flat.pt3.y * sintheta + Flat.pt3.z * costheta;
	Flat.pt3.y = y;

	TempRotation.fRotationAndScale[0][0] = 1;
	TempRotation.fRotationAndScale[0][1] = 0;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = 0;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = - sintheta;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = sintheta;
	TempRotation.fRotationAndScale[2][2] = costheta;


	Rotation.fRotationAndScale = TempRotation.fRotationAndScale * Rotation.fRotationAndScale;

	//Rotate around the Y axis
	temppt1 = Flat.pt3; temppt2 = Flat.pt3;
	temppt1.x = temppt1.z = temppt2.z = 0;
	d13 = distance(temppt1, Flat.pt3);
	if (d13 == 0)
	{
		theta = 0;
	}
	else
	{
		dSin = Flat.pt3.z / d13;
		if (dSin > 1) {
			dSin = 1;
			}
		else if(dSin < -1) {
			dSin = -1;
			}
		theta = asin(dSin);
	}
	if (Flat.pt3.x < 0) {
		theta = PI - theta;
		}

	sintheta = sin(theta);
	costheta = cos(theta);
	x = Flat.pt2.x * costheta + Flat.pt2.z * sintheta;
	Flat.pt2.z = - Flat.pt2.x * sintheta + Flat.pt2.z * costheta;
	Flat.pt2.x = x;
	x = Flat.pt3.x * costheta + Flat.pt3.z * sintheta;
	Flat.pt3.z = - Flat.pt3.x * sintheta + Flat.pt3.z * costheta;
	Flat.pt3.x = x;

	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = 0;
	TempRotation.fRotationAndScale[0][2] = sintheta;
	TempRotation.fRotationAndScale[1][0] = 0;
	TempRotation.fRotationAndScale[1][1] = 1;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = - sintheta;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = costheta;

	Rotation.fRotationAndScale = TempRotation.fRotationAndScale * Rotation.fRotationAndScale;
	flatpt2y = Flat.pt2.y;
	
	Transform.x[0] = Rotation.fRotationAndScale[0][0];
	Transform.y[0] = Rotation.fRotationAndScale[0][1];
	Transform.z[0] = Rotation.fRotationAndScale[0][2];
	Transform.x[1] = Rotation.fRotationAndScale[1][0];
	Transform.y[1] = Rotation.fRotationAndScale[1][1];
	Transform.z[1] = Rotation.fRotationAndScale[1][2];
	Transform.x[2] = Rotation.fRotationAndScale[2][0];
	Transform.y[2] = Rotation.fRotationAndScale[2][1];
	Transform.z[2] = Rotation.fRotationAndScale[2][2];
	
	pt1 = Flat.pt1;
	pt2 = Flat.pt2;
	pt3 = Flat.pt3;


	}

void IMeshTree::FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3, real32& flatpt2y, DCGTransform2D& Transform)
{
	
	
	TTransform3D TempRotation;
	TTransform3D Rotation;

	Rotation.fRotationAndScale.Reset();
	TVector3 temppt1, temppt2;

	DCGTriangle Flat;
	Flat.pt1 = pt1; Flat.pt2 = pt2; Flat.pt3 = pt3;
	real32 theta, sintheta, costheta;
	real32 d12, dSin, x;
	
	Transform.fTranslation[0] = pt1.x;
	Transform.fTranslation[1]= pt1.y;

	Flat.pt2.x -= pt1.x;	Flat.pt2.y -= pt1.y;	Flat.pt2.z -= pt1.z;
	Flat.pt3.x -= pt1.x;	Flat.pt3.y -= pt1.y;	Flat.pt3.z -= pt1.z;
	Flat.pt1.x = Flat.pt1.y = Flat.pt1.z = 0;
	//Rotate around the Z axis
	temppt1 = Flat.pt1; temppt2 = Flat.pt2;
	temppt1.z = temppt2.z = 0;
	d12 = distance(temppt1,temppt2);
	if (d12 == 0)
	{
		theta = 0;
	}
	else
	{
		dSin = temppt2.x/d12;
		if (dSin > 1) {
			dSin = 1;
			}
		else if(dSin < -1) {
			dSin = -1;
			}
		theta = asin(dSin);
	}
	if (Flat.pt2.y < 0) {
		theta = PI - theta;
		}
	sintheta = sin(theta);
	costheta = cos(theta);
	x = Flat.pt2.x * costheta - Flat.pt2.y * sintheta;
	Flat.pt2.y = Flat.pt2.x * sintheta + Flat.pt2.y * costheta;
	Flat.pt2.x = x;
	x = Flat.pt3.x * costheta - Flat.pt3.y * sintheta;
	Flat.pt3.y = Flat.pt3.x * sintheta + Flat.pt3.y * costheta;
	Flat.pt3.x = x;
	
	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = - sintheta;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = sintheta;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = 1;


	Rotation.fRotationAndScale = TempRotation.fRotationAndScale * Rotation.fRotationAndScale;

	flatpt2y = Flat.pt2.y;
	
	Transform.x[0] = Rotation.fRotationAndScale[0][0];
	Transform.y[0] = Rotation.fRotationAndScale[0][1];
	Transform.x[1] = Rotation.fRotationAndScale[1][0];
	Transform.y[1] = Rotation.fRotationAndScale[1][1];
	
	pt1 = Flat.pt1;
	pt2 = Flat.pt2;
	pt3 = Flat.pt3;


}

void IMeshTree::CacheMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges)
{

	TVector3 pt1, pt2, pt3;
	uint32 facetCount = mesh->fFacets.GetElemCount();
	TMCArray<boolean> addFacet;


	addFacet.SetElemCount(facetCount);

	for (uint32 facetIndex = 0; facetIndex < facetCount; facetIndex++) 
	{
		addFacet[facetIndex] = false;
		
		for (uint32 edgeIndex = 0; edgeIndex< 3;edgeIndex++)
		{
			if (drawedge[facetEdges[facetIndex][edgeIndex]]) 
			{
				addFacet[facetIndex] = true;
				edgeIndex = 3;
			}

		}
	}

	for (uint32 facetIndex = 0; facetIndex<facetCount;facetIndex++) 
	{
		if (addFacet[facetIndex])
		{
			pt1 = mesh->fVertices[mesh->fFacets[facetIndex].pt1];
			pt2 = mesh->fVertices[mesh->fFacets[facetIndex].pt2];
			pt3 = mesh->fVertices[mesh->fFacets[facetIndex].pt3];
			CountTriangle(pt1, pt2, pt3);
		}
	}
	AllocFacets();
	for (uint32 facetIndex = 0; facetIndex<facetCount;facetIndex++) 
	{
		if (addFacet[facetIndex])
		{
			pt1 = mesh->fVertices[mesh->fFacets[facetIndex].pt1];
			pt2 = mesh->fVertices[mesh->fFacets[facetIndex].pt2];
			pt3 = mesh->fVertices[mesh->fFacets[facetIndex].pt3];
			MeshTreeItem* item = AddTriangle(pt1, pt2, pt3, facetIndex);
			item->lPoints[0] = mesh->fFacets[facetIndex].pt1;
			item->lPoints[1] = mesh->fFacets[facetIndex].pt2;
			item->lPoints[2] = mesh->fFacets[facetIndex].pt3;

			//now save which edges are in our facet
			for (uint32 edge = 0; edge< 3;edge++)
			{
				item->lEdges[edge] = facetEdges[facetIndex][edge];
			}
		}
	}
	addFacet.ArrayFree();

	CountNodes(flatNodeCount);
	flatNodeList.SetElemCount(flatNodeCount);

	uint32 nodeIndex = 0; 
	FillFlatTree(nodeIndex, flatNodeList);
	CalcBoundingBoxes(mesh);

}