/*  Shader Plus - plug-in for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#include "Bevel.h"
#include "WireframeFacetMeshAccumulator.h"

#include "cOctTree.h"
#include "cMeshBucket.h"
#include "cLongSideTree.h"

#include "ShadersPlusDLL.h"
#include "I3DExVertexPrimitive.h"
#include "I3dExPrimitive.h"
#include "I3DShObject.h"
#include "IShComponent.h"
//#include "IPolymesh.h"
#if (VERSIONNUMBER >= 0x040000)
#include "IEdge.h"
#else
#endif
#include <stdlib.h>
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "MCCountedPtrArray.h"
#include "I3dExSkinable.h"
#include "../AnythingGrows/AnythingGrowsDeformer.h"
#include "MFPartMessages.h"
#include "commessages.h"

#include "IMFCurvePart.h"
#include "I3DShModifier.h"
#include "I3DShObject.h"

#ifdef WIN32
#include <search.h>
#endif
#include <string.h>
#include <stdio.h>
#include "dcgdebug.h"

//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

#include "DCGEdgeShaderTools.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Bevel(R_CLSID_Bevel);
const MCGUID IID_Bevel(R_IID_Bevel);
const MCGUID IID_AnythingGrowsDeformer(R_IID_AnythingGrowsDeformer);
#else
const MCGUID CLSID_Bevel={R_CLSID_Bevel};
const MCGUID IID_Bevel={R_IID_Bevel};
const MCGUID IID_AnythingGrowsDeformer={R_IID_AnythingGrowsDeformer};
#endif

DCGSharedCache<BevelCache, LightingContext, BevelKey> bevelCache;

Bevel::Bevel():DCGSharedCacheClient<BevelCache, LightingContext, BevelKey>(bevelCache)
{
	fData.fWireSize = .25;
	fData.iSpace = SPACE_LOCAL;
	fData.fVectorAngle = 30;
	fData.lFalloff = foInvRadial;
	fData.bEdgeInner = true;
	fData.bEdgeOuter = true;
	fData.bGrowsSafe = true;
	fData.bSoftenPoints = true;
#if (VERSIONNUMBER >= 0x050000)
	defaultlightingmodel = NULL;
#endif
}

Bevel::~Bevel(){
	releaseCache();
}


void* Bevel::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Bevel::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}

	if (globalStorageKey.iSpace != fData.iSpace
		|| globalStorageKey.fVectorAngle != fData.fVectorAngle
		|| globalStorageKey.bEdgeInner != fData.bEdgeInner
		|| globalStorageKey.bEdgeOuter != fData.bEdgeOuter
		|| globalStorageKey.bGrowsSafe != fData.bGrowsSafe)
	{
		releaseCache();
	}
	return MC_S_OK;
	}

boolean	Bevel::IsEqualTo(I3DExShader* aShader){
	return (false);
	}  

MCCOMErr Bevel::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.CombineFlagsWith(ChildFlags);
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
	}

EShaderOutput Bevel::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
	+ kUseCalculateDirectLighting
	+ kUseCalculateIndirectLighting
#endif
	);
}

void Bevel::CheckNode(const BevelCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point	, int32& lEdgeFound, real32& fAlongEdge
				, boolean& usePoint)
{
	MeshTreeItem* Item;
	long FacetsInNode = Node->FacetCount;

	for(long j = 0; j<FacetsInNode;j++) 
	{
		Item = &Node->Facets[j];
		CheckMeshTreeItem(instance, Node, mindistance
						  , point
						  , lEdgeFound, fAlongEdge, Item
						  , usePoint);
	}//end for
}

void Bevel::CheckMeshTreeItem(const BevelCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point
				, int32& lEdgeFound, real32& fAlongEdge, MeshTreeItem* Item
				, boolean& usePoint)
{
	TVector3 planept;
	TVector3 temppt;
	real32 fTemp;
	//i = 0
	temppt.x = point.x - Item->Rotation3D[0].fTranslation[0];	
	temppt.y = point.y - Item->Rotation3D[0].fTranslation[1];	
	temppt.z = point.z - Item->Rotation3D[0].fTranslation[2];
	planept.z = temppt.x * Item->Rotation3D[0].x[2]
			+ temppt.y * Item->Rotation3D[0].y[2]
			+ temppt.z * Item->Rotation3D[0].z[2];
	real32 z = fabs(planept.z);
	//check to see if it's in our plane, otherwise we don't really care about it
	if (z <= VERTEX_TOLERANCE)
	{
		//transform the rest
		planept.x = temppt.x * Item->Rotation3D[0].x[0]
				+ temppt.y * Item->Rotation3D[0].y[0]
				+ temppt.z * Item->Rotation3D[0].z[0];
		planept.y = temppt.x * Item->Rotation3D[0].x[1]
				+ temppt.y * Item->Rotation3D[0].y[1]
				+ temppt.z * Item->Rotation3D[0].z[1];
		
		if (instance.drawedge[Item->lEdges[0]] && planept.x >= 0)
		{
			
			if (planept.y > Item->flatpt2y[0])
			{
				fTemp = sqrt(sqr(planept, TVector3(0, Item->flatpt2y[0], 0)));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[0];
					fAlongEdge = 1.0f;
					if (Item->lPoints[0] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[0]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else if(planept.y < 0)
			{
				fTemp = sqrt(sqr(planept, TVector3::kZero));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[0];
					fAlongEdge = 0;
					if (Item->lPoints[0] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[0]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else
			{
				fTemp = planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[0];
					fAlongEdge = planept.y / Item->flatpt2y[0];
					if (Item->lPoints[0] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[0]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = instance.usepoint[lEdgeFound];
				}

			}
		}

		//i = 1
		temppt.x = planept.x - Item->Rotation2D[0].fTranslation[0];	
		temppt.y = planept.y - Item->Rotation2D[0].fTranslation[1];	

		planept.x = temppt.x * Item->Rotation2D[0].x[0]
				+ temppt.y * Item->Rotation2D[0].y[0];
		planept.y = temppt.x * Item->Rotation2D[0].x[1]
				+ temppt.y * Item->Rotation2D[0].y[1];

		if (instance.drawedge[Item->lEdges[1]] && planept.x >= 0)
		{
			
			if (planept.y > Item->flatpt2y[1])
			{
				fTemp = sqrt(sqr(planept, TVector3(0, Item->flatpt2y[1], 0)));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[1];
					fAlongEdge = 1.0f;
					if (Item->lPoints[1] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[1]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else if(planept.y < 0)
			{
				fTemp = sqrt(sqr(planept, TVector3::kZero));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[1];
					fAlongEdge = 0;
					if (Item->lPoints[1] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[1]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else
			{
				fTemp = planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[1];
					fAlongEdge = planept.y / Item->flatpt2y[1];
					if (Item->lPoints[1] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[1]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = instance.usepoint[lEdgeFound];
				}

			}
		}
		//i = 2
		temppt.x = planept.x - Item->Rotation2D[1].fTranslation[0];	
		temppt.y = planept.y - Item->Rotation2D[1].fTranslation[1];	

		planept.x = temppt.x * Item->Rotation2D[1].x[0]
				+ temppt.y * Item->Rotation2D[1].y[0];
		planept.y = temppt.x * Item->Rotation2D[1].x[1]
				+ temppt.y * Item->Rotation2D[1].y[1];

		if (instance.drawedge[Item->lEdges[2]] && planept.x >= 0)
		{
			
			if (planept.y > Item->flatpt2y[2])
			{
				fTemp = sqrt(sqr(planept, TVector3(0, Item->flatpt2y[2], 0)));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[2];
					fAlongEdge = 1.0f;
					if (Item->lPoints[2] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[2]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else if(planept.y < 0)
			{
				fTemp = sqrt(sqr(planept, TVector3::kZero));
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[2];
					fAlongEdge = 0;
					if (Item->lPoints[2] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[2]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = true;
				}
			}
			else
			{
				fTemp = planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[2];
					fAlongEdge = planept.y / Item->flatpt2y[2];
					if (Item->lPoints[2] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[2]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
					usePoint = instance.usepoint[lEdgeFound];
				}

			}
		}
	}//it's z more or less = 0, on our plane
}

void Bevel::BuildCache(BevelCache& instance, const LightingContext& lightingContext)
{

	TMCCountedPtr<FacetMesh>	amesh;		
	TMCCountedPtr<FacetMesh>	originalmesh;		
	TMCArray<TIndex3> facetEdges;
	TMCArray<TIndex3> vertexToEdge;
	TMCArray<TVector3> facenormals;

	TBBox3D bbox;

	const ShadingIn& shadingIn = static_cast<ShadingIn>(*lightingContext.fHit);
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShModifier> modifier;
	TMCCountedPtr<AnythingGrowsDeformer> agrodef;

	FacetMeshFacetIterator facetIterator;
	WireframeFacetMeshAccumulator accu;

	//we want a mesh where matching vertices are 
	//considered a single vertex, so blank the normal
	//and UV values
	tree = shadingIn.fInstance->GetTreeElement();

	uint32 modifiercount = tree->GetModifiersCount();
	boolean bHasGrows = false;
	for(uint32 i = 0; i < modifiercount; i++) 
	{
		if (tree->GetModifierByIndex(&modifier, i) == MC_S_OK) 
		{
			if (modifier->QueryInterface(IID_AnythingGrowsDeformer,(void**)&agrodef)==MC_S_OK) 
			{
				//it's got an anything grows modifier
				bHasGrows = true;
			}
		}
	}
	if (bHasGrows && fData.bGrowsSafe)
	{
		TMCCountedPtr<I3DShObject> object;
		shadingIn.fInstance->Get3DObject(&object);
		if (object != NULL)
		{
			TMCCountedPtr<I3DShPrimitive> primitive;
			object->QueryInterface(IID_I3DShPrimitive, (void**)&primitive);
			if (primitive != NULL)
			{
#if VERSIONNUMBER >= 0x050000
			{
				//CWhileInCS cs(gBackgroundCS);
				primitive->GetRenderingFacetMesh(&originalmesh);
			}
#else
				primitive->GetFMesh(0.00025000001f, &originalmesh);
#endif
			}
		}
	}
	else
	{
		if (lightingContext.fHit->fFacetMesh == NULL)
		{
#if VERSIONNUMBER >= 0x050000
			{
				//CWhileInCS cs(gBackgroundCS);
				originalmesh = shadingIn.fInstance->GetRenderingFacetMesh();
			}
#else
            shadingIn.fInstance->GetFMesh(0.00025000001f, &originalmesh);
#endif

		}
		else
		{
            originalmesh = lightingContext.fHit->fFacetMesh;
		}
	}

	originalmesh->BuildEdgeList();
	uint32 originalmaxedges = originalmesh->fEdgeList.GetElemCount();
	uint32 originalmaxfacets = originalmesh->fFacets.GetElemCount();
	facetIterator.Initialize(originalmesh);			
	accu.PrepareAccumulation(originalmaxfacets);

	for (facetIterator.First(); facetIterator.More(); facetIterator.Next()) 
	{
		TFacet3D aFacet = facetIterator.GetFacet();
		accu.AccumulateFacet(&aFacet);
	}
	accu.MakeFacetMesh(&amesh);
	amesh->BuildEdgeList();
	uint32 maxedges = amesh->fEdgeList.GetElemCount();
	uint32 maxvertices = amesh->fVertices.GetElemCount();
	uint32 maxfacets = amesh->fFacets.GetElemCount();
	instance.mesh = amesh;


	//calculate the normals at the points
	instance.pointnormal.SetElemCount(maxvertices);
	instance.usepoint.SetElemCount(maxedges);
	{
		TMCArray<uint32> numpoints(maxvertices, true);
		uint32 originalmaxvertices = originalmesh->fVertices.GetElemCount();
		for (uint32 vertexindex = 0; vertexindex < originalmaxvertices; vertexindex++)
		{
			uint32 newindex = accu.GetVertexIndex(originalmesh->fVertices[vertexindex]);
			if (newindex >= 0 && newindex <= maxvertices - 1)
			{
				instance.pointnormal[newindex] += originalmesh->fNormals[vertexindex];
				numpoints[newindex]++;
			}
			/*else
			{
				Alert("bad mesh");
				for (uint32 i = 0; i < originalmaxfacets; i++)
				{
					boolean bfound = false;
					Triangle facet = originalmesh->fFacets[i];
					if ((facet.pt1 == vertexindex)||
						(facet.pt2 == vertexindex)||
						(facet.pt3 == vertexindex))
					{ 
						bfound = true;
					}
				}
					int i = 5;
			}*/
		}
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			instance.pointnormal[vertexindex] = instance.pointnormal[vertexindex]/numpoints[vertexindex];
		}
	}
		
	//run through the original mesh and write down which facets are flat
	//flat = all normals match
	TMCArray<boolean> facetflat(originalmaxfacets, true);
	for (uint32 facetindex = 0; facetindex < originalmaxfacets; facetindex++)
	{
		facetflat[facetindex] = ((originalmesh->fNormals[originalmesh->fFacets[facetindex].pt1] 
			== originalmesh->fNormals[originalmesh->fFacets[facetindex].pt2] )
			&&
			(originalmesh->fNormals[originalmesh->fFacets[facetindex].pt3] 
			== originalmesh->fNormals[originalmesh->fFacets[facetindex].pt2] ));

	}
	//run through all the edges those with bounding flat facets use
	//edge style mix, otherwise point style
	//perimeter edges always use point style
	//sort and order the new edge list
	vertexToEdge.SetElemCount(maxedges);
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{
		int32 vindexx = amesh->fEdgeList.fVertexIndices[edgeindex].x;
		int32 vindexy = amesh->fEdgeList.fVertexIndices[edgeindex].y;
		if (vindexx > vindexy)
		{
			vertexToEdge[edgeindex].x = vindexy;
			vertexToEdge[edgeindex].y = vindexx;

		}
		else
		{
			vertexToEdge[edgeindex].x = vindexx;
			vertexToEdge[edgeindex].y = vindexy;

		}
		vertexToEdge[edgeindex].z = edgeindex;
		instance.usepoint[edgeindex] = false;
	}
	qsort(vertexToEdge.Pointer(0), vertexToEdge.GetElemCount(), sizeof(TIndex3), TIndex3CompareFirstTwo);

	for (uint32 edgeindex = 0; edgeindex < originalmaxedges; edgeindex++)
	{
		int32 vindexx = originalmesh->fEdgeList.fVertexIndices[edgeindex].x;
		int32 vindexy = originalmesh->fEdgeList.fVertexIndices[edgeindex].y;
		int32 indexx = originalmesh->fEdgeList.fFaceIndices[edgeindex].x;
		int32 indexy = originalmesh->fEdgeList.fFaceIndices[edgeindex].y;

		int32 newvindexx = accu.GetVertexIndex(originalmesh->fVertices[vindexx]);
		int32 newvindexy = accu.GetVertexIndex(originalmesh->fVertices[vindexy]);
		TIndex3 searchelement;
		if (newvindexx > newvindexy)
		{
			searchelement.x = newvindexy;
			searchelement.y = newvindexx;

		}
		else
		{
			searchelement.x = newvindexx;
			searchelement.y = newvindexy;

		}

		TIndex3* foundelement = (TIndex3*)bsearch(&searchelement, vertexToEdge.Pointer(0)
				, vertexToEdge.GetElemCount(), sizeof(TIndex3), TIndex3CompareFirstTwo);
		if (foundelement)
		{
			uint32 newedgeindex = foundelement->z;
			if (!facetflat[indexx])
			{
				instance.usepoint[newedgeindex] = true;
			}
			if (indexy != -1 && !facetflat[indexy])
			{
				instance.usepoint[newedgeindex] = true;
			}
		}
	}




	//if needed, run through the vertices and translate them to global space
	if (fData.iSpace == SPACE_GLOBAL)
	{
		//transform all our points into screen space
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			LocalToGlobal(lightingContext.fHit->fT, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);

		}
	}

	facenormals.SetElemCount(maxfacets);
	facetEdges.SetElemCount(maxfacets);
	//calculate the normal for each facet
	for (uint32 facetIndex = 0; facetIndex < maxfacets; facetIndex++)
	{
		const Triangle& facet = amesh->fFacets[facetIndex];

		const TVector3& globalCoordPoint0 = amesh->fVertices[facet.pt1];
		const TVector3& globalCoordPoint1 = amesh->fVertices[facet.pt2];
		const TVector3& globalCoordPoint2 = amesh->fVertices[facet.pt3];

		const uint32 vertices[4] = {facet.pt1, facet.pt2, facet.pt3, facet.pt1};

		//we don't want to draw lines for facets facing away from us
		facenormals[facetIndex] = 
			(globalCoordPoint1 - globalCoordPoint0)
			^ (globalCoordPoint2 - globalCoordPoint1);
		facenormals[facetIndex].Normalize(facenormals[facetIndex]);

		for (uint32 edgeIndex = 0; edgeIndex < 3;edgeIndex++)
		{
			TIndex3 vertexKey(vertices[edgeIndex], vertices[edgeIndex + 1], 0);

			//make lowest the first so we always match the array
			if (vertexKey.x > vertexKey.y)
			{
				vertexKey.x = vertices[edgeIndex + 1];
				vertexKey.y = vertices[edgeIndex];
			}

			TIndex3* vertexToEdgeItem = static_cast<TIndex3*>(bsearch(&vertexKey, vertexToEdge.Pointer(0)
				, vertexToEdge.GetElemCount(), sizeof(TIndex3), TIndex3CompareFirstTwo));

			facetEdges[facetIndex][edgeIndex] = vertexToEdgeItem->z;
		}

	}

	real32 fAngleThreshold = fData.fVectorAngle / 360 * 6.283185307179586476925286766559;

	//clear out what we don't need
	amesh->fNormals.ArrayFree();
	amesh->fuv.ArrayFree();
	amesh->fUVSpaceID.ArrayFree();
	amesh->fPolygonEqns.ArrayFree();
	amesh->fPolygonColors.ArrayFree();
	amesh->fPolygonBackColors.ArrayFree();
	vertexToEdge.ArrayFree();

	//run through the edges and mark which should be drawn
	instance.drawedge.SetElemCount(maxedges);
	instance.edgenormal.SetElemCount(maxedges);
	instance.linemagnitude.SetElemCount(maxedges);

	//precalc the edge values
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{
		real32 edgeangle;
		#if (VERSIONNUMBER < 0x040000)
		int32 indexx = amesh->fEdgeList[edgeindex].fFaceIndices.x;
		int32 indexy = amesh->fEdgeList[edgeindex].fFaceIndices.y;
		#else
		int32 indexx = amesh->fEdgeList.fFaceIndices[edgeindex].x;
		int32 indexy = amesh->fEdgeList.fFaceIndices[edgeindex].y;
		#endif
		int32 originalindexy = indexy;

		real32 edgecos; 
		if (indexy == -1)
		{
			edgeangle = 0;
		}//no indexy defined in mesh

		if (indexy != -1)
		{
			//create a vector that lies on the second face
			//check to see how far off it is from the first face
			//normal to see if this edge is convex or concave
			edgecos = facenormals[indexx] * facenormals[indexy];
			TVector3 planevector;
			const TVector3& normalx = facenormals[indexx];
			const TVector3& normaly = facenormals[indexy];
			TVector3 edgenorm = (normalx + normaly) * 0.5f;
			instance.edgenormal[edgeindex] = edgenorm;
			#if (VERSIONNUMBER < 0x040000)
			int32 vindexx = amesh->fEdgeList[edgeindex].fVertexIndices.x;
			int32 vindexy = amesh->fEdgeList[edgeindex].fVertexIndices.y;
			#else
			int32 vindexx = amesh->fEdgeList.fVertexIndices[edgeindex].x;
			int32 vindexy = amesh->fEdgeList.fVertexIndices[edgeindex].y;
			#endif

			if ((amesh->fFacets[indexx].pt1 == vindexx && amesh->fFacets[indexx].pt2 == vindexy)
				||
				(amesh->fFacets[indexx].pt2 == vindexx && amesh->fFacets[indexx].pt1 == vindexy))
			{
				//pt3 not on edge
				planevector = amesh->fVertices[amesh->fFacets[indexx].pt3]
					- amesh->fVertices[amesh->fFacets[indexx].pt1];

			}
			else if ((amesh->fFacets[indexx].pt2 == vindexx && amesh->fFacets[indexx].pt3 == vindexy)
				||
				(amesh->fFacets[indexx].pt3 == vindexx && amesh->fFacets[indexx].pt2 == vindexy))
			{
				//pt1 not on edge
				planevector = amesh->fVertices[amesh->fFacets[indexx].pt1]
					- amesh->fVertices[amesh->fFacets[indexx].pt2];
			}
			else
			{
				//pt2 not on edge
				planevector = amesh->fVertices[amesh->fFacets[indexx].pt2]
					- amesh->fVertices[amesh->fFacets[indexx].pt3];

			}

			real32 sign = facenormals[indexy] * planevector;
			if (sign < 0)
			{
				edgeangle = acos(edgecos);
			}
			else
			{
				edgeangle = -acos(edgecos);
			}
		}
		else
		{
			instance.edgenormal[edgeindex] = facenormals[indexx];
		}

		instance.drawedge[edgeindex] = false;
		//if the angle between the facets is too big, don't
		//draw this edge
		if (edgeangle != 0)
		{
			if ((edgeangle > fAngleThreshold) && fData.bEdgeOuter)
			{
				instance.drawedge[edgeindex] = true;
			}
			if ((edgeangle < -fAngleThreshold) && fData.bEdgeInner)
			{
				instance.drawedge[edgeindex] = true;
			}
		}
		#if (VERSIONNUMBER < 0x040000)
		const TVector3& p1 = amesh->fVertices[amesh->fEdgeList[edgeindex].fVertexIndices.x];
		const TVector3& p2 = amesh->fVertices[amesh->fEdgeList[edgeindex].fVertexIndices.y];
		#else
		const TVector3& p1 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].y];
		#endif
		//if the end points are the same, don't draw this edge
		if (p1 == p2)
		{
			instance.drawedge[edgeindex] = false;
			instance.linemagnitude[edgeindex] = 0;
		}
		else
		{
			instance.linemagnitude[edgeindex] = sqrt(sqr(p1, p2));
		}

	}//end edge loop

	amesh->CalcBBox(bbox);
	cOctTree* octtree;
	int32 bestoption;
	uint32 bestrank;	
	uint32 currentrank;
	if (maxedges <= 100)
	{
		bestoption = OPT_NONE;
	}
	else 
	{
		//octal tree
		bestoption = OPT_NONE;
		instance.meshtree = new cOctTree;
		instance.meshtree->SetBoundingBox(bbox);
		bestrank = instance.meshtree->RankMesh(amesh, instance.drawedge, facetEdges);
		delete instance.meshtree;
		//xy quadrant
		octtree = new cOctTree;
		octtree->mode = OCT_X + OCT_Y;
		instance.meshtree = octtree;
		instance.meshtree->SetBoundingBox(bbox);
		currentrank = instance.meshtree->RankMesh(amesh, instance.drawedge, facetEdges);
		if (currentrank < bestrank) {
			bestrank = currentrank;
			bestoption = OPT_XY_QUADRANT;
			}
		delete instance.meshtree;
		//xz quadrant
		octtree = new cOctTree;
		octtree->mode = OCT_Z + OCT_Y;
		instance.meshtree = octtree;
		instance.meshtree->SetBoundingBox(bbox);
		currentrank = instance.meshtree->RankMesh(amesh, instance.drawedge, facetEdges);
		if (currentrank < bestrank) {
			bestrank = currentrank;
			bestoption = OPT_XZ_QUADRANT;
			}
		delete instance.meshtree;
		//yz quadrant
		octtree = new cOctTree;
		octtree->mode = OCT_Y + OCT_Z;
		instance.meshtree = octtree;
		instance.meshtree->SetBoundingBox(bbox);
		currentrank = instance.meshtree->RankMesh(amesh, instance.drawedge, facetEdges);
		if (currentrank < bestrank) {
			bestrank = currentrank;
			bestoption = OPT_YZ_QUADRANT;
			}
		delete instance.meshtree;
		//long side
		instance.meshtree = new cLongSideTree;
		instance.meshtree->SetBoundingBox(bbox);
		currentrank = instance.meshtree->RankMesh(amesh, instance.drawedge, facetEdges);
		if (currentrank < bestrank) {
			bestrank = currentrank;
			bestoption = OPT_LONG_SIDE;
			}
		delete instance.meshtree;
	}
	LoadMeshTree(instance, amesh, bestoption, facetEdges, bbox);
	
}
boolean Bevel::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

