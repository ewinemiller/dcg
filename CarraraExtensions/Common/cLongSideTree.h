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
#include "IMeshTree.h"

#define OCT_X 4
#define OCT_Y 2
#define OCT_Z 1

class cLongSideTree;

class cLongSideTree:public IMeshTree {
	public:
		cLongSideTree();
		~cLongSideTree();

		void FillFlatTree(uint32& nodeIndex, TMCArray<IMeshTree*>& flatNodeList);

		void AllocFacets();
		long RankMesh(FacetMesh* mesh, const TMCArray<boolean>& drawedge, const TMCArray<TIndex3>& facetEdges);
		void CountTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3);
		MeshTreeItem* AddTriangle(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3, const long plFacet);
		void CountNodes(uint32& nodeIndex);

		uint32 mode;
	private:
		IMeshTree* child[8];
		boolean modeset;
	};

