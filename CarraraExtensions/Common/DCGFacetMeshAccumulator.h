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

//******************************************************************

class DCGFacetMeshAccumulator
{
public:
				DCGFacetMeshAccumulator();
				~DCGFacetMeshAccumulator();
	
	void	AccumulateFacet(const TFacet3D *aF);

			//Call this before accumulating facets to preallocate memory for them
	void	PrepareAccumulation(uint32 estimatedFacetsCount);
	
			//Computes the normals of the facets and does smoothing if abs(smoothAngle) > 1E-5
			//Call this after accumulating the facets and before MakeFacetMesh.
	MCErr	CalculateNormals(real smoothAngle);

	void	MakeFacetMesh(FacetMesh** outMesh);

	uint32	GetAccumulatedFacetsCount()						{ return fOrigFacets.GetElemCount(); }

	TMCArray<TFacet3D>& GetFacetsArray()					{ return fOrigFacets; }

protected:
	struct VertexPlusIndex
	{
		TVertex3D fVertex;
		int32 fNextIndex;
	};

	const TMCArray<VertexPlusIndex>&	GetVertices() const {return fVertices;}
	const TMCArray<IndexedFacet>&		GetFacets() const {return fFacets;}

	TVertex3D*				VertexAt(int32 index) {return (TVertex3D *)&fVertices[index];}

	void					BuildUsingHash(const TMCArray<TFacet3D> *facets,const TMCArray<TFacet3D> *facets2=nil);

	int32					AddVertex(const TVertex3D &avertex);
	int32					HashVertex(const TVertex3D &avertex);
	void					AddFacet(const TFacet3D &afacet);

	TMCArray<VertexPlusIndex>	fVertices;
	TMCArray<IndexedFacet>		fFacets;

	int32*					fHashTable;
	int32					fHashTableSize;
	int16					fHashPower;
	
	TMCArray<TFacet3D>		fOrigFacets;

private:
	friend class IndexedFacetIterator;
};
#endif