/*  Anything Goos - plug-in for Carrara
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
#include "AnythingGoos.h"
#include "WireframeFacetMeshAccumulator.h"

#include "cEdgeOctTree.h"
#include "cEdgeMeshBucket.h"
#include "cEdgeLongSideTree.h"

#include "AnythingGoosDLL.h"
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

#include "I3DShInstance.h"
#include "I3DShTreeElement.h"

//#include "../../Modifiers/AnythingGrows/AnythingGrowsDeformer.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "dcgutil.h"

#if (VERSIONNUMBER >= 0x030000)
#include "IMFCurvePart.h"
#endif

#include "I3DShModifier.h"
#include "I3DShObject.h"
#include "I3DShScene.h"

#ifdef WIN32
#include <search.h>
#endif
#include <string.h>
#include <stdio.h>

//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

#include "DCGEdgeShaderTools.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_AnythingGoos(R_CLSID_AnythingGoos);
const MCGUID CLSID_AnythingGoosTerrain(R_CLSID_AnythingGoosTerrain);
const MCGUID IID_AnythingGoos(R_IID_AnythingGoos);
#else
const MCGUID CLSID_AnythingGoos={R_CLSID_AnythingGoos};
const MCGUID CLSID_AnythingGoosTerrain={R_CLSID_AnythingGoosTerrain};
const MCGUID IID_AnythingGoos={R_IID_AnythingGoos};
//const MCGUID IID_AnythingGrowsDeformer={R_IID_AnythingGrowsDeformer};
#endif

DCGSharedCache<AnythingGoosCache, ShadingIn, AnythingGoosKey> agoosCache;


AnythingGoos::AnythingGoos(): DCGSharedCacheClient<AnythingGoosCache, ShadingIn, AnythingGoosKey>(agoosCache)
{
	fData.fWireSize = .25;
	fData.iSpace = SPACE_LOCAL;
	fData.fVectorAngle = 30;
	fData.lFalloff = foRadial;
	fData.bInvert = false;
	fData.bEdgePerimeter = true;
	fData.bAdvancedEdgePerimeter = false;
	fData.bEdgeInner = true;
	fData.bEdgeOuter = true;
	fData.fMinMax.x = 0;
	fData.fMinMax.y = 1.0f;
	fData.bShadingDomains = false;
	fData.fallOffAngle = 180;

}

AnythingGoos::~AnythingGoos() {
	releaseCache();
}


void* AnythingGoos::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr AnythingGoos::ExtensionDataChanged(){
	if ((IShParameterComponent*)fData.param == NULL) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}

	if (globalStorageKey.iSpace != fData.iSpace
		|| globalStorageKey.fVectorAngle != fData.fVectorAngle
		|| globalStorageKey.bEdgePerimeter != fData.bEdgePerimeter
		|| globalStorageKey.bAdvancedEdgePerimeter != fData.bAdvancedEdgePerimeter
		|| globalStorageKey.bEdgeInner != fData.bEdgeInner
		|| globalStorageKey.bEdgeOuter != fData.bEdgeOuter
		|| globalStorageKey.bShadingDomains != fData.bShadingDomains)
	{
		releaseCache();
	}
	fFalloffAngleThreshold = cos(fData.fallOffAngle / 360 * 6.283185307179586476925286766559);

	return MC_S_OK;
	}

boolean	AnythingGoos::IsEqualTo(I3DExShader* aShader){
	return (false);
	}  

MCCOMErr AnythingGoos::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.CombineFlagsWith(ChildFlags);
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
	}

EShaderOutput AnythingGoos::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue
#if VERSIONNUMBER >= 0x060000
			| kUsesGetShaderApproxColor
#endif
			);
	}

void AnythingGoos::BuildCache(AnythingGoosCache& instance, const ShadingIn& shadingIn)
{

	TMCCountedPtr<FacetMesh>	amesh;		
	TMCArray<real32>	linemagnitude;
	TMCArray<FacetEdges> facetedges;
	TMCArray<TVector3> facenormals;
	TMCArray<boolean> drawedge;

	TMCArray<real32> edgeangle;

	TBBox3D bbox;

	{
		TMCCountedPtr<I3DShTreeElement> tree;
		TMCCountedPtr<I3DShModifier> modifier;

		FacetMeshFacetIterator facetIterator;
		WireframeFacetMeshAccumulator accu;

		//we want a mesh where matching vertices are 
		//considered a single vertex, so blank the normal
		//and UV values
		tree = shadingIn.fInstance->GetTreeElement();

		uint32 modifiercount = tree->GetModifiersCount();
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
	amesh->BuildEdgeList();
	uint32 maxedges = amesh->fEdgeList.GetElemCount();
	uint32 maxvertices = amesh->fVertices.GetElemCount();
	//if needed, run through the vertices and translate them to global space
	if (fData.iSpace == SPACE_GLOBAL)
	{
		TTransform3D L2G ;		//Transformation from Local to Global
		TMCCountedPtr<I3DShTreeElement>	tree;

		shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&tree));

		#if (VERSIONNUMBER < 0x040000)
		tree->GetGlobalTransform3D(L2G);
		#else
		L2G = tree->GetGlobalTransform3D(kCurrentFrame);
		#endif
		//transform all our points into screen space
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			LocalToGlobal(L2G, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);

		}
	}

	uint32 maxfacets = amesh->fFacets.GetElemCount();
	facenormals.SetElemCount(maxfacets);
	facetedges.SetElemCount(maxfacets);
	//calculate the normal for each facet
	for (uint32 facetindex = 0; facetindex < maxfacets; facetindex++)
	{
		facetedges[facetindex].lCurrentIndex = 0;

		TVector3		globalCoordPoint[3];
		
		globalCoordPoint[0] = amesh->fVertices[amesh->fFacets[facetindex].pt1];
		globalCoordPoint[1] = amesh->fVertices[amesh->fFacets[facetindex].pt2];
		globalCoordPoint[2] = amesh->fVertices[amesh->fFacets[facetindex].pt3];

		//we don't want to draw lines for facets facing away from us
		facenormals[facetindex] = 
			(globalCoordPoint[1] - globalCoordPoint[0])
			^ (globalCoordPoint[2] - globalCoordPoint[1]);
		facenormals[facetindex].Normalize(facenormals[facetindex]);
	}

	real32 fAngleThreshold = fData.fVectorAngle / 360 * 6.283185307179586476925286766559;

	//run through the edges and mark which should be drawn
	drawedge.SetElemCount(maxedges);
	linemagnitude.SetElemCount(maxedges);
	edgeangle.SetElemCount(maxedges);
	instance.oneoverlinemagnitude.SetElemCount(maxedges);
	instance.p2minusp1.SetElemCount(maxedges);
	instance.edgeNormal.SetElemCount(maxedges);

	//precalc the edge values
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{

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
			edgeangle[edgeindex] = 0;
			if (fData.bAdvancedEdgePerimeter)
			{
				//if the advanced check is on, makes sure this isn't a 
				//perimeter by checking against other facets in the mesh
				
				//how much of the line part of the 
				//best facet, winner is full coverage
				//or most shared length
				real32 coverage = 0;
				real32 tempcoverage = 0;
				int32 bestfacet = -1;

				//spin through the facets and see if the edge lies within a facet
				for (uint32 facetindex = 0; facetindex < maxfacets; facetindex++)
				{
					//compare this edge against all the facets except the one
					//we know it's a part of 
					if (indexx != facetindex)
					{
						#if (VERSIONNUMBER < 0x040000)
						int32 vindexx = amesh->fEdgeList[edgeindex].fVertexIndices.x;
						int32 vindexy = amesh->fEdgeList[edgeindex].fVertexIndices.y;
						#else
						int32 vindexx = amesh->fEdgeList.fVertexIndices[edgeindex].x;
						int32 vindexy = amesh->fEdgeList.fVertexIndices[edgeindex].y;
						#endif
						boolean bXInTriangle = PointInTriangle(amesh->fVertices[vindexx]
							, amesh->fVertices[amesh->fFacets[facetindex].pt1]
							, amesh->fVertices[amesh->fFacets[facetindex].pt2]
							, amesh->fVertices[amesh->fFacets[facetindex].pt3]);
						boolean bYInTriangle = PointInTriangle(amesh->fVertices[vindexy]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]);
						if (bXInTriangle && bYInTriangle)
						{
							//if both points of this edge are in this facet
							//we know we are done so jump out
							indexy = facetindex;
							facetindex = maxfacets;
						}
						else if (bXInTriangle || bYInTriangle)
						{
							//first check to see if we just share an edge
							if (bXInTriangle
								&&
								IsLineFacetEdge(amesh->fVertices[vindexx]
								, amesh->fVertices[vindexy]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]
								, tempcoverage))
							{
								if (tempcoverage > coverage)
								{
									coverage = coverage;
									bestfacet = facetindex;
								}
							}
							else if (bYInTriangle
								&&
								IsLineFacetEdge(amesh->fVertices[vindexy]
								, amesh->fVertices[vindexx]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]
								, tempcoverage))
							{
								if (tempcoverage > coverage)
								{
									coverage = coverage;
									bestfacet = facetindex;
								}
							}
							else//one point in, but not shared edge
							{
								//if one point is in the face then check to see if the
								//line is in the plane
								if (IsLineOnPlane(amesh->fVertices[vindexx]
									, amesh->fVertices[vindexy]
									, amesh->fVertices[amesh->fFacets[facetindex].pt1]
									, amesh->fVertices[amesh->fFacets[facetindex].pt2]
									, amesh->fVertices[amesh->fFacets[facetindex].pt3]))
								{
								
									//we're on the face, now look to see 
									//where we intersect each line

									if (bXInTriangle)
									{
										GetLineSegmentLengthInFacet(amesh->fVertices[vindexx]
											, amesh->fVertices[vindexy]
											, amesh->fVertices[amesh->fFacets[facetindex].pt1]
											, amesh->fVertices[amesh->fFacets[facetindex].pt2]
											, amesh->fVertices[amesh->fFacets[facetindex].pt3]
											, tempcoverage);

									}
									else
									{
										GetLineSegmentLengthInFacet(amesh->fVertices[vindexy]
											, amesh->fVertices[vindexx]
											, amesh->fVertices[amesh->fFacets[facetindex].pt1]
											, amesh->fVertices[amesh->fFacets[facetindex].pt2]
											, amesh->fVertices[amesh->fFacets[facetindex].pt3]
											, tempcoverage);
									}
									if (tempcoverage > coverage)
									{
										coverage = coverage;
										bestfacet = facetindex;
									}
								}
							}//not shared edge
						}//end one point in facet 
						else if (IsLineFacetEdgeNoInterior(amesh->fVertices[vindexy]
								, amesh->fVertices[vindexx]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]
								, tempcoverage))
						{
							if (tempcoverage > coverage)
							{
								coverage = coverage;
								bestfacet = facetindex;
							}
						}
						//not a shared edge, but see if we lie over the plane
						else if (IsLineOnPlane(amesh->fVertices[vindexx]
								, amesh->fVertices[vindexy]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]))
						{
						
							//we're on the face, now look to see 
							GetLineSegmentLengthInFacetNoInterior(amesh->fVertices[vindexx]
								, amesh->fVertices[vindexy]
								, amesh->fVertices[amesh->fFacets[facetindex].pt1]
								, amesh->fVertices[amesh->fFacets[facetindex].pt2]
								, amesh->fVertices[amesh->fFacets[facetindex].pt3]
								, tempcoverage);

							if (tempcoverage > coverage)
							{
								coverage = coverage;
								bestfacet = facetindex;
							}
						}
					}//not myself
				}//end facet loop
				if (bestfacet > -1 && indexy == -1)
				{
					indexy = bestfacet;
				}

			}//do advanced perimeter

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
				edgeangle[edgeindex] = acos(edgecos);
			}
			else
			{
				edgeangle[edgeindex] = -acos(edgecos);
			}
			instance.edgeNormal[edgeindex] = (normalx + normaly) * 0.5f;
		}
		drawedge[edgeindex] = false;
		//if the angle between the facets is too big, don't
		//draw this edge
		if (indexy == -1 && fData.bEdgePerimeter)
		{
			drawedge[edgeindex] = true;
		}
		else 
		if ((edgeangle[edgeindex] > fAngleThreshold) && fData.bEdgeOuter)
		{
			drawedge[edgeindex] = true;
		}
		else
		if ((edgeangle[edgeindex] < -fAngleThreshold) && fData.bEdgeInner)
		{
			drawedge[edgeindex] = true;
		}else if (fData.bShadingDomains && indexy != -1 
		#if (VERSIONNUMBER < 0x040000)
			&& amesh->fUVSpaceID[amesh->fEdgeList[edgeindex].fFaceIndices.x] != amesh->fUVSpaceID[amesh->fEdgeList[edgeindex].fFaceIndices.y])
		#else
			&& amesh->fUVSpaceID[amesh->fEdgeList.fFaceIndices[edgeindex].x] != amesh->fUVSpaceID[amesh->fEdgeList.fFaceIndices[edgeindex].y])
		#endif
		{
			drawedge[edgeindex] = true;
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
			drawedge[edgeindex] = false;
			linemagnitude[edgeindex] = 0;
			instance.p2minusp1[edgeindex] = TVector3(0,0,0);
		}
		else
		{
			linemagnitude[edgeindex] = sqr(p1, p2);
			instance.oneoverlinemagnitude[edgeindex] = 1.0f / linemagnitude[edgeindex];
			instance.p2minusp1[edgeindex] = p2 - p1;
		}

		//build our index of facet edges
		facetedges[indexx].lEdgeIndex[facetedges[indexx].lCurrentIndex++] = edgeindex;
		if (originalindexy > -1)
		{
			facetedges[originalindexy].lEdgeIndex[facetedges[originalindexy].lCurrentIndex++] = edgeindex;
		}

	}//end edge loop

	amesh->CalcBBox(bbox);
	LoadEdgeTree(instance, amesh, OPT_OCTAL_TREE, bbox, drawedge);
}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 AnythingGoos::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{

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


	real32 fWireSize = fData.fWireSize;

	if (shader != NULL)
	{
		real32 result = 1.0f;
		boolean fullArea;
		if ((ChildOutput & kUsesGetValue) == kUsesGetValue) 
		{
			shader->GetValue(result, fullArea, shadingIn);
		}
		else if ((ChildOutput & kUsesGetColor) == kUsesGetColor) 
		{
			#if (VERSIONNUMBER >= 0x040000)
				TMCColorRGBA tempresult;
			#else
				TMCColorRGB tempresult;
			#endif
			shader->GetColor(tempresult, fullArea, shadingIn);
			result = 0.299 * tempresult.R + 0.587 * tempresult.G + 0.114 * tempresult.B;
		}
		fWireSize *= result;
	}

	TVector3& p = shadingIn.fPointLoc;
	if (fData.iSpace == SPACE_GLOBAL)
	{
		p = shadingIn.fPoint;
	}

	real32 mindistance = FPOSINF;
	real32 minpointdistance = FPOSINF;
	uint32 lEdgeIndex = 0;

	IEdgeTree* ClosestNode = NULL;
	IEdgeTree* CurrentNode = NULL;
	cacheElement->edgetree->FindClosestNode (&ClosestNode, p, callLocalStorage);
	if (ClosestNode == NULL)
	{
		result = 0;
		return MC_S_OK;
	}
	int32 closestedge = -1;
	CheckNode (*cacheElement, ClosestNode, mindistance, p, closestedge);
	if (mindistance < callLocalStorage.closestDistance)
	{
		callLocalStorage.closestDistance = mindistance;
	}
	callLocalStorage.closestDistance = min(mindistance, callLocalStorage.closestDistance);
	while (cacheElement->edgetree->enumSiblings(ClosestNode, &CurrentNode, callLocalStorage)==MC_S_OK) 
	{
		CheckNode (*cacheElement, CurrentNode, mindistance, p, closestedge);
		if (mindistance < callLocalStorage.closestDistance)
		{
			callLocalStorage.closestDistance = mindistance;
		}
	}//end while MC_S_OK

	real32 edgeCos = 1;
	
	if (closestedge >= 0)
		edgeCos = cacheElement->edgeNormal[closestedge] * shadingIn.fNormalLoc;
	
	if (edgeCos < fFalloffAngleThreshold)
	{
		result = 0;
	}
	else if (mindistance <= fWireSize)
	{
		switch (fData.lFalloff)
		{
		case foAbsolute:
			result = 1.0f;	
			break;
		case foLinear:
			result = (fWireSize - mindistance) / fWireSize;
			break;
		case foRadial:
			result = cos(mindistance / fWireSize * HALF_PI);
			break;
		case foInvRadial:
			result = cos(HALF_PI + mindistance / fWireSize * HALF_PI) + 1.0f;
			break;
		#if (VERSIONNUMBER >= 0x030000)
		case foCustom:
			result = fBezierCurve.GetYPos(mindistance / fWireSize);
			break;
		#endif
		} 
	}
	else if(minpointdistance <= fWireSize)
	{
		switch (fData.lFalloff)
		{
		case foAbsolute:
			result = 1.0f;	
			break;
		case foLinear:
			result = (fWireSize - minpointdistance) / fWireSize;
			break;
		case foRadial:
			result = cos(minpointdistance / fWireSize * HALF_PI);
			break;
		case foInvRadial:
			result = cos(minpointdistance / fWireSize * (-HALF_PI));
			break;
		#if (VERSIONNUMBER >= 0x030000)
		case foCustom:
			result = fBezierCurve.GetYPos(minpointdistance / fWireSize);
			break;
		#endif
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
	result = fData.fMinMax.x + result * (fData.fMinMax.y - fData.fMinMax.x);
	return MC_S_OK;
}


void AnythingGoos::CheckNode(const AnythingGoosCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge)
{
	uint32 maxedges = Node->Edges.GetElemCount();

	for (uint32 i = 0; i < maxedges; i++)
	{
		uint32 edgeindex = Node->Edges[i];

		#if (VERSIONNUMBER < 0x040000)
		const TVector3& p1 = instance.mesh->fVertices[instance.mesh->fEdgeList[edgeindex].fVertexIndices.x];
		const TVector3& p2 = instance.mesh->fVertices[instance.mesh->fEdgeList[edgeindex].fVertexIndices.y];
		#else
		const TVector3& p1 = instance.mesh->fVertices[instance.mesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = instance.mesh->fVertices[instance.mesh->fEdgeList.fVertexIndices[edgeindex].y];
		#endif
		real32 u;

		u = (((point.x - p1.x) * (instance.p2minusp1[edgeindex].x)) + ((point.y - p1.y) * (instance.p2minusp1[edgeindex].y)) + ((point.z - p1.z) * (instance.p2minusp1[edgeindex].z)))
			* instance.oneoverlinemagnitude[edgeindex];
		if (u < 0)
		{
			real32 distance = sqrt(sqr(point, p1));
			if (distance < mindistance)
			{
				mindistance = distance;
				closestedge = edgeindex;
			}
		}
		else if (u > 1.0f)
		{
			real32 distance = sqrt(sqr(point, p2));
			if (distance < mindistance)
			{
				mindistance = distance;
				closestedge = edgeindex;
			}
		}
		else if (( u >= 0)&&(u <= 1.0f))
		{
			TVector3 intersection(p1 + u * (instance.p2minusp1[edgeindex]));
			real32 distance = sqrt(sqr(point, intersection));
			if (distance < mindistance)
			{
				mindistance = distance;
				closestedge = edgeindex;
			}

		}

	}
}


void AnythingGoos::LoadEdgeTree(AnythingGoosCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox, const TMCArray<boolean>& drawedge) 
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
	instance.edgetree->CacheMesh(amesh, drawedge);
}

#if (VERSIONNUMBER >= 0x030000)
MCCOMErr AnythingGoos::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
 	return DCGBezier::HandleEvent(message, source, data);

}

void AnythingGoos::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter) {
     TMCCountedCreateHelper<IExDataExchanger> result(res);
     AnythingGoos* clone = new AnythingGoos();
     result = (IExDataExchanger*)clone;

     clone->CopyDataFrom(this);

     clone->SetControllingUnknown(pUnkOuter);
}

void AnythingGoos::CopyDataFrom(const AnythingGoos* source) {
     fBezierCurve.CopyDataFrom(&source->fBezierCurve);
	 fData = source->fData;
}

void AnythingGoos::InitCurve()
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

MCCOMErr AnythingGoos::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_AnythingGoos))
	{
		TMCCountedGetHelper<AnythingGoos> result(ppvObj);
		result = (AnythingGoos*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);	
}

uint32 AnythingGoos::AddRef()
{
	return TBasicShader::AddRef();
}
#endif

void AnythingGoos::emptyElement(AnythingGoosCache& oldElement) {
	oldElement.cleanup();
}

void AnythingGoos::fillElement(AnythingGoosCache& newElement, const ShadingIn& shadingIn) {
	BuildCache(newElement, shadingIn);
}