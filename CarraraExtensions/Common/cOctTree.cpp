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
#include "cOctTree.h"
 
cOctTree::cOctTree() {
	mode = OCT_X + OCT_Y + OCT_Z;
	child[0] = NULL;
	child[1] = NULL;
	child[2] = NULL;
	child[3] = NULL;
	child[4] = NULL;
	child[5] = NULL;
	child[6] = NULL;
	child[7] = NULL;
	}

cOctTree::~cOctTree() {
	flatNodeList.ArrayFree();
	if (child[0] != NULL) {
		delete child[0];
		}
	if (child[1] != NULL) {
		delete child[1];
		}
	if (child[2] != NULL) {
		delete child[2];
		}
	if (child[3] != NULL) {
		delete child[3];
		}
	if (child[4] != NULL) {
		delete child[4];
		}
	if (child[5] != NULL) {
		delete child[5];
		}
	if (child[6] != NULL) {
		delete child[6];
		}
	if (child[7] != NULL) {
		delete child[7];
		}
	}

MeshTreeItem* cOctTree::AddTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3, const long plFacet)
{
	//check to see if the facet will fit into one of the children
	//if so pass it to the child
	//if not add it to my array
	long where = 0;
	if ((mode & OCT_X) == OCT_X) {
		if ((pt1.x < center.x)&&(pt2.x < center.x)&&(pt3.x < center.x)) {
			//x low
			}
		else if ((pt1.x >= center.x)&&(pt2.x >= center.x)&&(pt3.x >= center.x)){
			//x high 
			where += OCT_X;
			}
		else {
			//mixed add it to ours
			LoadItem(pt1, pt2, pt3, plFacet, Facets[CurrentFacet]);
			CurrentFacet++;
			return &Facets[CurrentFacet -1];
			}
		}
	if ((mode & OCT_Y) == OCT_Y) {
		if ((pt1.y < center.y)&&(pt2.y < center.y)&&(pt3.y < center.y)) {
			//y low
			}
		else if ((pt1.y >= center.y)&&(pt2.y >= center.y)&&(pt3.y >= center.y)){
			//y high 
			where += OCT_Y;
			}
		else {
			//mixed add it to ours
			LoadItem(pt1, pt2, pt3, plFacet, Facets[CurrentFacet]);
			CurrentFacet++;
			return &Facets[CurrentFacet -1];
			}
		}
	if ((mode & OCT_Z) == OCT_Z) {
		if ((pt1.z < center.z)&&(pt2.z < center.z)&&(pt3.z < center.z)) {
			//z low
			}
		else if ((pt1.z >= center.z)&&(pt2.z >= center.z)&&(pt3.z >= center.z)){
			//z high 
			where += OCT_Z;
			}
		else {
			//mixed add it to ours
			LoadItem(pt1, pt2, pt3, plFacet, Facets[CurrentFacet]);
			CurrentFacet++;
			return &Facets[CurrentFacet -1];
			}
		}
	return child[where]->AddTriangle(pt1, pt2, pt3, plFacet);
}

void cOctTree::CountTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3){
	//check to see if the facet will fit into one of the children
	//if so pass it to the child
	//if not add it to my array
	long where = 0;
	if ((mode & OCT_X) == OCT_X) {
		if ((pt1.x < center.x)&&(pt2.x < center.x)&&(pt3.x < center.x)) {
			//x low
			}
		else if ((pt1.x >= center.x)&&(pt2.x >= center.x)&&(pt3.x >= center.x)){
			//x high 
			where += OCT_X;
			}
		else {
			//mixed add it to ours
			FacetCount++;
			return;
			}
		}
	if ((mode & OCT_Y) == OCT_Y) {
		if ((pt1.y < center.y)&&(pt2.y < center.y)&&(pt3.y < center.y)) {
			//y low
			}
		else if ((pt1.y >= center.y)&&(pt2.y >= center.y)&&(pt3.y >= center.y)){
			//y high 
			where += OCT_Y;
			}
		else {
			//mixed add it to ours
			FacetCount++;
			return;
			}
		}
	if ((mode & OCT_Z) == OCT_Z) {
		if ((pt1.z < center.z)&&(pt2.z < center.z)&&(pt3.z < center.z)) {
			//z low
			}
		else if ((pt1.z >= center.z)&&(pt2.z >= center.z)&&(pt3.z >= center.z)){
			//z high 
			where += OCT_Z;
			}
		else {
			//mixed add it to ours
			FacetCount++;
			return;
			}
		}

	switch (where) {
		case OCT_X + OCT_Y + OCT_Z://xyz
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMax = bbox.fMax;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_X + OCT_Y://xy
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.z = center.z;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_X + OCT_Z://xz
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.y = center.y;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_X://x
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.y = center.y;
				tempbox.fMax.z = center.z;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_Y + OCT_Z://yz
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_Y://y
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
				tempbox.fMax.z = center.z;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case OCT_Z://z
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
				tempbox.fMax.y = center.y;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		case 0://
			if (child[where] == NULL) {
				TBBox3D	tempbox;
				cOctTree* octtree;
				octtree = new cOctTree;
				octtree->mode = mode;
				child[where] = octtree;
				tempbox.fMax = center;
				tempbox.fMin = bbox.fMin;
				child[where]->SetBoundingBox(tempbox);
				child[where]->parent = this;
				}
			child[where]->CountTriangle(pt1, pt2, pt3);
			break;
		}

	}