#if VERSIONNUMBER >= 0x070000
MCCOMErr Bevel::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#elif VERSIONNUMBER >= 0x050000
MCCOMErr Bevel::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#else
void Bevel::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader)
#endif
{
	#if (VERSIONNUMBER >= 0x050000)
		defaultlightingmodel = inDefaultLightingModel;
	#endif
	if (!shader) {
#if VERSIONNUMBER >= 0x050000
		return MC_E_NOTIMPL;
#else
		return;
#endif	
	}
	const ShadingIn& shadingIn = static_cast<ShadingIn>(*lightingContext.fHit);

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	real currentTime;

	shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
	ThrowIfNil(tree);
	tree->GetScene(&scene);
	ThrowIfNil(scene);

	scene->GetTime(&currentTime);


	if (cacheElement == NULL || globalStorageKey.instance != shadingIn.fInstance || globalStorageKey.currentTime != currentTime)
	{
		if (globalStorageKey.currentTime != currentTime)
		{
			releaseCache();
		}
		globalStorageKey.fill(shadingIn.fInstance, fData, currentTime);
		getCache(lightingContext);
	}


	newnormal = shadingIn.fNormalLoc;
	newgnormal = shadingIn.fGNormal;

	real32 fWireSize = fData.fWireSize;

	TVector3 p = shadingIn.fPointLoc;
	if (fData.iSpace == SPACE_GLOBAL)
	{
		p = shadingIn.fPoint;
	}

	real32 mindistance = FPOSINF;

	IMeshTree* ClosestNode = NULL;
	IMeshTree* CurrentNode = NULL;

	cacheElement->meshtree->FindClosestNode (&ClosestNode, p, callLocalStorage);
	if (ClosestNode != NULL)
	{
		int32 closestedge = -1;
		real32 fAlongEdge;
		boolean usepoint = false;
		CheckNode(*cacheElement, ClosestNode, mindistance, p, closestedge, fAlongEdge, usepoint);
		callLocalStorage.closestDistance = min(mindistance, callLocalStorage.closestDistance);
		while (cacheElement->meshtree->enumSiblings(ClosestNode, &CurrentNode, callLocalStorage)==MC_S_OK) 
		{
			CheckNode (*cacheElement, CurrentNode, mindistance, p, closestedge, fAlongEdge, usepoint);
			callLocalStorage.closestDistance = min(mindistance, callLocalStorage.closestDistance);
		}//end while MC_S_OK

			if (mindistance <= fWireSize)
			{
				boolean doBlend = false;
				real32 fBlend;

				if (usepoint == false && fData.bSoftenPoints)
				{
					const TVector3& p1 = cacheElement->mesh->fVertices[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].x];
					const TVector3& p2 = cacheElement->mesh->fVertices[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].y];
					real32 distance;
					real32 linemagnitude = cacheElement->linemagnitude[closestedge];
					if (fAlongEdge < 0.5f)
					{
						//distance = linemagnitude * fAlongEdge;// sqrt(sqr(p, p1));
						distance = sqrt(sqr(p, p1));
					}
					else
					{
						//distance = linemagnitude * (1.0f - fAlongEdge);//sqrt(sqr(p, p2));
						distance = sqrt(sqr(p, p2));
					}
					if (distance < fWireSize)
					{
						fBlend = (distance / fWireSize);
						doBlend = true;
					}
				}
				real32 fFarAlong = 0.0f;
				switch (fData.lFalloff)
				{
					case foAbsolute:
						fFarAlong = 1.0f;
						break;
					case foLinear:
						fFarAlong = (fWireSize - mindistance) / fWireSize;
						break;
					case foRadial:
						fFarAlong = cos(mindistance / fWireSize * HALF_PI);
						break;
					case foInvRadial:
						fFarAlong = cos(HALF_PI + mindistance / fWireSize * HALF_PI) + 1.0f;
						break;
					case foCustom:
						fFarAlong = fBezierCurve.GetYPos(mindistance / fWireSize);
						break;
				} 
				TVector3 edgenormal;
				if (doBlend)
				{
					edgenormal = cacheElement->edgenormal[closestedge];
					newnormal = (edgenormal * fFarAlong + shadingIn.fNormalLoc * (1.0f - fFarAlong));

					fBlend = sin(fBlend * HALF_PI);

					const TVector3& n1 = cacheElement->pointnormal[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].x];
					const TVector3& n2 = cacheElement->pointnormal[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].y];
					if (fAlongEdge > 0.5f)
					{
						newnormal = (1.0f - fBlend) * n2 + fBlend * newnormal;
					}
					else
					{
						newnormal = (1.0f - fBlend) * n1 + fBlend * newnormal;
					}
				}
				else if (usepoint)
				{
					const TVector3& n1 = cacheElement->pointnormal[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].x];
					const TVector3& n2 = cacheElement->pointnormal[cacheElement->mesh->fEdgeList.fVertexIndices[closestedge].y];
					edgenormal = n1 * (1.0f - fAlongEdge) + n2 * fAlongEdge;
					newnormal = (edgenormal * fFarAlong + shadingIn.fNormalLoc * (1.0f - fFarAlong));
				}
				else
				{
					edgenormal = cacheElement->edgenormal[closestedge];
					newnormal = (edgenormal * fFarAlong + shadingIn.fNormalLoc * (1.0f - fFarAlong));
				}
				if (lightingContext.fHit->fNormalFlipped)
				{
					edgenormal = -edgenormal;
					newnormal = (edgenormal * fFarAlong + shadingIn.fNormalLoc * (1.0f - fFarAlong));
				}
				LocalToGlobalVector(lightingContext.fHit->fT, newnormal, newgnormal);
				newgnormal.Normalize(newgnormal);
				
			}
	}
		
