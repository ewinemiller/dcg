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
#include "WireframeFacetMeshAccumulator.h"
#include "PublicUtilities.h"

#include "MCException.h"
#include "MCBasicDefines.h"

#include "I3DShTreeElement.h"
#include "I3DShObject.h"
#include "I3DShShader.h"
#include "I3DShCamera.h"
#include "I3DShTweener.h"
#include "I3DShUtilities.h"
#include "ISceneSelection.h"
#include "ISelectableObject.h"

#include "COM3DUtilities.h"
#include "InlineOps.h"

#include "Matrix33.h"

#include "MCRealRect.h"
#include "MCCountedPtrArray.h"
#include "MCCountedPtrHelper.h"
#include "MCRect.h"
#include "MCBasicGUID.h"

#include <stdlib.h> 
//*********************************************************************
int TIndex2Compare(const void *elem1, const void *elem2)
{
	//sort by x, then by y
	real32 temp = (static_cast<const TIndex2*>(elem1))->x - (static_cast<const TIndex2*>(elem2))->x;

	if (temp < 0)
		return -1;
	else if (temp > 0)
		return 1;
	else
	{
		temp = (static_cast<const TIndex2*>(elem1))->y - (static_cast<const TIndex2*>(elem2))->y;

		if (temp < 0)
			return -1;
		else
			return (temp != 0);
	}
}

int TIndex3CompareFirstTwo(const void *elem1, const void *elem2)
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

WireframeFacetMeshAccumulator::WireframeFacetMeshAccumulator()
{
	fHashTable = nil;
	fHashTableSize = 0;
}

WireframeFacetMeshAccumulator::~WireframeFacetMeshAccumulator()
{
	MCfree(fHashTable);
}

void WireframeFacetMeshAccumulator::PrepareAccumulation(uint32 estimatedFacetsCount)
{
	if (fOrigFacets.GetElemSpace() < estimatedFacetsCount)
	{
		fOrigFacets.SetElemSpace(estimatedFacetsCount);
		fOrigFacets.SetGrowSize(estimatedFacetsCount / 10 + 100);
	}
}

void WireframeFacetMeshAccumulator::AddFacet(const TLocalFacet3D& afacet)
{
	IndexedFacet newFacet;

	newFacet.fVertexA = AddVertex(afacet.fVertices[0]);
	newFacet.fVertexB = AddVertex(afacet.fVertices[1]);
	newFacet.fVertexC = AddVertex(afacet.fVertices[2]);
	newFacet.fUVSpaceID = afacet.fUVSpace;
	fFacets.AddElem(newFacet);
}

int32 WireframeFacetMeshAccumulator::GetVertexIndex(const TVector3& avertex)
{
	int32 hash = HashVertex(avertex) & (fHashTableSize - 1);
	int32 foundIndex = -1;

	int32 curIndex = fHashTable[hash];
	while (curIndex >= 0)
	{
		VertexPlusIndex * curV = &fVertices[curIndex];
		TVector3 temp = curV->fVertex - avertex;
		temp.x = fabs(temp.x);
		temp.y = fabs(temp.y);
		temp.z = fabs(temp.z);

//		if (curV->fVertex == avertex)
		if ((temp.x < VERTEX_TOLERANCE)&&(temp.y < VERTEX_TOLERANCE)&&(temp.z < VERTEX_TOLERANCE))
		{
				return curIndex;
		}
		curIndex = curV->fNextIndex;
	}
	return foundIndex;
}


static int32 WireframeFacetMeshAccumulator_HashLong(int32 val)
{
	for (int16 ii = 1; ii < 10; ii++)
	{
		val += 0x12345678 + ((val << 5) | (val >> 27));
	}
	return val;
}

int32 WireframeFacetMeshAccumulator::HashVertex(const TVector3& avertex)
{
	int32 res = WireframeFacetMeshAccumulator_HashLong(RealToFixed(avertex[0]));
	res ^= WireframeFacetMeshAccumulator_HashLong(RealToFixed(avertex[1]));
	res ^= WireframeFacetMeshAccumulator_HashLong(RealToFixed(avertex[2]));
	return res;
}

