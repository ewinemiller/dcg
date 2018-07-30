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
#include "Intersect.h"

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

#include "triangleintersection.h"


//#include "Windows.h"
//#include <stdio.h>
//	char temp[80];
//	sprintf(temp, "create %i on %i\n\0", this, GetCurrentThreadId());
//	OutputDebugString(temp);

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Intersect(R_CLSID_Intersect);
const MCGUID IID_Intersect(R_IID_Intersect);
#else
const MCGUID CLSID_Intersect={R_CLSID_Intersect};
const MCGUID IID_Intersect={R_IID_Intersect};
#endif
const real32 HALF_PI = 1.5707963267948966192313216916398f;
#define EXPIRE_INSTANCE_LIMIT 10000

#define IP_SIG 1768842864

DCGSharedCache<TerrainToolsCache, ShadingIn, IntersectKey> intersectCache;

Intersect::Intersect() : DCGSharedCacheClient<TerrainToolsCache, ShadingIn, IntersectKey>(intersectCache)
{
	fData.fWireSize = 2.0f;
	fData.lFalloff = foRadial;
	fData.bInvert = false;
	fData.fMinMax.x = 0;
	fData.fMinMax.y = 1.0f;
	fData.lEffect = seInclude;
}

Intersect::~Intersect()
{
	releaseCache();
}


void* Intersect::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Intersect::ExtensionDataChanged()
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

boolean	Intersect::IsEqualTo(I3DExShader* aShader){
	return (false);
	}

MCCOMErr Intersect::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.CombineFlagsWith(ChildFlags);
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
	}

EShaderOutput Intersect::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue | kUsesGetVector | kUsesGetShaderApproxColor);
	}

