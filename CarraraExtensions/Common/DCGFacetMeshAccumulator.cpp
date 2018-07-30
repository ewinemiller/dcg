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
#include "DCGFacetMeshAccumulator.h"
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

#include <math.h> 

//*********************************************************************

DCGFacetMeshAccumulator::DCGFacetMeshAccumulator()
{
	fHashTable = nil;
	fHashTableSize = 0;
	//fOrigFacets = new TMCAccumulator<TFacet3D>;
}

DCGFacetMeshAccumulator::~DCGFacetMeshAccumulator()
{
	MCfree(fHashTable);
	//delete fOrigFacets;
}

void DCGFacetMeshAccumulator::PrepareAccumulation(uint32 estimatedFacetsCount)
{
	if (fOrigFacets.GetElemSpace() < estimatedFacetsCount)
	{
		fOrigFacets.SetElemSpace(estimatedFacetsCount);
		fOrigFacets.SetGrowSize(estimatedFacetsCount / 10 + 100);
	}
}

MCErr DCGFacetMeshAccumulator::CalculateNormals(real smoothAngle)
{
	return gShell3DUtilities->CalculateNormals(&fOrigFacets, smoothAngle);
}

void DCGFacetMeshAccumulator::AddFacet(const TFacet3D& afacet)
{
	IndexedFacet newFacet;

	newFacet.fVertexA = AddVertex(afacet.fVertices[0]);
	newFacet.fVertexB = AddVertex(afacet.fVertices[1]);
	newFacet.fVertexC = AddVertex(afacet.fVertices[2]);
	newFacet.fUVSpaceID = afacet.fUVSpace;
	newFacet.fFlags = 0;
	fFacets.AddElem(newFacet);
}


static int32 DCGFacetMeshAccumulator_HashLong(int32 val)
{
	for (int16 ii = 1; ii < 10; ii++)
	{
		val += 0x12345678 + ((val << 5) | (val >> 27));
	}
	return val;
}

int32 DCGFacetMeshAccumulator::HashVertex(const TVertex3D& avertex)
{
	int32 res = DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fVertex[0]));
	res ^= DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fVertex[1]));
	res ^= DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fVertex[2]));
	//res ^= DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fNormal[0]));
	//res ^= DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fNormal[1]));
	//res ^= DCGFacetMeshAccumulator_HashLong(RealToFixed(avertex.fNormal[2]));
	return res;
}

int32 DCGFacetMeshAccumulator::AddVertex(const TVertex3D& avertex)
{
	int32 hash = HashVertex(avertex) & (fHashTableSize - 1);
	int32 foundIndex = -1;

	int32 curIndex = fHashTable[hash];
	while (curIndex)
	{
		VertexPlusIndex * curV = &fVertices[curIndex];
		if ((curV->fVertex.fVertex == avertex.fVertex) && (curV->fVertex.fUV == avertex.fUV))
		{
			real dot = curV->fVertex.fNormal * avertex.fNormal;
			if (dot > FixedToReal(0xfff0) && dot < FixedToReal(0x10010))
			{
				foundIndex = curIndex;
				break;
			}
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


void DCGFacetMeshAccumulator::BuildUsingHash(const TMCArray<TFacet3D> *facets,const TMCArray<TFacet3D> *facets2)
{
	uint32 nbrF = 0;
	if (facets)
	{
		//Free some memory if its worth it
		const uint32 facetsCount= facets->GetElemCount();
		if (facetsCount < 0.75*facets->GetElemSpace())
			(const_cast<TMCArray<TFacet3D>*>(facets))->CompactArray();

		nbrF += facetsCount;
	}
	if (facets2)
	{
		const uint32 facetsCount= facets2->GetElemCount();
		if (facetsCount < 0.75*facets2->GetElemSpace())
			(const_cast<TMCArray<TFacet3D>*>(facets2))->CompactArray();

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

	if (facets)
	{
		TMCArray<TFacet3D>::const_iterator iter = facets->Begin();
		for (const TFacet3D* aF = iter.First(); iter.More(); aF = iter.Next())
		{
			AddFacet(*aF);
		}
	}
	if (facets2)
	{
		TMCArray<TFacet3D>::const_iterator iter2=facets2->Begin();
		for (const TFacet3D* aF2 = iter2.First(); iter2.More(); aF2 = iter2.Next())
		{
			AddFacet(*aF2);
		}
	}

	MCfree(fHashTable);
	fHashTable = nil;
	fHashTableSize = 0;
}


void DCGFacetMeshAccumulator::AccumulateFacet(const TFacet3D* aF)
{
/*
#ifdef Debug_ //checking the normal
	TVector3 calcNormal= (aF->fVertices[1].fVertex - aF->fVertices[0].fVertex)^(aF->fVertices[2].fVertex - aF->fVertices[0].fVertex);	
	boolean calcNormOk= calcNormal.Normalize(1e-6f);
		
	if (calcNormOk)
	{
		real cos0= (aF->fVertices[0].fNormal)*(calcNormal);
		real cos1= (aF->fVertices[1].fNormal)*(calcNormal);
		real cos2= (aF->fVertices[2].fNormal)*(calcNormal);

		if ( (cos0 < 0) || (cos1 < 0) || (cos2 < 0) )
		{
			//one normal (at least) is not in the same direction as the right handed computed normal.
			MCNotify("Invalid facet normal");
		}
	}
	else 
	{
		MCNotify("Warning: Facet may be degenerated");
	}	

#endif
*/
	if (fOrigFacets.GetElemCount() == fOrigFacets.GetElemSpace())
	{
		uint32 grow= fOrigFacets.GetElemSpace() + 100;
		if (grow > 20000)
			grow = 20000;
		fOrigFacets.SetElemSpace( fOrigFacets.GetElemSpace() + grow );
	}
	
	fOrigFacets.AddElem(*aF);	
}

void DCGFacetMeshAccumulator::MakeFacetMesh(FacetMesh** outMesh)
{
	FacetMesh::Create(outMesh);
	BuildUsingHash(&fOrigFacets);

	if (fFacets.GetElemCount() != 0)
	{
		uint ii;

		const uint verticesCount= fVertices.GetElemCount();
		(*outMesh)->SetVerticesCount(verticesCount);		
		FacetMesh* mesh= *outMesh;
								
		for (ii= 0; ii < verticesCount; ii++)
		{
			mesh->fVertices[ii]= fVertices[ii].fVertex.fVertex;
			mesh->fNormals[ii]= fVertices[ii].fVertex.fNormal;
			mesh->fuv[ii]= fVertices[ii].fVertex.fUV;			
		}
		
		uint max= fFacets.GetElemCount();
		(*outMesh)->SetFacetsCount(max);

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