void cOctTree::CountNodes(uint32& nodes) {
	if (child[0] != NULL) {
		child[0]->CountNodes(nodes);
		}
	if (child[1] != NULL) {
		child[1]->CountNodes(nodes);
		}
	if (child[2] != NULL) {
		child[2]->CountNodes(nodes);
		}
	if (child[3] != NULL) {
		child[3]->CountNodes(nodes);
		}
	if (child[4] != NULL) {
		child[4]->CountNodes(nodes);
		}
	if (child[5] != NULL) {
		child[5]->CountNodes(nodes);
		}
	if (child[6] != NULL) {
		child[6]->CountNodes(nodes);
		}
	if (child[7] != NULL) {
		child[7]->CountNodes(nodes);
		}
	if (FacetCount > 0) {
		nodes = nodes + 1;
		}
	}

void cOctTree::FillFlatTree(uint32& nodeIndex, TMCArray<IMeshTree*>& flatNodeList){
	if (child[0] != NULL) {
		child[0]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[1] != NULL) {
		child[1]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[2] != NULL) {
		child[2]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[3] != NULL) {
		child[3]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[4] != NULL) {
		child[4]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[5] != NULL) {
		child[5]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[6] != NULL) {
		child[6]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[7] != NULL) {
		child[7]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (FacetCount > 0) {
		flatNodeList[nodeIndex] = (IMeshTree*)this;
		nodeIndex++;
		}
	}

long cOctTree::RankMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges)
{
	long where = 0;
	TVector3 pt1, pt2, pt3;
	long i;
	long lNumFacets = mesh->fFacets.GetElemCount();
	long lTopLevelFacets = 0;
	for (i = 0; i<lNumFacets;i++) {
		pt1 = mesh->fVertices[mesh->fFacets[i].pt1];
		pt2 = mesh->fVertices[mesh->fFacets[i].pt2];
		pt3 = mesh->fVertices[mesh->fFacets[i].pt3];
		if ((mode & OCT_X) == OCT_X) {
			if ((pt1.x < center.x)&&(pt2.x < center.x)&&(pt3.x < center.x)) {
				//x low
				}
			else if ((pt1.x >= center.x)&&(pt2.x >= center.x)&&(pt3.x >= center.x)){
				//x high 
				}
			else {
				//mixed add it to ours
				lTopLevelFacets++;
				goto OneFound;
				}
			}
		if ((mode & OCT_Y) == OCT_Y) {
			if ((pt1.y < center.y)&&(pt2.y < center.y)&&(pt3.y < center.y)) {
				//y low
				}
			else if ((pt1.y >= center.y)&&(pt2.y >= center.y)&&(pt3.y >= center.y)){
				//y high 
				}
			else {
				//mixed add it to ours
				lTopLevelFacets++;
				goto OneFound;
				}
			}
		if ((mode & OCT_Z) == OCT_Z) {
			if ((pt1.z < center.z)&&(pt2.z < center.z)&&(pt3.z < center.z)) {
				//z low
				}
			else if ((pt1.z >= center.z)&&(pt2.z >= center.z)&&(pt3.z >= center.z)){
				//z high 
				}
			else {
				//mixed add it to ours
				lTopLevelFacets++;
				goto OneFound;
				}
			}
OneFound:;
		}
	return lTopLevelFacets;
	//return 0;
}

void cOctTree::AllocFacets()
{
	if (FacetCount>0) 
	{
		Facets = (MeshTreeItem*)MCcalloc(FacetCount, sizeof(MeshTreeItem));
	}
	if (child[0] != NULL) 
	{
		child[0]->AllocFacets();
	}
	if (child[1] != NULL) 
	{
		child[1]->AllocFacets();
	}
	if (child[2] != NULL) 
	{
		child[2]->AllocFacets();
	}
	if (child[3] != NULL) 
	{
		child[3]->AllocFacets();
	}
	if (child[4] != NULL) 
	{
		child[4]->AllocFacets();
	}
	if (child[5] != NULL) 
	{
		child[5]->AllocFacets();
	}
	if (child[6] != NULL) 
	{
		child[6]->AllocFacets();
	}
	if (child[7] != NULL) 
	{
		child[7]->AllocFacets();
	}
}
