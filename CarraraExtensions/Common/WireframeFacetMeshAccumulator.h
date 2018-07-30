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
#ifndef __DCGACCUMULATOR__
#define __DCGACCUMULATOR__

#if CP_PRAGMA_ONCE
#pragma once
#endif
#include "PublicUtilities.h"

#define VERTEX_TOLERANCE 0.00001

extern int TIndex2Compare(const void *elem1, const void *elem2);
extern int TIndex3CompareFirstTwo(const void *elem1, const void *elem2);


//******************************************************************

class WireframeFacetMeshAccumulator
{
public:
				WireframeFacetMeshAccumulator();
				~WireframeFacetMeshAccumulator();
	
	void	AccumulateFacet(const TFacet3D *aF);
	int32	GetVertexIndex(const TVector3& pt1);

			//Call this before accumulating facets to preallocate memory for them
	void	PrepareAccumulation(uint32 estimatedFacetsCount);
	
	void	MakeFacetMesh(FacetMesh** outMesh);

	//uint32	GetAccumulatedFacetsCount()						{ return fOrigFacets.GetElemCount(); }

	//TMCArray<TFacet3D>& GetFacetsArray()					{ return fOrigFacets; }

protected:
	struct VertexPlusIndex
	{
		TVector3 fVertex;
		int32 fNextIndex;
	};
	struct TLocalFacet3D
	{
		TVector3		fVertices[3];		// The facet three vertices
		uint32			fUVSpace;			///< UV Space ID this facet belongs to
	};

	const TMCArray<VertexPlusIndex>&	GetVertices() const {return fVertices;}
	const TMCArray<IndexedFacet>&		GetFacets() const {return fFacets;}

	TVector3*				VertexAt(int32 index) {return (TVector3 *)&fVertices[index];}

	int32					AddVertex(const TVector3 &avertex);
	void					BuildUsingHash(const TMCArray<TLocalFacet3D> *facets);

	int32					HashVertex(const TVector3 &avertex);
	void					AddFacet(const TLocalFacet3D &afacet);

	TMCArray<VertexPlusIndex>	fVertices;
	TMCArray<IndexedFacet>		fFacets;

	int32*					fHashTable;
	int32					fHashTableSize;
	int16					fHashPower;
	
	TMCArray<TLocalFacet3D>		fOrigFacets;

private:
	friend class IndexedFacetIterator;
};
#endif