int32 WireframeFacetMeshAccumulator::AddVertex(const TVector3& avertex)
{
	int32 hash = HashVertex(avertex) & (fHashTableSize - 1);
	int32 foundIndex = -1;

	int32 curIndex = fHashTable[hash];
	while (curIndex >= 0)
	{
		VertexPlusIndex * curV = &fVertices[curIndex];
		TVector3 temp = curV->fVertex - avertex;
		temp.x = fabs(temp.x);
		temp.y = fabs(temp.y);
		temp.z = fabs(temp.z);

//		if (curV->fVertex == avertex)
		if ((temp.x < VERTEX_TOLERANCE)&&(temp.y < VERTEX_TOLERANCE)&&(temp.z < VERTEX_TOLERANCE))
		{
				foundIndex = curIndex;
				break;
		}
		curIndex = curV->fNextIndex;
	}

	if (foundIndex<0)
	{
		VertexPlusIndex newVertexPlusIndex;
		newVertexPlusIndex.fNextIndex = fHashTable[hash];
		newVertexPlusIndex.fVertex = avertex;
		fHashTable[hash] = fVertices.GetElemCount();
		fVertices.AddElem(newVertexPlusIndex);
		foundIndex = fHashTable[hash];
	}
	return foundIndex;
}


void WireframeFacetMeshAccumulator::BuildUsingHash(const TMCArray<TLocalFacet3D> *facets)
{
	uint32 nbrF = 0;
	if (facets)
	{
		//Free some memory if its worth it
		const uint32 facetsCount= facets->GetElemCount();
		if (facetsCount < 0.75*facets->GetElemSpace())
			(const_cast<TMCArray<TLocalFacet3D>*>(facets))->CompactArray();

		nbrF += facetsCount;
	}
	
	//Do some preallocation
	const uint32 growSize= nbrF / 10 + 100;
	fVertices.SetGrowSize(growSize);
	fFacets.SetGrowSize(growSize);
	if (fVertices.GetElemCount() < nbrF)
		fVertices.SetElemSpace(nbrF);
	if (fFacets.GetElemCount() < nbrF)
		fFacets.SetElemSpace(nbrF);

	int32 fHashPower = 1;
	do
	{
		fHashPower++;
		fHashTableSize = 1 << fHashPower;
	}
	while (fHashTableSize < (int32)nbrF);

	fHashTable = (int32 *)MCcalloc(fHashTableSize, sizeof(int32));
	for (uint hashindex = 0; hashindex < fHashTableSize; hashindex++)
        fHashTable[hashindex] = -1;

	if (facets)
	{
		TMCArray<TLocalFacet3D>::const_iterator iter = facets->Begin();
		for (const TLocalFacet3D* aF = iter.First(); iter.More(); aF = iter.Next())
		{
			AddFacet(*aF);
		}
	}

	//MCfree(fHashTable);
	//fHashTable = nil;
	//fHashTableSize = 0;
}


void WireframeFacetMeshAccumulator::AccumulateFacet(const TFacet3D* aF)
{
	if (fOrigFacets.GetElemCount() == fOrigFacets.GetElemSpace())
	{
		uint32 grow= fOrigFacets.GetElemSpace() + 100;
		if (grow > 20000)
			grow = 20000;
		fOrigFacets.SetElemSpace( fOrigFacets.GetElemSpace() + grow );
	}
	TLocalFacet3D newFacet;
	newFacet.fVertices[0] = aF->fVertices[0].fVertex;
	newFacet.fVertices[1] = aF->fVertices[1].fVertex;
	newFacet.fVertices[2] = aF->fVertices[2].fVertex;
	newFacet.fUVSpace = aF->fUVSpace;

	fOrigFacets.AddElem(newFacet);	
}

void WireframeFacetMeshAccumulator::MakeFacetMesh(FacetMesh** outMesh)
{
	FacetMesh::Create(outMesh);
	BuildUsingHash(&fOrigFacets);

	if (fFacets.GetElemCount() != 0)
	{
		uint ii;

		const uint verticesCount= fVertices.GetElemCount();
		(*outMesh)->fVertices.SetElemCount(verticesCount);
		(*outMesh)->fNormals.SetElemCount(verticesCount);
		//(*outMesh)->SetVerticesCount(verticesCount);		
		FacetMesh* mesh= *outMesh;
								
		for (ii= 0; ii < verticesCount; ii++)
		{
			mesh->fVertices[ii]= fVertices[ii].fVertex;
			mesh->fNormals[ii].SetValues(0,0,0);
			//mesh->fuv[ii].SetValues(0,0);			
		}
		
		uint max= fFacets.GetElemCount();
		//(*outMesh)->SetFacetsCount(max);
		(*outMesh)->fFacets.SetElemCount(max);
		(*outMesh)->fUVSpaceID.SetElemCount(max);

		TMCArray<Triangle> &facets = (*outMesh)->fFacets;
		TMCArray<uint32> &uvSpaceID = (*outMesh)->fUVSpaceID;

		for (ii= 0; ii < max; ii++)
		{
			facets[ii].pt1=fFacets[ii].fVertexA;
			facets[ii].pt2=fFacets[ii].fVertexB;
			facets[ii].pt3=fFacets[ii].fVertexC;
			uvSpaceID[ii]=fFacets[ii].fUVSpaceID;
		}
	}	
}
