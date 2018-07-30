/*  Wireframe Pro - plug-in for Carrara
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
#include "Wireframe.h"
#include "WireframeFacetMeshAccumulator.h"

#include "cOctTree.h"
#include "cMeshBucket.h"
#include "cLongSideTree.h"

#include "cEdgeOctTree.h"
#include "cEdgeMeshBucket.h"
#include "cEdgeLongSideTree.h"

#include "WireframeProDLL.h"
#include "I3DExVertexPrimitive.h"
#include "I3dExPrimitive.h"
#include "I3DShObject.h"
#include "IShComponent.h"
#include "IPolymesh.h"
#include "IEdge.h"
#include <stdlib.h>
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "MCCountedPtrArray.h"
#include "I3DShTreeElement.h"
#include "I3DShScene.h"

//#include <search.h>
#include <string.h>
#include <stdio.h>

#include "dcgmeshutil.h"


//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Wireframe(R_CLSID_Wireframe);
#else
const MCGUID CLSID_Wireframe={R_CLSID_Wireframe};
#endif
const real32 HALF_PI = 1.5707963267948966192313216916398f;
#define EXPIRE_INSTANCE_LIMIT 10000
#define PLANE_Z_TOLERANCE .0001

DCGSharedCache<WireframeCache, ShadingIn, WireframeKey> wireframeCache;


Wireframe::Wireframe() : DCGSharedCacheClient<WireframeCache, ShadingIn, WireframeKey>(wireframeCache) {
	fData.fWireSize = .25;
	fData.iSpace = SPACE_LOCAL;
	fData.fVectorAngle = 1;
	fData.bSmartQuads = true;
	fData.bFilterEdges = true;
	fData.lFalloff = foRadial;
	fData.bInvert = false;
	fData.bVertexEdges = true;
	}

Wireframe::~Wireframe() {
	releaseCache();
	}

void* Wireframe::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Wireframe::ExtensionDataChanged()
{
	if (globalStorageKey.iSpace != fData.iSpace
		|| globalStorageKey.fVectorAngle != fData.fVectorAngle
		|| globalStorageKey.bSmartQuads != fData.bSmartQuads
		|| globalStorageKey.bFilterEdges != fData.bFilterEdges
		|| globalStorageKey.bVertexEdges != fData.bVertexEdges)
	{
		releaseCache();
	}
	return MC_S_OK;
}

boolean	Wireframe::IsEqualTo(I3DExShader* aShader){
	return (false);
	}  

MCCOMErr Wireframe::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormalLoc = true;
	theFlags.fNeedsIsoUV = true;
	return MC_S_OK;
	}

EShaderOutput Wireframe::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue | kUsesGetShaderApproxColor);// + kUsesGetVector);
	}

void Wireframe::BuildCache(WireframeCache& instance, const ShadingIn& shadingIn)
{

	TMCCountedPtr<I3DExVertexPrimitive> originalVertexPrimitive;
	TMCCountedPtr<FacetMesh>	amesh;		
	TMCArray<real32>	linemagnitude;
	TMCArray<TIndex3> vertexToEdge;
	TMCArray<TVector3> facenormals;
	TMCArray<TIndex3> facetEdges;

	TMCCountedPtr<I3DShObject> baseobject;
	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	TMCCountedPtr<I3DShPrimitive> primitive;
	TMCCountedPtr<IShComponent> component;
	TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
	TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
	TMCCountedPtr<IRaytracablePrimitive> raytracePrimitive;
#if VERSIONNUMBER >= 0x060000 
	boolean isFigure = false;
#endif 

	TBBox3D bbox;
	boolean bDoVMEdges = fData.bVertexEdges;

	shadingIn.fInstance->Get3DObject(&baseobject);
	ThrowIfNil(baseobject);

	baseobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
	ThrowIfNil(extprimitive);

	extprimitive->GetPrimitiveComponent(&component);
	ThrowIfNil(component);

	component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
	ThrowIfNil(primcomp);

	primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&originalVertexPrimitive);
	#if VERSIONNUMBER >= 0x060000 
	if (originalVertexPrimitive) 
	{
		isFigure = originalVertexPrimitive->GetIsFigurePrimitive();
	}
	#endif


	//check to see if it's a raytraced, it uses a special case
	primcomp->QueryInterface(IID_IRaytracablePrimitive,(void**)&raytracePrimitive);
	if (raytracePrimitive)
	{
		instance.isRayTraced = true;
	}

	int32 classSig = shadingIn.fInstance->GetClassSignature();
	if (classSig != 1986880609)//vertex object
	{
		bDoVMEdges = false;
	}
	
	{
		FacetMeshFacetIterator facetIterator;
		WireframeFacetMeshAccumulator accu;

		//we want a mesh where matching vertices are 
		//considered a single vertex, so blank the normal
		//and UV values
#if VERSIONNUMBER >= 0x050000
		{
			//CWhileInCS cs(gBackgroundCS);
			amesh = shadingIn.fInstance->GetRenderingFacetMesh();
		}
#else
		shadingIn.fInstance->GetFMesh(0.00025000001f, &amesh);
#endif
		facetIterator.Initialize(amesh);			
		accu.PrepareAccumulation(amesh->fFacets.GetElemCount());
		for (facetIterator.First(); facetIterator.More(); facetIterator.Next()) 
		{
			TFacet3D aFacet = facetIterator.GetFacet();
			accu.AccumulateFacet(&aFacet);
		}
		accu.MakeFacetMesh(&amesh);
	}
	instance.mesh = amesh;
	TMCArray<TIndex2> vmedges;
	long numvmedges = 0;

	if (bDoVMEdges)
	{
		TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
		TMCCountedPtr<I3DShPrimitive> primitive;
		TMCCountedPtr<IShComponent> component;
		TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<FacetMesh> vmesh;		
		TMCCountedPtr<I3DShObject> newobject;
		TMCCountedPtrArray<IPolymesh> polymeshes;

		shadingIn.fInstance->Get3DObject(&baseobject);
		ThrowIfNil(baseobject);

		{
			//CWhileInCS cs(gBackgroundCS);
			baseobject->Clone(&newobject, kNoAnim);
		}
		ThrowIfNil(newobject);

		newobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
		ThrowIfNil(extprimitive);

		newobject->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
		ThrowIfNil(primitive);

		extprimitive->GetPrimitiveComponent(&component);
		ThrowIfNil(component);

		component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
		ThrowIfNil(primcomp);

		primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);
		ThrowIfNil(vertexPrimitive);

#if VERSIONNUMBER >= 0x060000
		vertexPrimitive->Hydrate();
#endif
#if VERSIONNUMBER >= 0x060000 
		if (isFigure) 
		{
			fixBadFigureClone(vertexPrimitive, originalVertexPrimitive);
		}
#endif
		removeBonesAndMorphs(vertexPrimitive);

#if VERSIONNUMBER >= 0x050000
		{
			//CWhileInCS cs(gBackgroundCS);
			primitive->GetRenderingFacetMesh(&vmesh);
		}
#else
		primitive->GetFMesh(0.00025000001f, &vmesh);
#endif
		ThrowIfNil(vmesh);

		FacetMeshFacetIterator facetIterator;
		WireframeFacetMeshAccumulator accu;

		//we want a mesh where matching vertices are 
		//considered a single vertex, so blank the normal
		//and UV values
		facetIterator.Initialize(vmesh);			
		accu.PrepareAccumulation(vmesh->fFacets.GetElemCount());
		for (facetIterator.First(); facetIterator.More(); facetIterator.Next()) 
		{
			TFacet3D aFacet = facetIterator.GetFacet();
			accu.AccumulateFacet(&aFacet);
		}
		accu.MakeFacetMesh(&vmesh);

		//count the number of edges we have
		long numpolymesh = vertexPrimitive->GetNbPolymeshes();
		polymeshes.SetElemCount(numpolymesh);
		for (long polymeshindex = 0; polymeshindex < numpolymesh;polymeshindex++)
		{
			TMCCountedPtr<IPolymesh> polymesh;
			vertexPrimitive->GetPolymesh(&polymesh, polymeshindex);
			if (polymesh != NULL)
			{
				//sds on for this polymesh
#if VERSIONNUMBER >= 0x050000
				polymesh->SetSubdivisionLevel(polymesh->GetSubdivisionLevel(true), false);
				if ((polymesh->GetSubdivisionLevel(false) > 0)&&(polymesh->GetSubdivisionType() != eSubdivisionNone))
#else
				if ((polymesh->GetSubdivisionLevel() > 0)&&(polymesh->GetSubdivisionType() != eSubdivisionNone))
#endif
				{
#if VERSIONNUMBER >= 0x050000
					//CWhileInCS cs(gBackgroundCS);
#endif
					TMCCountedPtr<IPolymesh> temppolymesh;
					polymesh->ConvertSubdivisionToPolygons(&temppolymesh);
					polymesh = temppolymesh;
				}
				numvmedges += polymesh->GetNbEdges();
				polymeshes.SetElem(polymeshindex, polymesh);
			}
		}
		
		vmedges.SetElemCount(numvmedges);
		uint32 currentEdge = 0;

		for (long polymeshindex = 0; polymeshindex < numpolymesh;polymeshindex++)
		{
			long numedges = polymeshes[polymeshindex]->GetNbEdges();
			for (long edgeindex = 0; edgeindex < numedges; edgeindex++)
			{
				TIndex2& vmwatch = vmedges[currentEdge];
				IEdge* edge = NULL;
				TVector3 temp;
				edge = polymeshes[polymeshindex]->GetEdgeNoAddRef(edgeindex);

				I3DExVertex* vertex = NULL;
				vertex = edge->GetVertex(0);
				vertex->GetPosition(temp);
				vmedges[currentEdge].x = accu.GetVertexIndex(temp);
				vertex = edge->GetVertex(1);
				vertex->GetPosition(temp);
				vmedges[currentEdge].y = accu.GetVertexIndex(temp);
				currentEdge++;
			}
		}//end polymesh loop

		//run through the edges and make sure the lowest vertexindex is in the first position
		//this lets us do a simplier compare later
		for (uint32 vmedgeindex = 0; vmedgeindex < numvmedges; vmedgeindex++)
		{
			if(vmedges[vmedgeindex].x > vmedges[vmedgeindex].y)
			{
				uint32 temp = vmedges[vmedgeindex].x;
				vmedges[vmedgeindex].x = vmedges[vmedgeindex].y;
				vmedges[vmedgeindex].y = temp;
			}
		}
		qsort(vmedges.Pointer(0), vmedges.GetElemCount(), sizeof(TIndex2), TIndex2Compare);

	}//end is do vertex edges

	amesh->BuildEdgeList();

	uint32 maxedges = amesh->fEdgeList.GetElemCount();
	vertexToEdge.SetElemCount(maxedges);

	//run through the edges and make sure the lowest vertexindex is in the first position
	//this lets us do a simplier compare later
	//fill the vertexToEdge array
	for (uint32 edgeIndex = 0; edgeIndex < maxedges; edgeIndex++)
	{
		if(amesh->fEdgeList.fVertexIndices[edgeIndex].x > amesh->fEdgeList.fVertexIndices[edgeIndex].y)
		{
			uint32 temp = amesh->fEdgeList.fVertexIndices[edgeIndex].x;
			amesh->fEdgeList.fVertexIndices[edgeIndex].x = amesh->fEdgeList.fVertexIndices[edgeIndex].y;
			amesh->fEdgeList.fVertexIndices[edgeIndex].y = temp;
		}
		vertexToEdge[edgeIndex].z = edgeIndex;
		vertexToEdge[edgeIndex].x = amesh->fEdgeList.fVertexIndices[edgeIndex].x;
		vertexToEdge[edgeIndex].y = amesh->fEdgeList.fVertexIndices[edgeIndex].y;

	}
	qsort(vertexToEdge.Pointer(0), vertexToEdge.GetElemCount(), sizeof(TIndex3), TIndex3CompareFirstTwo);

	uint32 maxvertices = amesh->fVertices.GetElemCount();
	if (!instance.isRayTraced)
	{
		instance.drawpoint.SetElemCount(maxvertices);
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			instance.drawpoint[vertexindex] = false;
		}
	}
	//if needed, run through the vertices and translate them to global space
	if (fData.iSpace == SPACE_GLOBAL)
	{
		TTransform3D L2G ;		//Transformation from Local to Global
		TMCCountedPtr<I3DShTreeElement>	tree;

		shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&tree));

		L2G = tree->GetGlobalTransform3D(kCurrentFrame);
		//transform all our points into screen space
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			amesh->fVertices[vertexindex] = L2G.TransformPoint(amesh->fVertices[vertexindex]);
		}
	}

	uint32 maxfacets = amesh->fFacets.GetElemCount();
	facenormals.SetElemCount(maxfacets);
	facetEdges.SetElemCount(maxfacets);

	//calculate the normal for each facet
	//and set the values for the facetEdges array
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

	real32 fAngleThreshold = cos(fData.fVectorAngle / 360 * 6.283185307179586476925286766559 );

	//run through the edges and mark which should be drawn
	instance.drawedge.SetElemCount(maxedges);
	linemagnitude.SetElemCount(maxedges);
	if (instance.isRayTraced)
	{
		instance.oneoverlinemagnitude.SetElemCount(maxedges);
		instance.p2minusp1.SetElemCount(maxedges);
	}

	//precalc the edge values
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{

		int32 indexx = amesh->fEdgeList.fFaceIndices[edgeindex].x;
		int32 indexy = amesh->fEdgeList.fFaceIndices[edgeindex].y;

		real32 edgecos; 
		if (indexy != -1)
		{
			edgecos = facenormals[indexx] * facenormals[indexy];
		}

		instance.drawedge[edgeindex] = false;
		if (bDoVMEdges)
		{
			//check to see if it's in our list of edges to draw
			if (bsearch(&amesh->fEdgeList.fVertexIndices[edgeindex], vmedges.Pointer(0)
					, vmedges.GetElemCount(), sizeof(TIndex2), TIndex2Compare))
			{
					instance.drawedge[edgeindex] = true;
			}
		}
		else //!bDoVMEdges
		{
			//if the angle between the facets is too big, don't
			//draw this edge
			if (indexy == -1)
			{
				instance.drawedge[edgeindex] = true;
			}
			else if (fData.bFilterEdges == false)
			{
				instance.drawedge[edgeindex] = true;
			}
			else if (edgecos < fAngleThreshold)
			{
				instance.drawedge[edgeindex] = true;
			}

		}

		const TVector3& p1 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].y];
		//if the end points are the same, don't draw this edge
		if (p1 == p2)
		{
			instance.drawedge[edgeindex] = false;
			linemagnitude[edgeindex] = 0;
			if (instance.isRayTraced)
			{
				instance.p2minusp1[edgeindex] = TVector3(0,0,0);
			}		
		}
		else
		{
			linemagnitude[edgeindex] = sqr(p1, p2);
			if (instance.isRayTraced)
			{
				instance.oneoverlinemagnitude[edgeindex] = 1.0f / linemagnitude[edgeindex];
				instance.p2minusp1[edgeindex] = p2 - p1;
			}
		}
	}

	if ((fData.bSmartQuads)&&(!bDoVMEdges))
	{
		//if this edge is the largest of a pair of facets then don't draw it
		for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
		{
			int32 indexx = amesh->fEdgeList.fFaceIndices[edgeindex].x;
			int32 indexy = amesh->fEdgeList.fFaceIndices[edgeindex].y;

			//we have an edge on two facets
			if (indexy > -1)
			{
				real32 fMax = 0;
				for (uint32 edgeIndex = 0; edgeIndex < 3; edgeIndex++)
				{
					uint32 edgeIndexX = facetEdges[indexx][edgeIndex]
						, edgeIndexY = facetEdges[indexy][edgeIndex];
					if ((linemagnitude[edgeIndexX] > fMax)&&(edgeIndexX!=edgeindex))
						fMax = linemagnitude[edgeIndexX];
					if ((linemagnitude[edgeIndexY] > fMax)&&(edgeIndexY!=edgeindex))
						fMax = linemagnitude[edgeIndexY];
				}
				if (fMax  * 1.01f < linemagnitude[edgeindex])
					instance.drawedge[edgeindex] = false;
			}
			

		}
	}
	if (!instance.isRayTraced)
	{
		for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
		{
			if (instance.drawedge[edgeindex])
			{
				uint32 vertexindexx = amesh->fEdgeList.fVertexIndices[edgeindex].x;
				uint32 vertexindexy = amesh->fEdgeList.fVertexIndices[edgeindex].y;

				instance.drawpoint[vertexindexx] = true;
				instance.drawpoint[vertexindexy] = true;

			}
		}
	}

	cOctTree* octtree;
	int32 bestoption;
	uint32 bestrank;	
	uint32 currentrank;


	amesh->CalcBBox(bbox);
	if (instance.isRayTraced)
	{
		bestoption = OPT_OCTAL_TREE;
		LoadEdgeTree(instance, amesh, bestoption, bbox);
	}
	else
	{
		
		if (maxedges <= 100)
		{
			bestoption = OPT_LONG_SIDE;
			LoadEdgeTree(instance, amesh, bestoption, bbox);
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
		//bestoption = OPT_NONE;
		LoadMeshTree(instance, amesh, bestoption, facetEdges, bbox);
	}
}

real Wireframe::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn, const boolean bDoVector, TVector3& normal)
{
	normal.SetValues(0,0,0);

	if (shadingIn.fInstance == NULL)
	{
		result = 1.0f;
		return MC_S_OK;
	}
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	real currentTime;

	shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
	ThrowIfNil(tree);
	tree->GetScene(&scene);
	ThrowIfNil(scene);

	scene->GetTime(&currentTime);

	//find the instance in our cache
	if (cacheElement == NULL || globalStorageKey.instance != shadingIn.fInstance || globalStorageKey.currentTime != currentTime)
	{
		if (globalStorageKey.currentTime != currentTime)
		{
			releaseCache();
		}
		globalStorageKey.fill(shadingIn.fInstance, fData, currentTime);
		getCache(shadingIn);
	}

	TVector3& p = shadingIn.fPointLoc;
	if (fData.iSpace == SPACE_GLOBAL)
	{
		p = shadingIn.fPoint;
	}

	real32 mindistance = FPOSINF;
	real32 minpointdistance = FPOSINF;

	if (cacheElement->isRayTraced)
	{
		IEdgeTree* ClosestNode = NULL;
		IEdgeTree* CurrentNode = NULL;
		cacheElement->edgetree->FindClosestNode (&ClosestNode, p, edgeLocalStorage);
		if (ClosestNode == NULL)
		{
			result = 0;
			return MC_S_OK;
		}
		int32 closestedge = -1;
		CheckNode (*cacheElement, ClosestNode, mindistance, p, closestedge);
		edgeLocalStorage.closestDistance = min(mindistance, edgeLocalStorage.closestDistance);
		while (cacheElement->edgetree->enumSiblings(ClosestNode, &CurrentNode, edgeLocalStorage)==MC_S_OK) 
		{
			CheckNode (*cacheElement, CurrentNode, mindistance, p, closestedge);
			edgeLocalStorage.closestDistance = min(mindistance, edgeLocalStorage.closestDistance);
		}//end while MC_S_OK
	}
	else
	{
		//LightingContext* lightingContext = NULL;//(LightingContext*)gCurrentLightingContext->GetValue();

		TVector3 flatpoint;
		IMeshTree* ClosestNode = NULL;
		IMeshTree* CurrentNode = NULL;
		MeshTreeItem* ItemFound = NULL;
		MeshTreeItem* PointItemFound = NULL;
		uint32 lEdgeFound = 0;
		uint32 lPointFound = 0;

		cacheElement->meshtree->FindClosestNode (&ClosestNode, p, meshLocalStorage);
		if (ClosestNode == NULL)
		{
			result = 0;
			return MC_S_OK;
		}
		real32 fAlongEdge;
		int32 closestedge = -1;
		CheckNode(*cacheElement, ClosestNode, mindistance, p, closestedge, fAlongEdge);
		meshLocalStorage.closestDistance = min(mindistance, meshLocalStorage.closestDistance);
		while (cacheElement->meshtree->enumSiblings(ClosestNode, &CurrentNode, meshLocalStorage)==MC_S_OK) 
		{
			CheckNode (*cacheElement, CurrentNode, mindistance, p, closestedge, fAlongEdge);
			meshLocalStorage.closestDistance = min(mindistance, meshLocalStorage.closestDistance);
		}//end while MC_S_OK
	}

	if (mindistance <= fData.fWireSize)
	{
		switch (fData.lFalloff)
		{
		case foAbsolute:
			result = 1.0f;	
			break;
		case foLinear:
			result = (fData.fWireSize - mindistance) / fData.fWireSize;
			break;
		case foRadial:
			result = cos(mindistance / fData.fWireSize * HALF_PI);
			break;
		}

	}
	else
	{
		result = 0;
	}
	if (fData.bInvert)
	{
		result = 1.0f - result;

	}
	return MC_S_OK;
}

real Wireframe::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	TVector3 temp;
	return GetValue(result, fullArea, shadingIn, false, temp);

}

real MCCOMAPI Wireframe::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	real32 innerResult;
	boolean fullArea;

	GetValue(innerResult, fullArea, shadingIn, true, result);

	return MC_S_OK;
}

void Wireframe::CheckNode(const WireframeCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge)
{
	uint32 maxedges = Node->Edges.GetElemCount();

	for (uint32 i = 0; i < maxedges; i++)
	{
		uint32 edgeindex = Node->Edges[i];

		const TVector3& p1 = instance.mesh->fVertices[instance.mesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = instance.mesh->fVertices[instance.mesh->fEdgeList.fVertexIndices[edgeindex].y];

		real32 u;

		u = (((point.x - p1.x) * (instance.p2minusp1[edgeindex].x)) + ((point.y - p1.y) * (instance.p2minusp1[edgeindex].y)) + ((point.z - p1.z) * (instance.p2minusp1[edgeindex].z)))
			* instance.oneoverlinemagnitude[edgeindex];
		if (( u >= 0)&&(u <= 1.0f))
		{
			TVector3 intersection(p1 + u * (instance.p2minusp1[edgeindex]));
			real32 distance = sqrt(sqr(point, intersection));
			if (distance < mindistance)
			{
				mindistance = distance;
			}

		}

	}
}
void Wireframe::CheckNode(const WireframeCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point	, int32& lEdgeFound, real32& fAlongEdge)
{
	MeshTreeItem* Item;
	long FacetsInNode = Node->FacetCount;

	for(long j = 0; j<FacetsInNode;j++) 
	{
		Item = &Node->Facets[j];
		CheckMeshTreeItem(instance, Node, mindistance
						  , point
						  , lEdgeFound, fAlongEdge, Item);
	}//end for
}

void Wireframe::CheckMeshTreeItem(const WireframeCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point
				, int32& lEdgeFound, real32& fAlongEdge, MeshTreeItem* Item)
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
	if (z <= PLANE_Z_TOLERANCE)
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
				}
			}
			else
			{
				fTemp = planept.x;// * planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[0];
					fAlongEdge = planept.y / Item->flatpt2y[0];
					if (Item->lPoints[0] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[0]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
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
				}
			}
			else
			{
				fTemp = planept.x;// * planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[1];
					fAlongEdge = planept.y / Item->flatpt2y[1];
					if (Item->lPoints[1] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[1]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
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
				}
			}
			else
			{
				fTemp = planept.x;// * planept.x;
				if (fTemp < mindistance) 
				{
					mindistance = fTemp;
					lEdgeFound = Item->lEdges[2];
					fAlongEdge = planept.y / Item->flatpt2y[2];
					if (Item->lPoints[2] != instance.mesh->fEdgeList.fVertexIndices[Item->lEdges[2]].x)
					{
						fAlongEdge = 1.0f - fAlongEdge;
					}
				}

			}
		}
	}//it's z more or less = 0, on our plane
}

void Wireframe::LoadEdgeTree(WireframeCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox) 
{
	switch(Option) {
		case OPT_OCTAL_TREE:
			instance.edgetree = new cEdgeOctTree;
			break;
		case OPT_XY_QUADRANT:{
			cEdgeOctTree* octtree;
			octtree = new cEdgeOctTree;
			octtree->mode = OCT_X + OCT_Y;
			instance.edgetree = octtree;
			}
			break;
		case OPT_XZ_QUADRANT:{
			cEdgeOctTree* octtree;
			octtree = new cEdgeOctTree;
			octtree->mode = OCT_X + OCT_Z;
			instance.edgetree = octtree;
			}
			break;
		case OPT_YZ_QUADRANT:{
			cEdgeOctTree* octtree;
			octtree = new cEdgeOctTree;
			octtree->mode = OCT_Z + OCT_Y;
			instance.edgetree = octtree;
			}
			break;
		case OPT_NONE:
			instance.edgetree = new cEdgeMeshBucket;
			break;
		case OPT_LONG_SIDE:
			instance.edgetree = new cEdgeLongSideTree;
			break;
		}
	instance.edgetree->SetBoundingBox(bbox);
	instance.edgetree->CacheMesh(amesh, instance.drawedge);
}

void Wireframe::LoadMeshTree(WireframeCache& instance, const TMCCountedPtr<FacetMesh>& amesh
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

void Wireframe::emptyElement(WireframeCache& oldElement) {
	oldElement.cleanup();
}

void Wireframe::fillElement(WireframeCache& newElement, const ShadingIn& shadingIn) {
	BuildCache(newElement, shadingIn);
}