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
#include "MCArray.h"
#include "TBBox.h"
#include "MCBasicTypes.h"
#include "IEdgeTree.h"

class cEdgeMeshBucket;

class cEdgeMeshBucket:public IEdgeTree {
	public:
		cEdgeMeshBucket();
		~cEdgeMeshBucket();

		void FindClosestNode(void** node,const TVector3& point, edgeTreeCallLocalStorage& callLocalStorage);
		void AllocFacets();
		MCCOMErr enumSiblings(IEdgeTree* node, void** sibling, edgeTreeCallLocalStorage& callLocalStorage);
		void CacheMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge);
		long RankMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge);
		void CountEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex);
		void AddEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex);
		void CountNodes(uint32& nodes);
		void FillFlatTree(uint32& nodes, TMCArray<IEdgeTree*>& flatNodeList);
	};

