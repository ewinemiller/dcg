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
#ifndef __IMESHTREE__
#define __IMESHTREE__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCArray.h"
#include "TBBox.h"
#include "I3DShFacetMesh.h"
#include "MCBasicTypes.h"
#include "BasicComImplementations.h"
#include "I3DShFacetMesh.h"
#include "transforms.h"
#include "DCGUtil.h"


typedef struct 
{
		float fTranslation[3];
		float x[3];
		float y[3];
		float z[3];
}DCGTransform3D;

typedef struct 
{
		float fTranslation[2];
		float x[2];
		float y[2];
}DCGTransform2D;


class DCGTriangle
{
	public:
		TVector3 pt1, pt2, pt3;
};

typedef struct
{
	DCGTransform3D Rotation3D[1];
	DCGTransform2D Rotation2D[2];
	real32 flatpt2y[3];
	TVector2 flatpoint;
	long FacetNumber;
	int32 lEdges[3];
	int32 lPoints[3];

} MeshTreeItem;

struct meshTreeCallLocalStorage {
	TMCArray<real32> distance;
	uint32 currentIndex;
	real32 closestDistance;

	meshTreeCallLocalStorage() {
		currentIndex = 0;
	}
};

class IMeshTree {
	public:
		IMeshTree();
		virtual ~IMeshTree();

		//the bounding box of this Node
		TBBox3D				bbox;
		TVector3			center;
		real32				radius;
		MeshTreeItem*		Facets;
		uint32				FacetCount;
		uint32				CurrentFacet;

		//my parent
		IMeshTree* parent;

		virtual void CacheMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges);
		virtual long RankMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges) = 0;

		virtual void CountTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3) = 0;
		virtual MeshTreeItem* AddTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3, const long plFacet) = 0;
		virtual void AllocFacets() = 0;

		virtual void FindClosestNode(IMeshTree** node, const TVector3& point, meshTreeCallLocalStorage& callLocalStorage);
		virtual MCCOMErr enumSiblings(IMeshTree* node, IMeshTree** sibling, meshTreeCallLocalStorage& callLocalStorage);
		virtual void CountNodes(uint32& nodes) = 0;
		virtual void FillFlatTree(uint32& nodeIndex, TMCArray<IMeshTree*>& flatNodeList) = 0;
		
		virtual void CalcBoundingBoxes(FacetMesh* mesh);

		void SetBoundingBox(TBBox3D NewBox);
		void LoadItem(TVector3 pt1, TVector3 pt2, TVector3 pt3, long plFacet, MeshTreeItem& Item);

	protected:
		TMCArray<IMeshTree*> flatNodeList;
		uint32 flatNodeCount;
		static int TIndex3CompareFirstTwo(const void *elem1, const void *elem2)
		{
			//sort by x, then by y
			real32 temp = (static_cast<const TIndex3*>(elem1))->x - (static_cast<const TIndex3*>(elem2))->x;

			if (temp < 0)
				return -1;
			else if (temp > 0)
				return 1;
			else
			{
				temp = (static_cast<const TIndex3*>(elem1))->y - (static_cast<const TIndex3*>(elem2))->y;

				if (temp < 0)
					return -1;
				else
					return (temp != 0);
			}
		}
	private:
		void FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3
			, real32& flatpt2y, DCGTransform3D& Transform);
		void FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3
			, real32& flatpt2y, DCGTransform2D& Transform);

	};

#endif
