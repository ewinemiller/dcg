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
#include "cMeshBucket.h"

cMeshBucket::cMeshBucket() 
{
}

cMeshBucket::~cMeshBucket() 
{
}

MeshTreeItem* cMeshBucket::AddTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3, const long plFacet)
{
	LoadItem(pt1, pt2, pt3, plFacet, Facets[CurrentFacet]);
	CurrentFacet++;
	return &Facets[CurrentFacet - 1];
}

void cMeshBucket::FindClosestNode(IMeshTree** node, const TVector3& point, meshTreeCallLocalStorage& callLocalStorage)
{
	*node = (IMeshTree*)this;
}

MCCOMErr cMeshBucket::enumSiblings(IMeshTree* node, IMeshTree** sibling, meshTreeCallLocalStorage& callLocalStorage)
{
	return MC_S_FALSE;
}


long cMeshBucket::RankMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges)
{
	return mesh->fFacets.GetElemCount();
}


void cMeshBucket::AllocFacets()
{
	if (FacetCount>0) 
	{
		Facets = (MeshTreeItem*)MCcalloc(FacetCount, sizeof(MeshTreeItem));
	}
}

void cMeshBucket::CountTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3)
{
	FacetCount++;
}

void cMeshBucket::CountNodes(uint32& nodes) 
{
}

void cMeshBucket::FillFlatTree(uint32& nodeIndex, TMCArray<IMeshTree*>& flatNodeList)
{
}