void Intersect::BuildCache(TerrainToolsCache& instance, const ShadingIn& shadingIn)
{
	TMCDynamicString localList(fData.sList);
	localList.Append("\r", 1);
	
	TMCCountedPtr<I3DShTreeElement> tree;
	tree = shadingIn.fInstance->GetTreeElement();
	TMCCountedPtr<FacetMesh> objectmesh;
	TMCCountedPtr<FacetMesh> amesh;
	TMCCountedPtr<FacetMesh> mymesh;
	TMCCountedPtr<FacetMesh> newmesh;
	TMCArray<real32> linemagnitude;
	TMCCountedPtr<I3DShScene> scene;
	TMCArray<boolean> drawedge;
	TMCCountedPtr<I3DShTreeElement> iptree;
	TMCCountedPtr<I3DShInstance> ipinstance;
	TTransform3D L2G;		//Transformation from Local to Global
	TTransform3D inverseL2G;		//Transformation from Local to Global
	TTransform3D ipL2G;		//Transformation from Local to Global

	TBBox3D bbox;
	TBBox3D mybbox;

	FacetMesh::Create(&newmesh);
	instance.mesh = newmesh;
	uint32 lNewEdgeIndex = 0, lNewVertexIndex = 0;
	uint32 lNewVertexCount = 0, lNewEdgeCount = 0;
	uint32 maxvertices;

	int32 myClassSig = shadingIn.fInstance->GetClassSignature();

    tree->GetScene(&scene);

	L2G = tree->GetGlobalTransform3D(kCurrentFrame);
	uint32 instancecount = scene->GetInstanceListCount();

#if VERSIONNUMBER >= 0x050000
		{
			objectmesh = shadingIn.fInstance->GetRenderingFacetMesh();
		}
#else
		shadingIn.fInstance->GetFMesh(0.00025000001f, &objectmesh);
#endif
	if (objectmesh != NULL)
	{
		objectmesh->Clone(&mymesh);
	}
	else
	{
		FacetMesh::Create(&mymesh);
	}
	
	maxvertices = mymesh->fVertices.GetElemCount();

	//run through the vertices and translate them to global space
	for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
	{
		LocalToGlobal(L2G, mymesh->fVertices[vertexindex], mymesh->fVertices[vertexindex]);
		mybbox.AddPoint(mymesh->fVertices[vertexindex]);
	}
	//spin through all the other instances and compare them to me if 
	//needed
	for (uint32 currentinstance = 0; currentinstance < instancecount; currentinstance++)
	{
		TBBox3D instancebbox;
		scene->GetInstanceByIndex(&ipinstance, currentinstance);
		if (ipinstance->GetInstanceKind() != I3DShInstance::kPrimitiveInstance)
		{
			continue;
		}
		int32 instanceClassSig = ipinstance->GetClassSignature();


		//if this is me then skip
		if (ipinstance.fObject == shadingIn.fInstance)
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
		//is the object in the list?
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
		{
			objectmesh = ipinstance->GetRenderingFacetMesh();
		}
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

		//clear out the object mesh reference
		objectmesh = NULL;

		maxvertices = amesh->fVertices.GetElemCount();

		//run through the vertices and translate them to global space
		for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
		{
			LocalToGlobal(ipL2G, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);
			instancebbox.AddPoint(amesh->fVertices[vertexindex]);
		}

		if (myClassSig == IP_SIG && instanceClassSig != IP_SIG)
		{
			TTransform3D L2GInv;
			InvertTransform(L2G, L2GInv);
			uint32 lastmaxvertices = newmesh->fVertices.GetElemCount();
			//now run through the vertices and translate them into the infinite plane's local space
			for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
			{
				GlobalToLocalInv(L2GInv, amesh->fVertices[vertexindex], amesh->fVertices[vertexindex]);
			}
			doPlaneObjectIntersect(amesh, newmesh, lNewEdgeIndex, lNewVertexIndex
				, lNewVertexCount, lNewEdgeCount);

			//now translate back to global
			maxvertices = newmesh->fVertices.GetElemCount();
			for (uint32 vertexindex = lastmaxvertices; vertexindex < maxvertices; vertexindex++)
			{
				LocalToGlobal(L2G, newmesh->fVertices[vertexindex], newmesh->fVertices[vertexindex]);
			}
		}
		else if (instanceClassSig == IP_SIG && myClassSig != IP_SIG)
		{
			TTransform3D L2GInv;
			InvertTransform(ipL2G, L2GInv);

			uint32 lastmaxvertices = newmesh->fVertices.GetElemCount();
			TMCCountedPtr<FacetMesh> myclone;
			mymesh->Clone(&myclone);

			maxvertices = myclone->fVertices.GetElemCount();
			//now run through the vertices and translate them into the infinite plane's local space
			for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
			{
				GlobalToLocalInv(L2GInv, myclone->fVertices[vertexindex], myclone->fVertices[vertexindex]);
			}
			doPlaneObjectIntersect(myclone, newmesh, lNewEdgeIndex, lNewVertexIndex
				, lNewVertexCount, lNewEdgeCount);

			//now translate back to global
			maxvertices = newmesh->fVertices.GetElemCount();
			for (uint32 vertexindex = lastmaxvertices; vertexindex < maxvertices; vertexindex++)
			{
				LocalToGlobal(ipL2G, newmesh->fVertices[vertexindex], newmesh->fVertices[vertexindex]);
			}


		}
		//find the intersection of mybbox and instancebbox
		//use that to throw out any facets that aren't a possible intersection
		else 
#if VERSIONNUMBER >= 0x050000			
		if (mybbox.Intersect(instancebbox))
#endif
		{
			TMCArray<boolean> myIntersect;
			TMCArray<boolean> instanceIntersect;
			uint32 myFacetCount, instanceFacetCount;

			if (mybbox.fMin.x > instancebbox.fMin.x)
				instancebbox.fMin.x = mybbox.fMin.x;
			if (mybbox.fMin.y > instancebbox.fMin.y)
				instancebbox.fMin.y = mybbox.fMin.y;
			if (mybbox.fMin.z > instancebbox.fMin.z)
				instancebbox.fMin.z = mybbox.fMin.z;

			if (mybbox.fMax.x < instancebbox.fMax.x)
				instancebbox.fMax.x = mybbox.fMax.x;
			if (mybbox.fMax.y < instancebbox.fMax.y)
				instancebbox.fMax.y = mybbox.fMax.y;
			if (mybbox.fMax.z < instancebbox.fMax.z)
				instancebbox.fMax.z = mybbox.fMax.z;
			
			myFacetCount = mymesh->fFacets.GetElemCount();
			instanceFacetCount = amesh->fFacets.GetElemCount();

			myIntersect.SetElemCount(myFacetCount);
			instanceIntersect.SetElemCount(instanceFacetCount);
			
			for (uint32 myFacetIndex = 0; myFacetIndex < myFacetCount; myFacetIndex++)
			{
				const TVector3& myPt1 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt1];
				const TVector3& myPt2 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt2];
				const TVector3& myPt3 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt3];
				if ((myPt1.x < instancebbox.fMin.x && myPt2.x < instancebbox.fMin.x && myPt3.x < instancebbox.fMin.x)
					||
					(myPt1.y < instancebbox.fMin.y && myPt2.y < instancebbox.fMin.y && myPt3.y < instancebbox.fMin.y)
					||
					(myPt1.z < instancebbox.fMin.z && myPt2.z < instancebbox.fMin.z && myPt3.z < instancebbox.fMin.z)
					||
					(myPt1.x > instancebbox.fMax.x && myPt2.x > instancebbox.fMax.x && myPt3.x > instancebbox.fMax.x)
					||
					(myPt1.y > instancebbox.fMax.y && myPt2.y > instancebbox.fMax.y && myPt3.y > instancebbox.fMax.y)
					||
					(myPt1.z > instancebbox.fMax.z && myPt2.z > instancebbox.fMax.z && myPt3.z > instancebbox.fMax.z))
				{
					myIntersect[myFacetIndex] = false;
				}
				else
				{
					myIntersect[myFacetIndex] = true;
				}
			}


			for (uint32 instanceFacetIndex = 0; instanceFacetIndex < instanceFacetCount; instanceFacetIndex++)
			{
				const TVector3& instancePt1 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt1];
				const TVector3& instancePt2 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt2];
				const TVector3& instancePt3 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt3];
				TBBox3D facetbbox;
				facetbbox.AddPoint(instancePt1);
				facetbbox.AddPoint(instancePt2);
				facetbbox.AddPoint(instancePt3);

				if ((instancePt1.x < instancebbox.fMin.x && instancePt2.x < instancebbox.fMin.x && instancePt3.x < instancebbox.fMin.x)
					||
					(instancePt1.y < instancebbox.fMin.y && instancePt2.y < instancebbox.fMin.y && instancePt3.y < instancebbox.fMin.y)
					||
					(instancePt1.z < instancebbox.fMin.z && instancePt2.z < instancebbox.fMin.z && instancePt3.z < instancebbox.fMin.z)
					||
					(instancePt1.x > instancebbox.fMax.x && instancePt2.x > instancebbox.fMax.x && instancePt3.x > instancebbox.fMax.x)
					||
					(instancePt1.y > instancebbox.fMax.y && instancePt2.y > instancebbox.fMax.y && instancePt3.y > instancebbox.fMax.y)
					||
					(instancePt1.z > instancebbox.fMax.z && instancePt2.z > instancebbox.fMax.z && instancePt3.z > instancebbox.fMax.z))
				{
					instanceIntersect[instanceFacetIndex] = false;
				}
				else
				{
					//there's a possibility of an intersect, but only set it true
					//if we actually find one
					boolean bHasIntersect = false;
					for (uint32 myFacetIndex = 0; myFacetIndex < myFacetCount; myFacetIndex++)
					{
						if (myIntersect[myFacetIndex])
						{
							const TVector3& myPt1 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt1];
							const TVector3& myPt2 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt2];
							const TVector3& myPt3 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt3];
							if ((myPt1.x < facetbbox.fMin.x && myPt2.x < facetbbox.fMin.x && myPt3.x < facetbbox.fMin.x)
								||
								(myPt1.y < facetbbox.fMin.y && myPt2.y < facetbbox.fMin.y && myPt3.y < facetbbox.fMin.y)
								||
								(myPt1.z < facetbbox.fMin.z && myPt2.z < facetbbox.fMin.z && myPt3.z < facetbbox.fMin.z)
								||
								(myPt1.x > facetbbox.fMax.x && myPt2.x > facetbbox.fMax.x && myPt3.x > facetbbox.fMax.x)
								||
								(myPt1.y > facetbbox.fMax.y && myPt2.y > facetbbox.fMax.y && myPt3.y > facetbbox.fMax.y)
								||
								(myPt1.z > facetbbox.fMax.z && myPt2.z > facetbbox.fMax.z && myPt3.z > facetbbox.fMax.z))
							{
							}
							else
							{
								TVector3 pt1, pt2;
								int coplaner;
								if (tri_tri_intersect_with_isectline(&instancePt1[0], &instancePt2[0], &instancePt3[0]
									,&myPt1[0], &myPt2[0] , &myPt3[0]
									, &coplaner, &pt1[0], &pt2[0]) == 1)
								{
									lNewVertexCount+=2;
									lNewEdgeCount+=1;
									bHasIntersect = true;
								}
							}
						}
					}
					instanceIntersect[instanceFacetIndex] = bHasIntersect;
				}
			}

			newmesh->fVertices.SetElemCount(lNewVertexCount);
			newmesh->fEdgeList.SetElemCount(lNewEdgeCount);
			for (uint32 instanceFacetIndex = 0; instanceFacetIndex < instanceFacetCount; instanceFacetIndex++)
			{
				if (instanceIntersect[instanceFacetIndex])
				{
					const TVector3& instancePt1 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt1];
					const TVector3& instancePt2 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt2];
					const TVector3& instancePt3 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt3];
					TBBox3D facetbbox;
					facetbbox.AddPoint(instancePt1);
					facetbbox.AddPoint(instancePt2);
					facetbbox.AddPoint(instancePt3);
					for (uint32 myFacetIndex = 0; myFacetIndex < myFacetCount; myFacetIndex++)
					{
						if (myIntersect[myFacetIndex])
						{
							const TVector3& myPt1 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt1];
							const TVector3& myPt2 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt2];
							const TVector3& myPt3 = mymesh->fVertices[mymesh->fFacets[myFacetIndex].pt3];
							if ((myPt1.x < facetbbox.fMin.x && myPt2.x < facetbbox.fMin.x && myPt3.x < facetbbox.fMin.x)
								||
								(myPt1.y < facetbbox.fMin.y && myPt2.y < facetbbox.fMin.y && myPt3.y < facetbbox.fMin.y)
								||
								(myPt1.z < facetbbox.fMin.z && myPt2.z < facetbbox.fMin.z && myPt3.z < facetbbox.fMin.z)
								||
								(myPt1.x > facetbbox.fMax.x && myPt2.x > facetbbox.fMax.x && myPt3.x > facetbbox.fMax.x)
								||
								(myPt1.y > facetbbox.fMax.y && myPt2.y > facetbbox.fMax.y && myPt3.y > facetbbox.fMax.y)
								||
								(myPt1.z > facetbbox.fMax.z && myPt2.z > facetbbox.fMax.z && myPt3.z > facetbbox.fMax.z))
							{
							}
							else
							{
								TVector3 pt1, pt2;
								int coplaner;
								if (tri_tri_intersect_with_isectline(&instancePt1[0], &instancePt2[0], &instancePt3[0]
									,&myPt1[0], &myPt2[0] , &myPt3[0]
									, &coplaner, &pt1[0], &pt2[0]) == 1)
								{
									newmesh->fVertices[lNewVertexIndex] = pt1;
									newmesh->fVertices[lNewVertexIndex + 1] = pt2;
									newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].x = lNewVertexIndex;
									newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].y = lNewVertexIndex + 1;
									lNewVertexIndex+=2;
									lNewEdgeIndex+=1;
								}
							}
						}
					}
				}
					
			}
		}//end box/box intersection
	}//end instance loop

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

