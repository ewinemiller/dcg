/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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
#include "Cutout.h"

#include "cEdgeOctTree.h"
#include "cEdgeMeshBucket.h"
#include "cEdgeLongSideTree.h"

#include "TerrainToolsDLL.h"
#include "I3DShInstance.h"
#include "I3DShTreeElement.h"

#include "IMFPart.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "IMFCurvePart.h"
#include <algorithm>


//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

const MCGUID CLSID_Cutout(R_CLSID_Cutout);

DCGSharedCache<EdgeCache2D, ShadingIn, CutoutKey> cutoutCache;


int ShapeFileEdgeCompare(const void *elem1, const void *elem2)
{
	//sort by y on first point
	real32 temp = (static_cast<const ShapeFileEdge*>(elem1))->p1.y - (static_cast<const ShapeFileEdge*>(elem2))->p1.y;

	if (temp < 0)
		return -1;
	else if (temp > 0)
		return 1;
	else
	{
		return 0;
	}
}

inline boolean lineSegmentIntersection(const TVector2& pp1,const TVector2& pp2,const TVector2& pp3,const TVector2& pp4)
{
	TVector2d p1 = TVector2d(pp1.x, pp1.y);
	TVector2d p2 = TVector2d(pp2.x, pp2.y);
	TVector2d p3 = TVector2d(pp3.x, pp3.y);
	TVector2d p4 = TVector2d(pp4.x, pp4.y);
	
	real64 denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
	if (denom == 0)
		return false;
	real64 ua = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
	ua /= denom;
	if (ua < 0 || ua > 1.0f)
		return false;
	real64 ub = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);
	ub /= denom;
	if (ub < 0 || ub > 1.0f)
		return false;
	return true;
}


uint32 EdgeNode::tallyIntersect(const TVector2& p1, const TVector2& p2)
{
	uint32 intersectTally = 0;

	if (p1.y < midPoint.y || p2.y < midPoint.y)
	{
		if (lowYlowX != NULL && (p1.x < midPoint.x || p2.x < midPoint.x))
		{
			intersectTally += lowYlowX->tallyIntersect(p1, p2);
		}
		if (lowYhighX != NULL && (p1.x > midPoint.x || p2.x > midPoint.x))
		{
			intersectTally += lowYhighX->tallyIntersect(p1, p2);
		}
	}
	if (p1.y > midPoint.y || p2.y > midPoint.y)
	{
		if (highYlowX != NULL && (p1.x < midPoint.x || p2.x < midPoint.x))
		{
			intersectTally += highYlowX->tallyIntersect(p1, p2);
		}
		if (highYhighX != NULL && (p1.x > midPoint.x || p2.x > midPoint.x))
		{
			intersectTally += highYhighX->tallyIntersect(p1, p2);
		} 
	}

	uint32 maxEdges = edges.GetElemCount();
	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		ShapeFileEdge& currentEdge = edges[edgeIndex];
		
		//if (currentEdge.p2.y >= p1.y && currentEdge.p1.y <= p1.y)
		//{
			if (lineSegmentIntersection(p1, p2, currentEdge.p1, currentEdge.p2))
			{
				intersectTally++;
			}
		//}
		//else
		//if (p1.y < currentEdge.p1.y)
		//{
		//	edgeIndex = maxEdges;
		//}
	}

	return intersectTally;

}

Cutout::Cutout():DCGSharedCacheClient<EdgeCache2D, ShadingIn, CutoutKey>(cutoutCache)
{
	fData.bInvert = false;
	fData.lEffect = seExclude;
}

Cutout::~Cutout()
{
	releaseCache();
}


void* Cutout::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Cutout::ExtensionDataChanged()
{
	if (globalStorageKey.lEffect != fData.lEffect
		|| globalStorageKey.sList != fData.sList)
	{
		releaseCache();
	}
	return MC_S_OK;
}

boolean	Cutout::IsEqualTo(I3DExShader* aShader){
	return (false);
	}

MCCOMErr Cutout::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
	}

EShaderOutput Cutout::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue | kUsesGetVector | kUsesGetShaderApproxColor);
	}

