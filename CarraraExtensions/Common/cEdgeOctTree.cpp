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
#include "cEdgeOctTree.h"

cEdgeOctTree::cEdgeOctTree() 
{
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

cEdgeOctTree::~cEdgeOctTree() 
{
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

void cEdgeOctTree::AddEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex)
{
	//check to see if the facet will fit into one of the children
	//if so pass it to the child
	//if not add it to my array
	long where = 0;
	#if (VERSIONNUMBER < 0x040000)
	const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.x];
	const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.y];
	#else
	const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].x];
	const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].y];
	#endif
	if ((mode & OCT_X) == OCT_X) {
		if ((pt1.x < center.x)&&(pt2.x < center.x)) {
			//x low
			}
		else if ((pt1.x >= center.x)&&(pt2.x >= center.x)){
			//x high 
			where += OCT_X;
			}
		else {
			//mixed add it to ours
			Edges[CurrentEdge] = EdgeIndex;
			CurrentEdge++;
			return;
			}
		}
	if ((mode & OCT_Y) == OCT_Y) {
		if ((pt1.y < center.y)&&(pt2.y < center.y)) {
			//y low
			}
		else if ((pt1.y >= center.y)&&(pt2.y >= center.y)){
			//y high 
			where += OCT_Y;
			}
		else {
			//mixed add it to ours
			Edges[CurrentEdge] = EdgeIndex;
			CurrentEdge++;
			return;
			}
		}
	if ((mode & OCT_Z) == OCT_Z) {
		if ((pt1.z < center.z)&&(pt2.z < center.z)) {
			//z low
			}
		else if ((pt1.z >= center.z)&&(pt2.z >= center.z)){
			//z high 
			where += OCT_Z;
			}
		else {
			//mixed add it to ours
			Edges[CurrentEdge] = EdgeIndex;
			CurrentEdge++;
			return;
			}
		}
	child[where]->AddEdge(mesh, EdgeIndex);
}

void cEdgeOctTree::CountEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex)
{
	#if (VERSIONNUMBER < 0x040000)
	const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.x];
	const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.y];
	#else
	const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].x];
	const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].y];
	#endif
	//check to see if the facet will fit into one of the children
	//if so pass it to the child
	//if not add it to my array
	long where = 0;
	if ((mode & OCT_X) == OCT_X) {
		if ((pt1.x < center.x)&&(pt2.x < center.x)) {
			//x low
			}
		else if ((pt1.x >= center.x)&&(pt2.x >= center.x)){
			//x high 
			where += OCT_X;
			}
		else {
			//mixed add it to ours
			EdgeCount++;
			return;
			}
		}
	if ((mode & OCT_Y) == OCT_Y) {
		if ((pt1.y < center.y)&&(pt2.y < center.y)) {
			//y low
			}
		else if ((pt1.y >= center.y)&&(pt2.y >= center.y)){
			//y high 
			where += OCT_Y;
			}
		else {
			//mixed add it to ours
			EdgeCount++;
			return;
			}
		}
	if ((mode & OCT_Z) == OCT_Z) {
		if ((pt1.z < center.z)&&(pt2.z < center.z)) {
			//z low
			}
		else if ((pt1.z >= center.z)&&(pt2.z >= center.z)){
			//z high 
			where += OCT_Z;
			}
		else {
			//mixed add it to ours
			EdgeCount++;
			return;
			}
		}

	if (child[where] == NULL) {
		TBBox3D	tempbox;
		cEdgeOctTree* octtree;
		octtree = new cEdgeOctTree;
		octtree->mode = mode;
		child[where] = octtree;
	switch (where) {
		case OCT_X + OCT_Y + OCT_Z://xyz
				tempbox.fMin = center;
				tempbox.fMax = bbox.fMax;
			break;
		case OCT_X + OCT_Y://xy
				tempbox.fMin = center;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.z = center.z;
			break;
		case OCT_X + OCT_Z://xz
				tempbox.fMin = center;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.y = center.y;
			break;
		case OCT_X://x
				tempbox.fMin = center;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.y = center.y;
				tempbox.fMax.z = center.z;
			break;
		case OCT_Y + OCT_Z://yz
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
			break;
		case OCT_Y://y
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMin.z = bbox.fMin.z;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
				tempbox.fMax.z = center.z;
			break;
		case OCT_Z://z
				tempbox.fMin = center;
				tempbox.fMin.x = bbox.fMin.x;
				tempbox.fMin.y = bbox.fMin.y;
				tempbox.fMax = bbox.fMax;
				tempbox.fMax.x = center.x;
				tempbox.fMax.y = center.y;
			break;
		case 0://
				tempbox.fMax = center;
				tempbox.fMin = bbox.fMin;
			break;
		}
		child[where]->SetBoundingBox(tempbox);
	}
	child[where]->CountEdge(mesh, EdgeIndex);
}

