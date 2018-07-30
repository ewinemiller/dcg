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
#include "Surf.h"

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


//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Surf(R_CLSID_Surf);
const MCGUID IID_Surf(R_IID_Surf);
#else
const MCGUID CLSID_Surf={R_CLSID_Surf};
const MCGUID IID_Surf={R_IID_Surf};
#endif
const real32 HALF_PI = 1.5707963267948966192313216916398f;
#define EXPIRE_INSTANCE_LIMIT 10000

DCGSharedCache<TerrainToolsCache, ShadingIn, SurfKey> surfCache;

inline void FillPoint(const real32& fFarAlong, const TVector3& pt1, const TVector3& pt2, TVector3& result)
{
	//do the 3D point
	result = pt1 + fFarAlong * (pt2 - pt1);
};

Surf::Surf():DCGSharedCacheClient<TerrainToolsCache, ShadingIn, SurfKey>(surfCache)
{
	fData.fWireSize = 2.0f;
	fData.lFalloff = foRadial;
	fData.bInvert = false;
	fData.fMinMax.x = 0;
	fData.fMinMax.y = 1.0f;
	fData.lEffect = seExclude;
	fData.fWindDirection = 0;
	fData.fWindEffect = 0;
	fData.fSurfLength = 1.0f;
}

Surf::~Surf()
{
	releaseCache();
}


void* Surf::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Surf::ExtensionDataChanged()
{
	if (!fData.param) 
	{
		shader = NULL;
	}
	else 
	{
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
	}
	
	if (globalStorageKey.lEffect != fData.lEffect
		|| globalStorageKey.sList != fData.sList)
	{
		releaseCache();
	}

	return MC_S_OK;
	
}

boolean	Surf::IsEqualTo(I3DExShader* aShader){
	return (false);
	}

MCCOMErr Surf::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.CombineFlagsWith(ChildFlags);
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
	}

EShaderOutput Surf::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue | kUsesGetVector | kUsesGetShaderApproxColor);
	}

void Surf::BuildCache(TerrainToolsCache& instance, const ShadingIn& shadingIn)
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
	instance.mesh = newmesh;
	uint32 lNewEdgeIndex = 0, lNewVertexIndex = 0;
	uint32 lNewVertexCount = 0, lNewEdgeCount = 0;
	uint32 maxvertices;

    tree->GetScene(&scene);

	L2G = tree->GetGlobalTransform3D(kCurrentFrame);

	uint32 instancecount = scene->GetInstanceListCount();

	for (uint32 currentinstance = 0; currentinstance < instancecount; currentinstance++)
	{
		scene->GetInstanceByIndex(&ipinstance, currentinstance);

		int32 classSig = ipinstance->GetClassSignature();
		if (classSig != 1382437959)//terrain
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
#if VERSIONNUMBER >= 0x050000
			objectmesh = ipinstance->GetFacetMesh(0.00025000001f);
#else
            ipinstance->GetFMesh(0.00025000001f, &objectmesh);
#endif

		if (objectmesh != NULL)
		{
			objectmesh->Clone(&amesh);
		}
		else
		{
			FacetMesh::Create(&amesh);
		}
		
		maxvertices = amesh->fVertices.GetElemCount();

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

	//now translate back to global
	maxvertices = newmesh->fVertices.GetElemCount();
	for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
	{
		LocalToGlobal(L2G, newmesh->fVertices[vertexindex], newmesh->fVertices[vertexindex]);
	}

	uint32 maxedges = newmesh->fEdgeList.GetElemCount();
	instance.oneoverlinemagnitude.SetElemCount(maxedges);
	instance.p2minusp1.SetElemCount(maxedges);
	drawedge.SetElemCount(maxedges);
	//precalc the edge values
	for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
	{

		const TVector3& p1 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[edgeindex].x];
		const TVector3& p2 = newmesh->fVertices[newmesh->fEdgeList.fVertexIndices[edgeindex].y];
		//if the end points are the same, don't draw this edge
		if (p1 == p2)
		{
			instance.p2minusp1[edgeindex] = TVector3(0,0,0);
			drawedge[edgeindex] = false;
		}
		else
		{
			instance.oneoverlinemagnitude[edgeindex] = 1.0f / sqr(p1, p2);
			instance.p2minusp1[edgeindex] = p2 - p1;
			drawedge[edgeindex] = true;
		}

	}//end edge loop

	newmesh->CalcBBox(bbox);
	LoadEdgeTree(instance, newmesh, OPT_OCTAL_TREE, bbox, drawedge);

}

