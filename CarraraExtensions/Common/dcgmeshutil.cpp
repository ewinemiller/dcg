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
#include "copyright.h"
#include "MCArray.h"
#include "I3dExSkinable.h"
#include "dcgmeshutil.h"
#include "IShComponent.h"
#include "IPolymesh.h"
#include "IPolygon.h"


#if VERSIONNUMBER >= 0x060000
boolean getIsFigurePrimitive(I3DShObject* object) {
	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	TMCCountedPtr<IShComponent> component;
	TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;

	object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
	if(!extprimitive)
	{
		return false;
	}

	extprimitive->GetPrimitiveComponent(&component);
	if(!component)
	{
		return false;
	}

	component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
	if(!primcomp)
	{
		return false;
	}

	primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);

	if (vertexPrimitive && vertexPrimitive->GetIsFigurePrimitive()) {
		return true;
	}
	return false;
}

boolean hasGlobalDeformers(I3DShObject* object) {
	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	TMCCountedPtr<IShComponent> component;
	TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;

	object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
	ThrowIfNil(extprimitive);

	extprimitive->GetPrimitiveComponent(&component);
	ThrowIfNil(component);

	component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
	ThrowIfNil(primcomp);

	primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);

	if (vertexPrimitive) {
		TMCCountedPtr<I3dExSkinable> skinable;

		skinable = vertexPrimitive->GetSkinableInterface();
		ThrowIfNil(skinable);

		return skinable->HasGlobalDeformers();
	}
	return false;
}


void fixBadFigureClone(I3DExVertexPrimitive* vertexPrimitive, 
	I3DExVertexPrimitive* originalVertexPrimitive)
{
	boolean wasOriginalHydrated	= originalVertexPrimitive->IsHydrated();

	originalVertexPrimitive->Hydrate();

	//correct the hidden facet problem on the clone
	long numpolymesh = vertexPrimitive->GetNbPolymeshes();
	for (long polymeshindex = 0; polymeshindex < numpolymesh;polymeshindex++)
	{
		TMCCountedPtr<IPolymesh> polymesh;
		TMCCountedPtr<IPolymesh> originalPolymesh;
		vertexPrimitive->GetPolymesh(&polymesh, polymeshindex);
		originalVertexPrimitive->GetPolymesh(&originalPolymesh, polymeshindex);

		long numPolygons = polymesh->GetNbPolygons();
		for (long polygonIndex = numPolygons - 1; polygonIndex >= 0;polygonIndex--)
		{
			IPolygon* polygon = polymesh->GetPolygonNoAddRef(polygonIndex);
			IPolygon* originalPolygon = originalPolymesh->GetPolygonNoAddRef(polygonIndex);

			if (originalPolygon->Hidden())
			{
				polygon->Delete();
			}
		}
		/*TMCPtrArray<I3DExVertex> outVertices;
		TMCPtrArray<I3DExVertex> outOriginalVertices;

		polymesh->GetVertices(outVertices);
		originalPolymesh->GetVertices(outOriginalVertices);

		long numVertices = outVertices.GetElemCount();
		for (long vertexIndex = numVertices - 1; vertexIndex >= 0;vertexIndex--)
		{
			if (outOriginalVertices[vertexIndex]->GetHidden() && !outVertices[vertexIndex]->GetHidden())
			{
				outVertices[vertexIndex]->SetHidden(true);
				outVertices[vertexIndex]->Delete();
			}
		}*/

	}
	if (!wasOriginalHydrated)
	{
		originalVertexPrimitive->Dehydrate();
	}
}
#endif