real Intersect::GetVector(TVector3& result,ShadingIn& shadingIn)
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

real Intersect::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
		|| (!globalStorageKey.ipL2G.IsEqual(currentTransform, 0)
			&& globalStorageKey.instance == shadingIn.fInstance))

#else
		|| ((globalStorageKey.ipL2G.fRotationAndScale != currentTransform.fRotationAndScale
			|| globalStorageKey.ipL2G.fTranslation != currentTransform.fTranslation)
			&& globalStorageKey.instance == shadingIn.fInstance)))
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


void Intersect::CheckNode(const TerrainToolsCache& instance, IEdgeTree* Node, real32& mindistance
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

void Intersect::LoadEdgeTree(TerrainToolsCache& instance, const TMCCountedPtr<FacetMesh>& amesh
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

MCCOMErr Intersect::HandleEvent(MessageID message, IMFResponder* source, void* data)
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


void Intersect::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
         TMCCountedCreateHelper<IExDataExchanger> result(res);
         Intersect* clone = new Intersect();
         result = (IExDataExchanger*)clone;

         clone->CopyDataFrom(this);

         clone->SetControllingUnknown(pUnkOuter);
}

void Intersect::CopyDataFrom(const Intersect* source)
{
         fBezierCurve.CopyDataFrom(&source->fBezierCurve);
		 fData = source->fData;
}

MCCOMErr Intersect::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_Intersect))
	{
		TMCCountedGetHelper<Intersect> result(ppvObj);
		result = (Intersect*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);
}

uint32 Intersect::AddRef()
{
	return TBasicShader::AddRef();
}

void Intersect::emptyElement(TerrainToolsCache& oldElement) {
	oldElement.cleanup();
}

void Intersect::fillElement(TerrainToolsCache& newElement, const ShadingIn& shadingIn) {
	BuildCache(newElement, shadingIn);
}