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
#include "cEdgeMeshBucket.h"

cEdgeMeshBucket::cEdgeMeshBucket() 
{
}

cEdgeMeshBucket::~cEdgeMeshBucket() 
{
}

void cEdgeMeshBucket::AddEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex)
{
	Edges[CurrentEdge] = EdgeIndex;
	CurrentEdge++;
}

void cEdgeMeshBucket::FindClosestNode(void** node, const TVector3& point, edgeTreeCallLocalStorage& callLocalStorage)
{
	*node = (IEdgeTree*)this;
}

MCCOMErr cEdgeMeshBucket::enumSiblings(IEdgeTree* node, void** sibling, edgeTreeCallLocalStorage& callLocalStorage)
{
	return MC_S_FALSE;
}

void cEdgeMeshBucket::CacheMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge)
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
}

long cEdgeMeshBucket::RankMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge)
{
	return mesh->fFacets.GetElemCount();
}


void cEdgeMeshBucket::AllocFacets()
{
	Edges.SetElemCount(EdgeCount);
}

void cEdgeMeshBucket::CountEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex)
{
	EdgeCount++;
}

void cEdgeMeshBucket::CountNodes(uint32& nodes) 
{
}

void cEdgeMeshBucket::FillFlatTree(uint32& nodes, TMCArray<IEdgeTree*>& flatNodeList)
{
}