void cEdgeOctTree::CountNodes(uint32& nodes) 
{
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
	if (EdgeCount > 0) {
		nodes = nodes + 1;
		}
}

void cEdgeOctTree::FillFlatTree(uint32& nodeIndex, TMCArray<IEdgeTree*>& flatNodeList)
{
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
	if (EdgeCount > 0) {
		flatNodeList[nodeIndex] = this;
		nodeIndex++;
		}
}

void cEdgeOctTree::CacheMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge)
{
	uint32 lNumEdges = mesh->fEdgeList.GetElemCount();
	for (uint32 i = 0; i<lNumEdges;i++) 
	{
		if (drawedge[i])
			CountEdge(mesh, i);
	}
	AllocFacets();
	for (uint32 i = 0; i<lNumEdges;i++) 
	{
		if (drawedge[i])
			AddEdge(mesh, i);
	}
	CountNodes(flatNodeCount);
	flatNodeList.SetElemCount(flatNodeCount);

	uint32 nodeIndex = 0; 
	FillFlatTree(nodeIndex, flatNodeList);
	CalcBoundingBoxes(mesh);

}

long cEdgeOctTree::RankMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge)
{
	long where = 0;
	long EdgeIndex;
	long lNumEdges = mesh->fEdgeList.GetElemCount();
	long lTopLevelFacets = 0;
	for (EdgeIndex = 0; EdgeIndex<lNumEdges;EdgeIndex++) {
		#if (VERSIONNUMBER < 0x040000)
		const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.x];
		const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.y];
		#else
		const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].x];
		const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].y];
		#endif
		if ((mode & OCT_X) == OCT_X) {
			if ((pt1.x < center.x)&&(pt2.x < center.x)) {
				//x low
				}
			else if ((pt1.x >= center.x)&&(pt2.x >= center.x)){
				//x high 
				}
			else {
				//mixed add it to ours
				lTopLevelFacets++;
				goto OneFound;
				}
			}
		if ((mode & OCT_Y) == OCT_Y) {
			if ((pt1.y < center.y)&&(pt2.y < center.y)) {
				//y low
				}
			else if ((pt1.y >= center.y)&&(pt2.y >= center.y)){
				//y high 
				}
			else {
				//mixed add it to ours
				lTopLevelFacets++;
				goto OneFound;
				}
			}
		if ((mode & OCT_Z) == OCT_Z) {
			if ((pt1.z < center.z)&&(pt2.z < center.z)) {
				//z low
				}
			else if ((pt1.z >= center.z)&&(pt2.z >= center.z)){
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

void cEdgeOctTree::AllocFacets()
{
	Edges.SetElemCount(EdgeCount);
	if (child[0] != NULL) {
		child[0]->AllocFacets();
		}
	if (child[1] != NULL) {
		child[1]->AllocFacets();
		}
	if (child[2] != NULL) {
		child[2]->AllocFacets();
		}
	if (child[3] != NULL) {
		child[3]->AllocFacets();
		}
	if (child[4] != NULL) {
		child[4]->AllocFacets();
		}
	if (child[5] != NULL) {
		child[5]->AllocFacets();
		}
	if (child[6] != NULL) {
		child[6]->AllocFacets();
		}
	if (child[7] != NULL) {
		child[7]->AllocFacets();
		}
}
