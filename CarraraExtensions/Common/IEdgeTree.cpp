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
#include "IEdgeTree.h"
#include "math.h"

IEdgeTree::IEdgeTree() {
		flatNodeCount = 0;
		EdgeCount = 0;
		CurrentEdge = 0;
		center.x = 0;
		center.y = 0;
		center.z = 0;
	}

IEdgeTree::~IEdgeTree() 
{
}

void IEdgeTree::SetBoundingBox(TBBox3D NewBox) 
{
	bbox = NewBox;
	bbox.GetCenter(center);
}


void IEdgeTree::CalcBoundingBoxes(FacetMesh* mesh){
	//calculate the real bounding box and center 
	for(uint32 i = 0; i<flatNodeCount;i++) 
	{
		TBBox3D bbox;
		IEdgeTree* node;
		
		node = flatNodeList[i];

		for (uint32 j = 0; j < node->EdgeCount; j++) 
		{
			uint32 EdgeIndex = node->Edges[j];
			#if (VERSIONNUMBER < 0x040000)
			const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.x];
			const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList[EdgeIndex].fVertexIndices.y];
			#else
			const TVector3& pt1 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].x];
			const TVector3& pt2 = mesh->fVertices[mesh->fEdgeList.fVertexIndices[EdgeIndex].y];
			#endif
			bbox.AddPoint(pt1);
			bbox.AddPoint(pt2);
		}
		bbox.GetCenter(node->center);

		node->radius = sqrt(sqr(node->center, bbox.fMax));
	}
}


void IEdgeTree::FindClosestNode(IEdgeTree** node,const TVector3& point, edgeTreeCallLocalStorage& callLocalStorage)
{
	callLocalStorage.closestDistance = FPOSINF;
	if (callLocalStorage.distance.GetElemCount() != flatNodeCount)
	{
		callLocalStorage.distance.SetElemCount(flatNodeCount);
	}
	callLocalStorage.currentIndex = 0;

	real32 fTemp;
	for(uint32 i = 0; i < flatNodeCount; i++) 
	{
		callLocalStorage.distance[i] = sqrt(sqr(point, flatNodeList[i]->center));
		fTemp = callLocalStorage.distance[i] + flatNodeList[i]->radius;
		if (fTemp < callLocalStorage.closestDistance) 
		{
			callLocalStorage.closestDistance = fTemp;
			*node = flatNodeList[i];
		}
	}
}

MCCOMErr IEdgeTree::enumSiblings(IEdgeTree* node, IEdgeTree** sibling, edgeTreeCallLocalStorage& callLocalStorage)
{
	while (callLocalStorage.currentIndex < flatNodeCount) 
	{
		if ((callLocalStorage.distance[callLocalStorage.currentIndex]
			- flatNodeList[callLocalStorage.currentIndex]->radius
			<= callLocalStorage.closestDistance)
			&&(flatNodeList[callLocalStorage.currentIndex] != node))
		{
			*sibling = flatNodeList[callLocalStorage.currentIndex];
			callLocalStorage.currentIndex++;
			return MC_S_OK;
		}
		callLocalStorage.currentIndex++;
	}
	callLocalStorage.currentIndex = 0;
	return MC_S_FALSE;
}