real Surf::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
#if VERSIONNUMBER >= 0x060000
		|| !globalStorageKey.ipL2G.IsEqual(currentTransform, 0))
#else
		|| globalStorageKey.ipL2G.fRotationAndScale != currentTransform.fRotationAndScale
		|| globalStorageKey.ipL2G.fTranslation != currentTransform.fTranslation)
#endif

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
			TMCColorRGBA tempresult;
			shader->GetColor(tempresult, fullArea, shadingIn);
			result = 0.299 * tempresult.R + 0.587 * tempresult.G + 0.114 * tempresult.B;
		}
		fWireSize *= result;
	}

	TVector3& p = shadingIn.fPoint;

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
	callLocalStorage.closestDistance = min(mindistance, callLocalStorage.closestDistance);
	while (cacheElement->edgetree->enumSiblings(ClosestNode, &CurrentNode, callLocalStorage)==MC_S_OK)
	{
		CheckNode (*cacheElement, CurrentNode, mindistance, p, closestedge);
		callLocalStorage.closestDistance = min(mindistance, callLocalStorage.closestDistance);
	}//end while MC_S_OK

	if (mindistance <= fWireSize)
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
		case foCustom:
			result = fBezierCurve.GetYPos(mindistance / fWireSize);
			break;
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
		case foCustom:
			result = fBezierCurve.GetYPos(minpointdistance / fWireSize);
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
	result = fData.fMinMax.x + result * (fData.fMinMax.y - fData.fMinMax.x);
	return MC_S_OK;
}
real Surf::GetVector(TVector3& result,ShadingIn& shadingIn)
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

void Surf::CheckNode(const TerrainToolsCache& instance, IEdgeTree* Node, real32& mindistance
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
		if (u < 0)
		{
			real32 distance = sqrt(sqr(point, p1));
			if (distance < mindistance)
			{
				mindistance = distance;
			}
		}
		else if (u > 1.0f)
		{
			real32 distance = sqrt(sqr(point, p2));
			if (distance < mindistance)
			{
				mindistance = distance;
			}
		}
		else if (( u >= 0)&&(u <= 1.0f))
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

void Surf::LoadEdgeTree(TerrainToolsCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox, TMCArray<boolean> drawedge)
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

MCCOMErr Surf::HandleEvent(MessageID message, IMFResponder* source, void* data)
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


 	return DCGBezier::HandleEvent(message, source, data);
}


void Surf::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
         TMCCountedCreateHelper<IExDataExchanger> result(res);
         Surf* clone = new Surf();
         result = (IExDataExchanger*)clone;

         clone->CopyDataFrom(this);

         clone->SetControllingUnknown(pUnkOuter);
}

void Surf::CopyDataFrom(const Surf* source)
{
         fBezierCurve.CopyDataFrom(&source->fBezierCurve);
		 fData = source->fData;
}

MCCOMErr Surf::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_Surf))
	{
		TMCCountedGetHelper<Surf> result(ppvObj);
		result = (Surf*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);
}

uint32 Surf::AddRef()
{
	return TBasicShader::AddRef();
}

void Surf::emptyElement(TerrainToolsCache& oldElement) {
	oldElement.cleanup();
}

void Surf::fillElement(TerrainToolsCache& newElement, const ShadingIn& shadingIn) {
	BuildCache(newElement, shadingIn);
}