#if VERSIONNUMBER >= 0x050000
	return MC_E_NOTIMPL;
#else
	shader->ShadeAndLight(result, lightingContext, inShader);
#endif

}

void Bevel::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
	if (shader != NULL)
	{
#if (VERSIONNUMBER >= 0x050000)
		if ((ChildOutput & kUseCalculateDirectLighting) == kUseCalculateDirectLighting)
		{
			(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateDirectLighting(result, lightingContext);
		}
		else
		{
			defaultlightingmodel->CalculateDirectLighting(result, lightingContext);
		}
#else
		(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateDirectLighting(result, lightingContext);
#endif
	}
}

#if (VERSIONNUMBER >= 0x070000)
void Bevel::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void Bevel::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{
	if (shader != NULL)
	{
#if (VERSIONNUMBER >= 0x050000)
		if ((ChildOutput & kUseCalculateIndirectLighting) == kUseCalculateIndirectLighting)
		{
#if (VERSIONNUMBER >= 0x070000)
				(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
				(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
		else
		{
#if (VERSIONNUMBER >= 0x070000)
				defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
				defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
#else
		(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
	}
}

MCCOMErr Bevel::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{

	if (shader != NULL)
	{
		shadingIn.fNormalLoc = newnormal;
		shadingIn.fGNormal = newgnormal;

		shader->DoShade(result, shadingIn);		
	}
	return MC_S_OK;
}

void Bevel::LoadMeshTree(BevelCache& instance, const TMCCountedPtr<FacetMesh>& amesh
								, const int32 Option, const TMCArray<TIndex3>& facetEdges, const TBBox3D& bbox) 
{
	switch(Option) {
		case OPT_OCTAL_TREE:
			instance.meshtree = new cOctTree;
			break;
		case OPT_XY_QUADRANT:{
			cOctTree* octtree;
			octtree = new cOctTree;
			octtree->mode = OCT_X + OCT_Y;
			instance.meshtree = octtree;
			}
			break;
		case OPT_XZ_QUADRANT:{
			cOctTree* octtree;
			octtree = new cOctTree;
			octtree->mode = OCT_X + OCT_Z;
			instance.meshtree = octtree;
			}
			break;
		case OPT_YZ_QUADRANT:{
			cOctTree* octtree;
			octtree = new cOctTree;
			octtree->mode = OCT_Z + OCT_Y;
			instance.meshtree = octtree;
			}
			break;
		case OPT_NONE:
			instance.meshtree = new cMeshBucket;
			break;
		case OPT_LONG_SIDE:
			instance.meshtree = new cLongSideTree;
			break;
		}
	instance.meshtree->SetBoundingBox(bbox);
	instance.meshtree->CacheMesh(amesh, instance.drawedge, facetEdges);
}


MCCOMErr Bevel::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
 	return DCGBezier::HandleEvent(message, source, data);
}

void Bevel::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
         TMCCountedCreateHelper<IExDataExchanger> result(res);
         Bevel* clone = new Bevel();
         result = (IExDataExchanger*)clone;

         clone->CopyDataFrom(this);

         clone->SetControllingUnknown(pUnkOuter);
}

void Bevel::CopyDataFrom(const Bevel* source)
{
         fBezierCurve.CopyDataFrom(&source->fBezierCurve);
		 fData = source->fData;
}

void Bevel::InitCurve()
{
	fBezierCurve.fLocations.ArrayFree();

    TMCLocationRec newLoc;
    newLoc.fSegmentBeginPos=0.0f;
    newLoc.fLocation=TVector3(0.0f, 1.0f, 0.0f);
    newLoc.SetPrevControlLocation(TVector3(-0.25f, -0.25f, 0.0f));
    newLoc.SetNextControlLocation( TVector3(0.25f, 0.25f, 0.0f) );
    fBezierCurve.SetLocation(newLoc);

    newLoc.fSegmentBeginPos=1.0f;
    newLoc.fLocation=TVector3(1.0f, 0.0f, 0.0f);
    newLoc.SetPrevControlLocation( TVector3(0.75f, 0.75f, 0.0f) );
    newLoc.SetNextControlLocation( TVector3(1.25f, 1.25f, 0.0f) );
    fBezierCurve.SetLocation(newLoc);

    fBezierCurve.SetIsLinear(true);

    fBezierCurve.ReCalcAllApprox();


}

MCCOMErr Bevel::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_Bevel))
	{
		TMCCountedGetHelper<Bevel> result(ppvObj);
		result = (Bevel*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);	
}

uint32 Bevel::AddRef()
{
	return TBasicShader::AddRef();
}

void Bevel::emptyElement(BevelCache& oldElement) {
	oldElement.cleanup();
}

void Bevel::fillElement(BevelCache& newElement, const LightingContext& lightingContext) {
	BuildCache(newElement, lightingContext);
}