void removeBonesAndMorphs(I3DExVertexPrimitive* vertexPrimitive)
{

	TMCCountedPtr<I3dExSkinable> skinable;
	skinable = vertexPrimitive->GetSkinableInterface();
	ThrowIfNil(skinable);


#if (VERSIONNUMBER >= 0x060000)
	skinable->DetachAll();
	skinable->DetachAllGlobalDeformers();
#else
	skinable->DettachAllJoints();
#endif
	//remove all the morph targets

#if (VERSIONNUMBER >= 0x060000)
	uint32 morphareas = vertexPrimitive->GetMorphAreaCount();
	if (morphareas > 0)
	{
		TMCArray<int32> morphIndexList;
		morphIndexList.SetElemCount(morphareas);
		for (uint32 morphAreaIndex = 0; morphAreaIndex < morphareas; morphAreaIndex++)
		{
			morphIndexList[morphAreaIndex] = morphAreaIndex;
			IMorphArea* morphArea = vertexPrimitive->GetMorphAreaNoAddRef(morphAreaIndex);
			uint32 morphTargets = morphArea->GetMorphTargetCount();


			for (uint32 morphTargetIndex = 0; morphTargetIndex < morphTargets; morphTargetIndex++)
			{
				vertexPrimitive->RemoveMorphTarget(morphAreaIndex, 0);
			}
		}
		vertexPrimitive->RemoveMorphAreas(morphIndexList);
	}
#elif (VERSIONNUMBER >= 0x040000)

	uint32 morphareas = vertexPrimitive->GetMorphAreaCount();
	if (morphareas > 0)
	{
		TMCArray<int32> morphIndexList;
		morphIndexList.SetElemCount(morphareas);
		for (uint32 i = 0; i < morphareas; i++)
		{
			morphIndexList[i] = i;
		}
		vertexPrimitive->RemoveMorphAreas(morphIndexList);
	}

#endif
}

TIndex2 buildOrderedEdge(const uint32& vertexIndex1, const uint32& vertexIndex2) {
	TIndex2 returnValue;

	if (vertexIndex1 > vertexIndex2)
	{
		returnValue.Set(vertexIndex2, vertexIndex1);
	}
	else
	{
		returnValue.Set(vertexIndex1, vertexIndex2);
	}
	return returnValue;
}

void addEdge(FacetMesh *mesh, const uint32& vertexIndex1, const uint32& vertexIndex2, const uint32& facetIndex, uint32& edgeCount) {

	TIndex2 edge = buildOrderedEdge(vertexIndex1, vertexIndex2);
	boolean found = false;

	for (uint32 edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++) {

		if (edge == mesh->fEdgeList.fVertexIndices[edgeIndex]) {
			mesh->fEdgeList.fFaceIndices[edgeIndex].y = facetIndex;
			found = true;
			edgeIndex = edgeCount;
		}

	}
	if (found == false) {
		mesh->fEdgeList.fVertexIndices[edgeCount] = edge;
		mesh->fEdgeList.fFaceIndices[edgeCount].x = facetIndex;
		edgeCount++;
	}
}


void buildEdgeList(FacetMesh *mesh){
	mesh->BuildEdgeList();
	/*uint32 edgeCount = 0;

	uint32 facetCount = mesh->fFacets.GetElemCount();
	mesh->fEdgeList.SetElemCount(facetCount * 3);

	for (uint32 facetIndex = 0; facetIndex < facetCount; facetIndex++) {
	addEdge(mesh, mesh->fFacets[facetIndex].pt1, mesh->fFacets[facetIndex].pt2, facetIndex, edgeCount); 
	addEdge(mesh, mesh->fFacets[facetIndex].pt2, mesh->fFacets[facetIndex].pt3, facetIndex, edgeCount); 
	addEdge(mesh, mesh->fFacets[facetIndex].pt3, mesh->fFacets[facetIndex].pt1, facetIndex, edgeCount); 
	}

	mesh->fEdgeList.fFaceIndices.SetElemCount(edgeCount);
	mesh->fEdgeList.fVertexIndices.SetElemCount(edgeCount);*/
}

MCCOMErr getVertexPrimitiveFromObject(I3DShObject* object, I3DExVertexPrimitive** vertexPrimitive)
{
	TMCCountedGetHelper<I3DExVertexPrimitive> result(vertexPrimitive);
	TMCCountedPtr<IShComponent> component;
	TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;

	object->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
	if(!extprimitive)
	{
		return MC_S_FALSE;
	}

	extprimitive->GetPrimitiveComponent(&component);
	if(!component)
	{
		return MC_S_FALSE;
	}

	component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
	if(!primcomp)
	{
		return MC_S_FALSE;
	}

	primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);
	if (vertexPrimitive)
		return MC_S_OK;
	else
		return MC_S_FALSE;

}