void Cutout::BuildCache(EdgeCache2D& instance, const ShadingIn& shadingIn)
{
	TMCDynamicString localList(fData.sList);
	localList.Append("\r", 1);
	
	TMCCountedPtr<I3DShTreeElement> tree;
	tree = shadingIn.fInstance->GetTreeElement();
	TMCCountedPtr<FacetMesh> objectmesh;
	TMCCountedPtr<FacetMesh> amesh;
	TMCCountedPtr<FacetMesh> newmesh;
	TMCCountedPtr<I3DShScene> scene;
	TMCArray<boolean> drawedge;
	TMCCountedPtr<I3DShTreeElement> iptree;
	TMCCountedPtr<I3DShInstance> ipinstance;
	TTransform3D L2G;		//Transformation from Local to Global
	TTransform3D inverseL2G;		//Transformation from Local to Global
	TTransform3D ipL2G;		//Transformation from Local to Global

	TBBox3D bbox;

	FacetMesh::Create(&newmesh);
	uint32 lNewEdgeIndex = 0, lNewVertexIndex = 0;
	uint32 lNewVertexCount = 0, lNewEdgeCount = 0;

    tree->GetScene(&scene);

	L2G = tree->GetGlobalTransform3D(kCurrentFrame);

	uint32 instancecount = scene->GetInstanceListCount();

	for (uint32 currentinstance = 0; currentinstance < instancecount; currentinstance++)
	{
		scene->GetInstanceByIndex(&ipinstance, currentinstance);

		if (ipinstance == shadingIn.fInstance)
		{
			continue;
		}

		boolean bIncludeTerrain = (fData.lEffect == seExclude);
		ipinstance->QueryInterface(IID_I3DShTreeElement, (void**)&iptree);
		ThrowIfNil(iptree);

		TMCString255 name;
		iptree->GetName(name);
		TMCDynamicString localName(name);
		localName.Append("\r",1);
		uint32 index = localList.SearchStr(localName);
		//is the light in the list?
		if (index != kUINT32_MAX)
		{
			bIncludeTerrain = !bIncludeTerrain;
		}

		if (bIncludeTerrain == false)
		{
			continue;
		}

		ipL2G = iptree->GetGlobalTransform3D(kCurrentFrame);

		objectmesh = ipinstance->GetFacetMesh(0.00025000001f);

		if (objectmesh != NULL)
		{
			objectmesh->Clone(&amesh);
		}
		else
		{
			FacetMesh::Create(&amesh);
		}
		

		uint32 maxvertices = amesh->fVertices.GetElemCount();
		//run through the vertices and translate them to global space
		if (iptree != NULL)
		{
			for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
			{
				LocalToGlobal(ipL2G, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);
			}
		}

		//now run through the vertices and translate them into the infinite plane's local space
		TTransform3D L2GInv;
		InvertTransform(L2G, L2GInv);
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			GlobalToLocalInv(L2GInv, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);
		}

		doPlaneObjectIntersect(amesh, newmesh, lNewEdgeIndex, lNewVertexIndex
			, lNewVertexCount, lNewEdgeCount);
	}//end instance loop

	uint32 maxedges = newmesh->fEdgeList.GetElemCount(), edgeCount = 0;
	drawedge.SetElemCount(maxedges);

	//precalc the edge values
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{

		const TVector3& p1 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[edgeindex].y];
		//if the end points are the same, don't draw this edge
		if (p1 == p2)
		{
			drawedge[edgeindex] = false;
		}
		else
		{
			drawedge[edgeindex] = true;
			edgeCount++;
		}

	}//end edge loop

	TMCArray<ShapeFileEdge> edges;

	edges.SetElemCount(edgeCount);
	uint32 shapeEdgeIndex = 0;

	for (uint32 meshEdgeIndex = 0; meshEdgeIndex < maxedges; meshEdgeIndex++)
	{
		if (drawedge[meshEdgeIndex] == true) 
		{
			const TVector3& meshp1 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[meshEdgeIndex].x];
			const TVector3& meshp2 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[meshEdgeIndex].y];
			TVector2& p1 = edges[shapeEdgeIndex].p1;
			TVector2& p2 = edges[shapeEdgeIndex].p2;
			if (meshp1.y < meshp2.y)
			{
				p1.SetValues(meshp1.x, meshp1.y);
				p2.SetValues(meshp2.x, meshp2.y);
			}
			else
			{
				p2.SetValues(meshp1.x, meshp1.y);
				p1.SetValues(meshp2.x, meshp2.y);
			}
			bbox.AddPoint(TVector3(p1.x, p1.y, 0));
			bbox.AddPoint(TVector3(p2.x, p2.y, 0));
			shapeEdgeIndex++;
		}

	}//end edge loop
	instance.range.SetMax(TVector2(bbox.fMax.x, bbox.fMax.y));
	instance.range.SetMin(TVector2(bbox.fMin.x, bbox.fMin.y));
	instance.range.GetCenter(instance.targetPoint);
	//instance.targetPoint.SetValues(0,0);
	//qsort(edges.Pointer(0), edges.GetElemCount(), sizeof(ShapeFileEdge), ShapeFileEdgeCompare);
	
	fillEdgeTree(instance, edges);

}

