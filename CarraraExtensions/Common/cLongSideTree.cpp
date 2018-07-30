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
#include "cLongSideTree.h"

cLongSideTree::cLongSideTree() {
	mode = OCT_X + OCT_Y + OCT_Z;
	modeset = false;
	child[0] = NULL;
	child[1] = NULL;
	child[2] = NULL;
	child[3] = NULL;
	child[4] = NULL;
	child[5] = NULL;
	child[6] = NULL;
	child[7] = NULL;
	}

cLongSideTree::~cLongSideTree() {
	if (child[0] != NULL) {
		delete child[0];
		}
	if (child[OCT_Z] != NULL) {
		delete child[OCT_Z];
		}
	if (child[OCT_Y] != NULL) {
		delete child[OCT_Y];
		}
	if (child[OCT_X] != NULL) {
		delete child[OCT_X];
		}
	}

MeshTreeItem* cLongSideTree::AddTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3, const long plFacet)
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

void cLongSideTree::CountTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3)
{
	//check to see if the facet will fit into one of the children
	//if so pass it to the child
	//if not add it to my array
	if (modeset == false) {
		real32 x = bbox.fMax.x - bbox.fMin.x
			, y = bbox.fMax.y - bbox.fMin.y
			, z = bbox.fMax.z - bbox.fMin.z;
		if ((x >= y)&&(x >= z)) {
			mode = OCT_X;
			}
		if ((y >= x)&&(y >= z)) {
			mode = OCT_Y;
			}
		if ((z >= x)&&(z >= y)) {
			mode = OCT_Z;
			}
		modeset = true;
		}
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

	if (child[where] == NULL) {
		TBBox3D	tempbox;
		cLongSideTree* octtree;
		octtree = new cLongSideTree;
		child[where] = octtree;
		
	
		switch (where) {
			case OCT_X://x
				tempbox = bbox;
				tempbox.fMin.x = center.x;
				break;
			case OCT_Y://y
				tempbox = bbox;
				tempbox.fMin.y = center.y;
				break;
			case OCT_Z://z
				tempbox = bbox;
				tempbox.fMin.z = center.z;
				break;
			case 0://
				switch (mode) {
					case OCT_X:
						tempbox = bbox;
						tempbox.fMax.x = center.x;
						break;
					case OCT_Y:
						tempbox = bbox;
						tempbox.fMax.y = center.y;
						break;
					case OCT_Z:
						tempbox = bbox;
						tempbox.fMax.z = center.z;
						break;
					}
				break;
			}
		child[where]->SetBoundingBox(tempbox);
		child[where]->parent = this;
		}//end child null

	child[where]->CountTriangle(pt1, pt2, pt3);

}


long cLongSideTree::RankMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges)
{
	TVector3 pt1, pt2, pt3;
	long i;
	long lNumFacets = mesh->fFacets.GetElemCount();
	long lTopLevelFacets = 0;
	for (i = 0; i<lNumFacets;i++) {
		pt1 = mesh->fVertices[mesh->fFacets[i].pt1];
		pt2 = mesh->fVertices[mesh->fFacets[i].pt2];
		pt3 = mesh->fVertices[mesh->fFacets[i].pt3];
		if (modeset == false) {
			real32 x = bbox.fMax.x - bbox.fMin.x
				, y = bbox.fMax.y - bbox.fMin.y
				, z = bbox.fMax.z - bbox.fMin.z;
			if ((x >= y)&&(x >= z)) {
				mode = OCT_X;
				}
			if ((y >= x)&&(y >= z)) {
				mode = OCT_Y;
				}
			if ((z >= x)&&(z >= y)) {
				mode = OCT_Z;
				}
			modeset = true;
			}
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
}

void cLongSideTree::AllocFacets()
{
	if (FacetCount>0) {
		Facets = (MeshTreeItem*)MCcalloc(FacetCount, sizeof(MeshTreeItem));
		}
	if (child[0] != NULL) {
		child[0]->AllocFacets();
		}
	if (child[OCT_Z] != NULL) {
		child[OCT_Z]->AllocFacets();
		}
	if (child[OCT_Y] != NULL) {
		child[OCT_Y]->AllocFacets();
		}
	if (child[OCT_X] != NULL) {
		child[OCT_X]->AllocFacets();
		}
}

void cLongSideTree::CountNodes(uint32& nodes) 
{
	if (child[0] != NULL) {
		child[0]->CountNodes(nodes);
		}
	if (child[OCT_Z] != NULL) {
		child[OCT_Z]->CountNodes(nodes);
		}
	if (child[OCT_Y] != NULL) {
		child[OCT_Y]->CountNodes(nodes);
		}
	if (child[OCT_X] != NULL) {
		child[OCT_X]->CountNodes(nodes);
		}
	if (FacetCount > 0) {
		nodes = nodes + 1;
		}
}

void cLongSideTree::FillFlatTree(uint32& nodeIndex, TMCArray<IMeshTree*>& flatNodeList)
{
	if (child[0] != NULL) {
		child[0]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[OCT_Z] != NULL) {
		child[OCT_Z]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[OCT_Y] != NULL) {
		child[OCT_Y]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (child[OCT_X] != NULL) {
		child[OCT_X]->FillFlatTree(nodeIndex, flatNodeList);
		}
	if (FacetCount > 0) {
		flatNodeList[nodeIndex] = (IMeshTree*)this;
		nodeIndex++;
		}
}

