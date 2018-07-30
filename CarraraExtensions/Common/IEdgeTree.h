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
#ifndef __IEdgeTree__
#define __IEdgeTree__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "vector3.h"
#include "vector2.h"

#include "copyright.h"
#include "MCArray.h"
#include "TBBox.h"
#include "I3DShFacetMesh.h"
#include "MCBasicTypes.h"
#include "BasicComImplementations.h"
#include "I3DShFacetMesh.h"
#include "transforms.h"
#include "DCGUtil.h"

struct edgeTreeCallLocalStorage {
	TMCArray<real32> distance;
	uint32 currentIndex;
	real32 closestDistance;

	edgeTreeCallLocalStorage() {
		currentIndex = 0;
		closestDistance = FPOSINF;
	}
};

class IEdgeTree {
	public:
		IEdgeTree();
		virtual ~IEdgeTree();

		TBBox3D				bbox;
		TVector3			center;
		TMCArray<uint32> Edges;
		uint32				EdgeCount;
		uint32				CurrentEdge;
		real32 radius;


		virtual void CalcBoundingBoxes(FacetMesh* mesh);
		void SetBoundingBox(TBBox3D NewBox);
		virtual void FindClosestNode(IEdgeTree** node, const TVector3& point, edgeTreeCallLocalStorage& callLocalStorage);
		virtual MCCOMErr enumSiblings(IEdgeTree* node, IEdgeTree** sibling, edgeTreeCallLocalStorage& callLocalStorage);

		virtual void CountNodes(uint32& nodes) = 0;
		virtual void FillFlatTree(uint32& nodeIndex, TMCArray<IEdgeTree*>& flatNodeList) = 0;
		virtual void CacheMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge) = 0;
		virtual long RankMesh(TMCCountedPtr<FacetMesh> mesh, const TMCArray<boolean>& drawedge) = 0;
		virtual void CountEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex) = 0;
		virtual void AddEdge(TMCCountedPtr<FacetMesh> mesh, uint32 EdgeIndex) = 0;
		virtual void AllocFacets() = 0;


	protected:
		TMCArray<IEdgeTree*> flatNodeList;
		uint32 flatNodeCount;
	};

#endif