real Cutout::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
	TTransform3D currentTransform = tree->GetGlobalTransform3D(kCurrentFrame);

	//find the instance in our cache
	if (globalStorageKey.currentTime != currentTime
		|| !globalStorageKey.ipL2G.IsEqual(currentTransform, 0))
	{
		releaseCache();
		globalStorageKey.fill(shadingIn.fInstance, fData, currentTime, currentTransform);
		getCache(shadingIn);
	}
	else if (globalStorageKey.instance != shadingIn.fInstance || cacheElement == NULL)
	{
		globalStorageKey.fill(shadingIn.fInstance, fData, currentTime, currentTransform);
		getCache(shadingIn);
	}

	uint32 intersections = 1;
	TVector2 samplepoint;
	TVector2 exteriorpoint;
	
	samplepoint.SetValues(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y);
	//if (samplepoint.x >= cacheElement->shapefile.range.fMin.x && samplepoint.x <= cacheElement->shapefile.range.fMax.x
	//	&& samplepoint.y >= cacheElement->shapefile.range.fMin.y && samplepoint.y <= cacheElement->shapefile.range.fMax.y)
	//{
	//	real32 mindistance = samplepoint.x - cacheElement->shapefile.range.fMin.x;
	//	//exteriorpoint.SetValues(cacheElement->shapefile.range.fMin.x, samplepoint.y);
	//	if (mindistance > cacheElement->shapefile.range.fMax.x - samplepoint.x)
	//	{
	//		exteriorpoint.x = cacheElement->shapefile.range.fMax.x;
	//	}

		intersections += cacheElement->edgeTree.tallyIntersect(samplepoint, cacheElement->targetPoint);
	//}

	result = intersections % 2;
	if (fData.bInvert)
		result = 1.0f - result;

	return MC_S_OK;
}
real Cutout::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	boolean fullArea = false;

    ShadingIn shadingInToUse = shadingIn;

	real32 globalstep = GetVectorSampleStep3D(shadingInToUse, SPACE_GLOBAL);

	real32 libresult[6];

	TVector3 AVec, BVec, CVec;

	shadingInToUse.fPointLocx.Normalize(BVec);
	shadingInToUse.fPointLocy.Normalize(AVec);

	BVec = -BVec;
	CVec = AVec ^ BVec;

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x - globalstep
									, shadingIn.fPoint.y
									, shadingIn.fPoint.z);
	GetValue(libresult[0], fullArea, shadingInToUse);

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x 
									, shadingIn.fPoint.y - globalstep
									, shadingIn.fPoint.z);
	GetValue(libresult[1], fullArea, shadingInToUse);

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x 
									, shadingIn.fPoint.y
									, shadingIn.fPoint.z - globalstep);
	GetValue(libresult[2], fullArea, shadingInToUse);

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x + globalstep
									, shadingIn.fPoint.y
									, shadingIn.fPoint.z);
	GetValue(libresult[3], fullArea, shadingInToUse);

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x 
									, shadingIn.fPoint.y + globalstep
									, shadingIn.fPoint.z);
	GetValue(libresult[4], fullArea, shadingInToUse);

	shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x 
									, shadingIn.fPoint.y
									, shadingIn.fPoint.z + globalstep);
	GetValue(libresult[5], fullArea, shadingInToUse);
	
	AVec = AVec * (libresult[3] - libresult[0]);
	BVec = BVec * (libresult[4] - libresult[1]);
	CVec = CVec * (libresult[5] - libresult[2]);

	result = AVec + BVec + CVec;
	return MC_S_OK;
}

void Cutout::fillEdgeTree(EdgeCache2D& instance, const TMCArray<ShapeFileEdge>& edges)
{
	uint32 maxEdges = edges.GetElemCount();

	instance.edgeTree.clearNodes();

	instance.edgeTree.setRange(instance.range.fMin, instance.range.fMax);
	
	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		const ShapeFileEdge& currentEdge = edges[edgeIndex];
		instance.edgeTree.countEdge(currentEdge.p1, currentEdge.p2, 4);
	}

	instance.edgeTree.allocNodes();

	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		const ShapeFileEdge& currentEdge = edges[edgeIndex];
		instance.edgeTree.addEdge(currentEdge.p1, currentEdge.p2, 4);
	}

}

MCCOMErr Cutout::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	if ((sourceID == 'REFR')&&(message == EMFPartMessage::kMsg_PartValueChanged)) 
	{
		releaseCache();
	}

	return MC_S_OK;
}


void Cutout::emptyElement(EdgeCache2D& oldElement) {
	oldElement.cleanup();
}

void Cutout::fillElement(EdgeCache2D& newElement, const ShadingIn& shadingIn) {
	BuildCache(newElement, shadingIn);
}
