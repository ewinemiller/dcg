/*  Wirerender utilities for Carrara
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
#include "copyright.h"
#include "math.h"
#include "WireRenderer.h"
#include "MCCountedPtrHelper.h"
#include "I3DShUtilities.h"
#include "PublicUtilities.h"
#include "IShRasterLayerUtilities.h"
#include "comutilities.h"
#include "TBBox.h"
#include "WireframeFacetMeshAccumulator.h"
#include <stdlib.h>
#include "IShThreadUtilities.h"
#include "I3DShScene.h"
#include "IShUtilities.h"
#include "MCVersionNumber.h"
#include "I3DExVertexPrimitive.h"
#include "I3dExPrimitive.h"
#include "I3DShObject.h"
#include "IShComponent.h"
#include "IPolymesh.h"
#include "IPolygon.h"
#include "IEdge.h"
#include "I3dExSkinable.h"
#if VERSIONNUMBER >= 0x050000
#include "comsafeutilities.h"
#endif

#include "I3DShComponentOwner.h"
#include "dcgmeshutil.h"
#include "dcgdebug.h"


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_WireRenderer(R_CLSID_WireRenderer);
const MCGUID CLSID_ToonRenderer(R_CLSID_ToonRenderer);

#define R_CLSID_PhotoRenderer 0x0709aed1,0xeb93,0x11d1,0x84,0x29,0x00,0x60,0x97,0x0b,0xc5,0x21
//extern const MCGUID CLSID_PhotoRenderer;
const MCGUID CLSID_PhotoRenderer(R_CLSID_PhotoRenderer);
#else
const MCGUID CLSID_WireRenderer={R_CLSID_WireRenderer};
const MCGUID CLSID_ToonRenderer={R_CLSID_ToonRenderer};

#define R_CLSID_PhotoRenderer 0x0709aed1,0xeb93,0x11d1,0x84,0x29,0x00,0x60,0x97,0x0b,0xc5,0x21
//extern const MCGUID CLSID_PhotoRenderer;
const MCGUID CLSID_PhotoRenderer={R_CLSID_PhotoRenderer};
#endif

const real32 BEHINDCAMERAFIX = 0.1f;

static uint32 lineindexes[3][2] =
{
	{0, 1}, {1, 2}, {2, 0}
};

inline boolean Unlocked(int32 x, int32 y) 
{	
	int32 top = y / 64;
	int32 left = x / 64;
	return ((top + left) % 2 == 1);
}

// Constructor of the renderer

WireRenderer::WireRenderer(RenderStyle renderStyle)

{
	gComponentUtilities->CoCreateInstance(CLSID_PhotoRenderer, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DExFinalRenderer, reinterpret_cast<void**>(&baserenderer));

	fDrawingArea.left = 0;
	fDrawingArea.right = 0;
	fDrawingArea.top = 0;
	fDrawingArea.bottom = 0;

	fImageArea.left = 0;
	fImageArea.right = 0;
	fImageArea.top = 0;
	fImageArea.bottom = 0;

	fOffscrOffset[0] = 0.0f;
	fOffscrOffset[1] = 0.0f;

	fZoom[0] = 1.0f;
	fZoom[1] = 1.0f;

#if VERSIONNUMBER >= 0x050000
	TMCCountedPtr<IExDataExchanger> dataexchanger;
#else
	TMCCountedPtr<TBasicDataExchanger> dataexchanger;
#endif
	
	baserenderer->QueryInterface(IID_IExDataExchanger, reinterpret_cast<void**>(&dataexchanger));
	if (dataexchanger)
	{
		RealisticRendererPublicData* basedata;
		basedata = reinterpret_cast<RealisticRendererPublicData*>(dataexchanger->GetExtensionDataBuffer());

		fData.basedata = *basedata;

	}

	//get the basegbufferrenderer
	baserenderer->QueryInterface(IID_I3DExGBufferRenderer, reinterpret_cast<void**>(&basegbufferrenderer));



	fData.fLineWidth = 2;
	fData.lRenderStyle = renderStyle;
	if (fData.lRenderStyle == rsToon)
	{
		fData.fVectorAngle = 30;
		fData.bSmartQuads = false;
		fData.bVertexEdges = false;
		//toon! always does silhouette edges so doesn't need the special override
		fData.bSilhouetteEdges = false;
	}
	else
	{
		fData.fVectorAngle = 1;
		fData.bSmartQuads = true;
		fData.bVertexEdges = true;
		fData.bSilhouetteEdges = true;
	}
	fData.fOverdraw = 0;
	fData.fCueDepth = 100;
	fData.bDepthCueing = false;
	fData.bBackfaceOptimization = true;
	fData.bRemoveHiddenLines = true;
	fData.bDrawBackfaces = false;
	fData.iBucketOptimization = 4;
	fData.lineColor.Set(0,0,0,0);
	fData.fLineEffect = 1.0f;
	fData.bFilterEdges = true;
	fData.iOversampling = 2;
	fData.bUseObjectColor = false;
	fData.bBaseRender = true;
	fData.doDomainBoundaryEdges = false;
	fData.alphaContent = acBaseRender;

}

WireRenderer::~WireRenderer() {
	if (calcKey)
		gShellUtilities->EndProgress(calcKey);
	if (removeKey)
		gShellUtilities->EndProgress(removeKey);
	if (drawKey)
		gShellUtilities->EndProgress(drawKey);
}



//*****************************************************************

//				BasicDataExchanger Methods

//*****************************************************************



void* WireRenderer::GetExtensionDataBuffer()

{

	return ((void*) &(fData));

}	// used by the shell to set the new parameters


int16 WireRenderer::GetResID()

{
	TMCVersionNumber VersionNumber;
	gShellUtilities->GetVersion(VersionNumber);
	if (fData.lRenderStyle == rsToon)
	{
		switch (VersionNumber.fMajor)
		{
		case 4:
			return 136;
			break;
		case 5:
			return 130;
			break;
		case 6:
			return 130;
			break;
		case 7:
		case 8:
			return 131;
			break;
		}
	}
	else
	{
		switch (VersionNumber.fMajor)
		{
		case 4:
			return 137;
			break;
		case 5:
			return 130;
			break;
		case 6:
			return 130;
			break;
		case 7:
		case 8:
			return 131;
			break;
		}
	}

}	


MCCOMErr WireRenderer::ExtensionDataChanged()
{
#if VERSIONNUMBER >= 0x050000
	TMCCountedPtr<IExDataExchanger> dataexchanger;
#else
	TMCCountedPtr<TBasicDataExchanger> dataexchanger;
#endif
	baserenderer->QueryInterface(IID_IExDataExchanger, reinterpret_cast<void**>(&dataexchanger));
	if (dataexchanger)
	{
		RealisticRendererPublicData* basedata;
		basedata = reinterpret_cast<RealisticRendererPublicData*>(dataexchanger->GetExtensionDataBuffer());

		*basedata = fData.basedata;

		dataexchanger->ExtensionDataChanged();
	}
	if (fData.iOversampling < 1)
		fData.iOversampling = 1;
	return MC_S_OK;

}

MCCOMErr WireRenderer::HandleEvent			(MessageID message, IMFResponder* source, void* data)
{
#if VERSIONNUMBER >= 0x050000
	TMCCountedPtr<IExDataExchanger> dataexchanger;
#else
	TMCCountedPtr<TBasicDataExchanger> dataexchanger;
#endif
	baserenderer->QueryInterface(IID_IExDataExchanger, reinterpret_cast<void**>(&dataexchanger));
	if (dataexchanger)
	{
		return dataexchanger->HandleEvent(message, source, data);
	}
	return MC_S_OK;

}



//*****************************************************************

//				I3DExRenderer Methods

//*****************************************************************



// specifies which tree needs to be rendered (generally the universe)
MCCOMErr WireRenderer::Abort()
{
	//bDoAbort = true; 
	return MC_S_OK; 
}

MCCOMErr WireRenderer::SetTreeTop(I3DShGroup* treeTop)

{
	baserenderer->SetTreeTop(treeTop);
	
	fTreeTop = treeTop;

	return MC_S_OK;

}



// specifies the rendering camera

MCCOMErr WireRenderer::SetCamera (I3DShCamera* camera)

{
	baserenderer->SetCamera(camera);

	fCamera = camera;

	return MC_S_OK;

} 



// gives the background, backdrop & atmospheric shader.

MCCOMErr WireRenderer::SetEnvironment (I3DShEnvironment* environment)

{
	baserenderer->SetEnvironment(environment);
	if (!environment || environment->HasBackground()) fBackground=environment;
	if (!environment || environment->HasBackdrop()) fBackdrop=environment;
	if (!environment || environment->HasAtmosphere()) fAtmosphere=environment;
	return MC_S_OK;

}	



// sets the ambient light color

MCCOMErr WireRenderer::SetAmbientLight (const TMCColorRGB &ambientColor)

{
	baserenderer->SetAmbientLight(ambientColor);
	return MC_S_OK;

}



// used to specify which buffers owned by the component could be freed in case Kwak would lack memory

MCCOMErr WireRenderer::Dehydrate (int16 level)
{
	return MC_S_OK;
}



// this corresponds to an option in Scene Settings/Output

uint32 WireRenderer::GetRenderingTime ()

{
	return 0;

}



// multiple frames for video compositing (option in Scene Settings/Renderer)

MCCOMErr WireRenderer::SetFieldRenderingData (int32 useFieldRendering,int16 firstFrame)

{
	baserenderer->SetFieldRenderingData(useFieldRendering,firstFrame);
	return MC_S_OK;

}

//I3DExGBufferRenderer
#if VERSIONNUMBER >= 0x070000
MCCOMErr MCCOMAPI WireRenderer::SetGBufferNeed(const TMCArray<TGBufferInfo>& bufferInfos)
{
	return basegbufferrenderer->SetGBufferNeed(bufferInfos);
}

#else
MCCOMErr WireRenderer::SetGBufferNeed(uint32 bufferNbr,uint32* bufferID)
{
	return basegbufferrenderer->SetGBufferNeed(bufferNbr, bufferID);
}
#endif




//*****************************************************************

//				WireRenderer Methods

//*****************************************************************

void CompareMesh(TMCCountedPtr<FacetMesh> amesh, TMCCountedPtr<FacetMesh> pmesh)
{
	for (int i = 0; i < amesh->fVertices.GetElemCount();i++)
	{
		TVector3& a = amesh->fVertices[i];
		TVector3& p = pmesh->fVertices[i];
		if (a != p)
		{
			uint j = 5;
		}
	}
	for (int i = 0; i < amesh->fNormals.GetElemCount();i++)
	{
		TVector3& a = amesh->fNormals[i];
		TVector3& p = pmesh->fNormals[i];
		if (a != p)
		{
			uint j = 5;
		}
	}
	for (int i = 0; i < amesh->fFacets.GetElemCount();i++)
	{
		Triangle& a = amesh->fFacets[i];
		Triangle& p = pmesh->fFacets[i];
		if (a.pt1 != p.pt1 || a.pt2 != p.pt2 || a.pt3 != p.pt3)
		{
			uint j = 5;
		}
	}
}

void WireRenderer::FillObjectCaches(FacetMesh **inoutmesh
									, TMCArray<boolean> &awayfromcamera
									, TMCArray<TVector3> &facenormals
									, TMCArray<FacetEdges> &facetedges
									, TMCArray<real32>	&linemagnitude
									, TMCArray<boolean> &drawedge
									, const TTransform3D &L2C 
									, const TTransform3D &L2G
									, const TTransform3D &cameraTransform
									, uint32 stage
									, I3DShInstance *instance
									, boolean& bDoVMEdges
									, TMCArray<boolean> &drawFacet)
{
	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	TMCCountedPtr<I3DExVertexPrimitive> originalVertexPrimitive;
	TMCCountedPtr<I3DShPrimitive> primitive;
	TMCCountedGetHelper<FacetMesh> outMeshHelper(inoutmesh);
	TMCCountedPtr<FacetMesh> amesh;
	TMCCountedPtr<I3DShObject> newobject;
	TMCArray<Triangle>	fFacets;
			
	bDoVMEdges = fData.bVertexEdges;
	boolean bIsSmoothTerrain = false;
#if VERSIONNUMBER >= 0x060000 
	boolean isFigure = false;
#endif 

	IDType classSig = instance->GetClassSignature();
	switch (classSig)
	{
	case 1986880609://vertex object
		if (bDoVMEdges)
		{

			TMCCountedPtr<I3DShObject> baseobject;
			TMCCountedPtr<IShComponent> component;
			TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
			TMCCountedPtr<I3DShPrimitiveComponent> primcomp;

			instance->Get3DObject(&baseobject);
			ThrowIfNil(baseobject);

			baseobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
			ThrowIfNil(extprimitive);

			extprimitive->GetPrimitiveComponent(&component);
			ThrowIfNil(component);

			component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
			ThrowIfNil(primcomp);

			primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);
			ThrowIfNil(vertexPrimitive);
			
#if VERSIONNUMBER >= 0x060000 
			isFigure = vertexPrimitive->GetIsFigurePrimitive();
#endif
			originalVertexPrimitive = vertexPrimitive;

			baseobject->Clone(&newobject, kWithAnim);
			ThrowIfNil(newobject);


			newobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
			ThrowIfNil(extprimitive);

			extprimitive->GetPrimitiveComponent(&component);
			ThrowIfNil(component);

			component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
			ThrowIfNil(primcomp);

			primcomp->QueryInterface(IID_I3DExVertexPrimitive,(void**)&vertexPrimitive);
			ThrowIfNil(vertexPrimitive);

#if VERSIONNUMBER >= 0x060000 
			vertexPrimitive->Hydrate();
#endif

			newobject->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
			ThrowIfNil(primitive);
		}

#if VERSIONNUMBER >= 0x050000
		amesh = instance->GetRenderingFacetMesh() ;		//get a pointer to IFacetMesh interface
#else
		instance->GetFMesh(0.0, &amesh) ;		//get a pointer to IFacetMesh interface
#endif		
		break;
	case 1382437959://terrain
		{
			TMCCountedPtr<I3DShObject> object;
			TMCCountedPtr<IExDataExchanger> dataexchanger;
			instance->Get3DObject(&object);

			if (object->QueryInterface(IID_IExDataExchanger, (void**)&dataexchanger)== MC_S_OK) 
			{
				dataexchanger->GetParameter('SMOO', &bIsSmoothTerrain);
			}
		}
	default:
#if VERSIONNUMBER >= 0x050000
	amesh = instance->GetRenderingFacetMesh() ;		//get a pointer to IFacetMesh interface
#else
	instance->GetFMesh(0.0, &amesh) ;		//get a pointer to IFacetMesh interface
#endif
		bDoVMEdges = false;
		break;
	
	}

	if (bIsSmoothTerrain)
	{
		amesh->Clone(&amesh);
		if (fData.lRenderStyle == rsToon)
		{
			boolean drawFacetValue = true;

			if (overrideCache.cache[currentObjectIndex].GetElemCount() > 0)
				drawFacetValue = (overrideCache.cache[currentObjectIndex].getDomainByIndex(0).lineEffect > 0);

			uint32 maxfacets = amesh->fFacets.GetElemCount();
			drawFacet.SetElemCount(maxfacets);
			drawFacet.FillWithValue(drawFacetValue);
		}
	}
	else
	{//so the accumulator goes out of scope when it's done
		TFacet3D aFacet;
		uint32 maxfacets = amesh->fFacets.GetElemCount();
		WireframeFacetMeshAccumulator accu;
		accu.PrepareAccumulation(maxfacets);
		drawFacet.SetElemCount(maxfacets);
		for (uint32 facetIndex = 0; facetIndex < maxfacets; facetIndex++)
		{
			drawFacet[facetIndex] = true;
			if (fData.lRenderStyle == rsToon)
			{
				uint32 uvSpaceId = amesh->fUVSpaceID[facetIndex];
				if (overrideCache.cache[currentObjectIndex].GetElemCount() > 0)
					drawFacet[facetIndex] = 
						(overrideCache.cache[currentObjectIndex].getDomainByIndex(uvSpaceId).lineEffect > 0);
			}

			aFacet.fVertices[0].fVertex =  amesh->fVertices[amesh->fFacets[facetIndex].pt1];
			aFacet.fVertices[1].fVertex =  amesh->fVertices[amesh->fFacets[facetIndex].pt2];
			aFacet.fVertices[2].fVertex =  amesh->fVertices[amesh->fFacets[facetIndex].pt3];
			aFacet.fUVSpace = amesh->fUVSpaceID[facetIndex];
			accu.AccumulateFacet(&aFacet);
		}
		accu.MakeFacetMesh(&amesh);

	}//end accu life span


	if(stage == 1)
	{
		TMCArray<TIndex2> vmedges;
		long numvmedges = 0;
		//check to see if it's a sphere, it uses a special case
		if (bDoVMEdges)
		{
			TMCCountedPtr<FacetMesh> vmesh;		
			TMCCountedPtrArray<IPolymesh> polymeshes;
#if VERSIONNUMBER >= 0x060000 
			if (isFigure) 
			{
				fixBadFigureClone(vertexPrimitive, originalVertexPrimitive);
			}
#endif
			removeBonesAndMorphs(vertexPrimitive);

#if VERSIONNUMBER >= 0x050000 
			primitive->GetRenderingFacetMesh(&vmesh);
#else
			primitive->GetFMesh(0.00025000001f, &vmesh);
#endif
			ThrowIfNil(vmesh);

			//FacetMeshFacetIterator facetIterator;
			WireframeFacetMeshAccumulator accu;

			//we want a mesh where matching vertices are 
			//considered a single vertex, so blank the normal
			//and UV values
			uint32 lMaxFacets = vmesh->fFacets.GetElemCount();
			accu.PrepareAccumulation(vmesh->fFacets.GetElemCount());
			for (uint32 facetindex = 0; facetindex < lMaxFacets; facetindex++)
			{	
				TFacet3D aFacet;
				aFacet.fVertices[0].fVertex = vmesh->fVertices[vmesh->fFacets[facetindex].pt1];
				aFacet.fVertices[1].fVertex = vmesh->fVertices[vmesh->fFacets[facetindex].pt2];
				aFacet.fVertices[2].fVertex = vmesh->fVertices[vmesh->fFacets[facetindex].pt3];
				accu.AccumulateFacet(&aFacet);
			}
			accu.MakeFacetMesh(&vmesh);
			//CompareMesh(amesh, vmesh);
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
					if ((polymesh->GetSubdivisionLevel(true) > 0)&&(polymesh->GetSubdivisionType() != eSubdivisionNone))
#else
					if ((polymesh->GetSubdivisionLevel() > 0)&&(polymesh->GetSubdivisionType() != eSubdivisionNone))
#endif					
					{
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

		}//end bDoVMEdges
		amesh->BuildEdgeList();

		uint32 maxedges = amesh->fEdgeList.GetElemCount();
		if (linemagnitude.GetElemCount() < maxedges)
		{
			linemagnitude.SetElemCount(maxedges);
			drawedge.SetElemCount(maxedges);
		}
		for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
		{
			//put lowest vertex index first so we can so we can do a quicker compare later
			if(amesh->fEdgeList.fVertexIndices[edgeindex].x > amesh->fEdgeList.fVertexIndices[edgeindex].y)
			{
				uint32 temp = amesh->fEdgeList.fVertexIndices[edgeindex].x;
				amesh->fEdgeList.fVertexIndices[edgeindex].x = amesh->fEdgeList.fVertexIndices[edgeindex].y;
				amesh->fEdgeList.fVertexIndices[edgeindex].y = temp;
			}

			//assume we do not draw this
			drawedge[edgeindex] = false;
			if (bDoVMEdges) 
			{
				//check to see if it's in our list of edges to draw
				if (bsearch(&amesh->fEdgeList.fVertexIndices[edgeindex], vmedges.Pointer(0)
						, vmedges.GetElemCount(), sizeof(TIndex2), TIndex2Compare))
				{
						drawedge[edgeindex] = true;
				}
			}
			
			const TVector3& p1 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].x];
			const TVector3& p2 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].y];
			//if the end points are the same, don't draw this edge
			if (p1 == p2)
			{
				linemagnitude[edgeindex] = 0;
			}
			else
			{
				linemagnitude[edgeindex] = sqr(p1, p2);
			}
		}
	}//end stage = 1

	uint32 maxfacets = amesh->fFacets.GetElemCount();
	if (facenormals.GetElemCount() < maxfacets)
	{
		awayfromcamera.SetElemCount(maxfacets);
		if (stage == 1)
		{
			facetedges.SetElemCount(maxfacets);
			facenormals.SetElemCount(maxfacets);
		}
	}
	//calculate the normal for each facet
	//and transform it to the camera space
	//so that we can figure out if it's away 
	//from the camera
	TVector3		facenormal;
	for (uint32 facetindex = 0; facetindex < maxfacets; facetindex++)
	{
		TVector3		globalCoordPoint[3];

		globalCoordPoint[0] = L2G.TransformPoint(amesh->fVertices[amesh->fFacets[facetindex].pt1]);
		globalCoordPoint[1] = L2G.TransformPoint(amesh->fVertices[amesh->fFacets[facetindex].pt2]);
		globalCoordPoint[2] = L2G.TransformPoint(amesh->fVertices[amesh->fFacets[facetindex].pt3]);

		//we don't want to draw lines for facets facing away from us
		facenormal = 
			(globalCoordPoint[1] - globalCoordPoint[0])
			^ (globalCoordPoint[2] - globalCoordPoint[1]);
			
		if (cameraInfo.fProjectionType == kProjectionType_kOrthographic)
		{
			awayfromcamera[facetindex] = (facenormal * cameraDirection > 0);
		}
		else
		{
			awayfromcamera[facetindex] = (facenormal * (globalCoordPoint[0] - cameraTransform.fTranslation) > 0);
		}
		if (stage == 1)
		{
			facenormal.Normalize(facenormals[facetindex]);
			facetedges[facetindex].lCurrentIndex = 0;
		}
	}

	uint32 maxvertices = amesh->fVertices.GetElemCount();
	//transform all our points into screen space
	for (uint32 vertexindex = 0; vertexindex < maxvertices; vertexindex++)
	{
		TVector3 screenCoordPoint;
		
		screenCoordPoint = L2C.TransformPoint(amesh->fVertices[vertexindex]);

		//stuff the camera space point in the normal for future reference
		amesh->fNormals[vertexindex] = screenCoordPoint;

		if (screenCoordPoint.z <= 0) {
			//only transform to 2D space if it's in front of the camera
			//otherwise output is just junk
			project3DTo2D(screenCoordPoint, amesh->fVertices[vertexindex]);
		}


	}
	outMeshHelper = amesh;
}


// This method is called by the the PrepareDraw method. It allows us to draw the image in our buffer

void WireRenderer::RenderScene(const TMCRect* area)
{
	bDoAbort = false;
	boolean bDoVMEdges;

	TMCCountedPtr<FacetMesh>	amesh ;				// Facet Mesh data
	TMCArray<FacetEdges> facetedges;
	TMCArray<real32> linemagnitude;
	TMCArray<boolean>	drawedge;
	TMCArray<boolean> awayfromcamera;
	TMCArray<TVector3> facenormals;
	TMCArray<boolean> drawFacets;
	TTransform3D G2C ;		//Transformation from Global to Camera 
	TTransform3D L2C ;		//Transformation from Local to Camera
	TTransform3D L2G ;		//Transformation from Local to Global
	TTransform3D cameraTransform;		//Transformation from Local to Global

	fCamera->GetGlobalToCameraTransform(&G2C) ; //We get transformation from Global to Camera,
												//actually in Screen Coord 
	fCamera->GetStandardCameraInfo(cameraInfo);

	if (cameraInfo.fProjectionType == kProjectionType_kOrthographic)
	{
		TVector3 junk;
		TVector2 origin(0,0);
		fCamera->CreateRay(&origin, &junk, &cameraDirection);
	}

	TMCCountedPtr<I3DShTreeElement>	cameratree;

	fCamera->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&cameratree));

	cameraTransform = cameratree->GetGlobalTransform3D(kCurrentFrame);
	//fCameraPos = cameraTransform.fTranslation;

	//cameraTransform.fTranslation.Normalize(cameraTransform.fTranslation.Normalize);

	int32 theKind;	//Used to stock the kind of an instance
	TRenderableAndTfmArray::const_iterator	iter = fInstances->Begin();

	//count the number of objects
	uint32 maxobjects = 0;
	TMCCountedPtr<I3DShScene> scene;
	for (const TRenderableAndTfm* current = iter.First() ; iter.More(); current = iter.Next())
	{
		theKind = current->fInstance->GetInstanceKind();
		if((theKind == 3) || (theKind == 4)) 
			continue;
		if (maxobjects == 0)
		{
			TMCCountedPtr<I3DShTreeElement>	tree;
			tree = current->fInstance->GetTreeElement();
			tree->GetScene(&scene);
		}
		maxobjects++;
	}

	currentObjectIndex = 0;

	maxLineWidth = fData.fLineWidth;

	overrideCache.Init(scene, fData, maxobjects);

	MCIID riid = IID_ToonEnabled;
	if (fData.lRenderStyle == rsWireFrame)
	{
		riid = IID_WireframeEnabled;
	}
	for (const TRenderableAndTfm* current = iter.First() ; iter.More(); current = iter.Next())
	{
		theKind = current->fInstance->GetInstanceKind();
		if((theKind == 3) || (theKind == 4)) 
			continue;
		TMCCountedPtr<ToonEnabled> toonenabled;
		if(overrideCache.GetToonOverriden(riid, &toonenabled, current->fInstance) == MC_S_OK) 
		{
				TMCCountedPtr<I3DShTreeElement>	tree;
				tree = current->fInstance->GetTreeElement();
				toonenabled->IsActive(tree);
				toonenabled->mergeLevelsWithScene();
				overrideCache.cache[currentObjectIndex] = toonenabled->toonSettings;

				uint32 domainCount = toonenabled->toonSettings.GetElemCount();
				for (uint32 domainIndex = 0; domainIndex < domainCount; domainIndex++)
				{
					ShadingDomainLineLevels& currentDomain = overrideCache.cache[currentObjectIndex].getDomainByIndex(domainIndex);
					if (!currentDomain.overrideSettings)
					{
						currentDomain.lineEffect = fData.fLineEffect;
						currentDomain.lineWidth = fData.fLineWidth;
						currentDomain.color = fData.lineColor;
						currentDomain.depthCue = fData.fCueDepth;
						currentDomain.doDepthCueing = fData.bDepthCueing;
						currentDomain.overDraw = fData.fOverdraw;
						currentDomain.vectorAngle = fData.fVectorAngle;
						currentDomain.doDomainBoundaryEdges = fData.doDomainBoundaryEdges;
						currentDomain.removeHiddenLines = true;
					}
					else {
						if (currentDomain.lineWidth > maxLineWidth) {
							maxLineWidth = currentDomain.lineWidth;
						}
					}
		

					if ((fData.bUseObjectColor && !currentDomain.overrideSettings)
						|| (currentDomain.useObjectColor && currentDomain.overrideSettings))
					{
						TShadingApproximation shadingApproximation;
						MCCOMErr test = current->fRenderable->GetShadingApproximation(shadingApproximation, domainIndex);
						currentDomain.color	= shadingApproximation.fDiffuseColor;
					}

				}
		}
		currentObjectIndex++;
	}


	TMCString255 progressMsg;

	for (uint32 pass = 0; pass < 2; pass++)
	{
		if (pass == 0)
		{
			progressMsg = "Count Lines";
			gShellUtilities->BeginProgress(progressMsg, &calcKey);

			//collect all the points we may want to render
			bJustCount = true;
		}
		else
		{
			progressMsg = "Calculate Lines";
			gShellUtilities->BeginProgress(progressMsg, &calcKey);
			bJustCount = false;
		}

		currentObjectIndex = 0;
		for (const TRenderableAndTfm* current = iter.First() ; (iter.More())&&(!bDoAbort); current = iter.Next())
		{
			theKind = current->fInstance->GetInstanceKind();
			if((theKind == 3) || (theKind == 4)) 
				continue;

			{//if this object is overriden to no effect jump out
				uint32 domainCount = overrideCache.cache[currentObjectIndex].GetElemCount();
				real32 sumLineEffect = 0;

				for (uint32 domainIndex = 0; domainIndex < domainCount; domainIndex++)
				{
					sumLineEffect += overrideCache.cache[currentObjectIndex].getDomainByIndex(domainIndex).lineEffect;
				}
				if (sumLineEffect == 0)
				{
					currentObjectIndex++;
					continue;
				}
			}

			L2G = current->fT ;		//In InstanceAndTransform fT is not a Translation as we may imagine
									//but a AFFINETRANSFORM cad Translation + Rotation
									//we get transformation from Local to Global system

			//Then we get our complete transformation from local to Camera Coordinate
			L2C = G2C*L2G;
			FillObjectCaches(&amesh, awayfromcamera, facenormals, facetedges
				, linemagnitude, drawedge, L2C, L2G, cameraTransform, 1, current->fInstance, bDoVMEdges
				, drawFacets);

			uint32 maxedges = amesh->fEdgeList.GetElemCount();
			if (!bDoVMEdges)
			{
				//for each edge, if it is along at least one facet that 
				//faces the camera, draw it.
				for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
				{
					int32 indexx = amesh->fEdgeList.fFaceIndices[edgeindex].x;
					int32 indexy = amesh->fEdgeList.fFaceIndices[edgeindex].y;

					switch (fData.lRenderStyle)
					{
					case rsWireFrame:
						{
							//it borders only once face
							if (indexy == -1)
							{
								//first face is towards camera or we're drawing backfaces
								if ((!awayfromcamera[indexx])||(fData.bDrawBackfaces))
								{
									drawedge[edgeindex] = true;
								}
							}
							//we draw all lines
							else if (fData.bFilterEdges == false)
							{
								//if either facet faces camera or we're drawing backfaces
								if ((!awayfromcamera[indexx])||(!awayfromcamera[indexy])||(fData.bDrawBackfaces))
								{
									drawedge[edgeindex] = true;
								}
							}
							else
							{
								//if both faces face camera or we're drawing backfaces
								if ((!awayfromcamera[indexx])||(!awayfromcamera[indexy])||(fData.bDrawBackfaces))
								{
									currentDomainIndex = overrideCache.cache[currentObjectIndex].getHighestSortedDomainIndex(
										amesh->fUVSpaceID[indexx], amesh->fUVSpaceID[indexy]); 
									ShadingDomainLineLevels sdSettings = 
										overrideCache.cache[currentObjectIndex].getDomainByIndex(currentDomainIndex);
									real32 fAngleThreshold = cos(sdSettings.vectorAngle / 360 * 6.283185307179586476925286766559 );

									real32 edgecos = facenormals[indexx] * facenormals[indexy];
									//edge is sharp
									if (edgecos < fAngleThreshold)
									{
										drawedge[edgeindex] = true;
									}
								}

							}
							if (fData.bSmartQuads)
							{
								FacetEdges& facetx = facetedges[indexx];

								//build our index of facet edges
								if (facetedges[indexx].lCurrentIndex < 3)
								{
									facetedges[indexx].lEdgeIndex[facetedges[indexx].lCurrentIndex++] = edgeindex;
								}
								else
								{
									uint32 bad = 6;
								}

								if (indexy > -1)
								{
									FacetEdges& facety = facetedges[indexy];
									if (facetedges[indexy].lCurrentIndex < 3)
									{
										facetedges[indexy].lEdgeIndex[facetedges[indexy].lCurrentIndex++] = edgeindex;
									}
									else
									{
										uint32 bad = 6;
									}

								}
							}//end smart quads
						}
						break;
					case rsToon:
						//is it an edge
						if ((indexy == -1)&&((!awayfromcamera[indexx])||(!fData.bBackfaceOptimization)))
						{
							drawedge[edgeindex] = true;
						}
						//is one face showing and the other turned away
						if ((indexy != -1)&&(awayfromcamera[indexx]!=awayfromcamera[indexy]))
						{
							drawedge[edgeindex] = true;
						}
						//is there a sharp edge between the facets and both face the camera
						if ((indexy != -1)
							&&(((!awayfromcamera[indexx])&&(!awayfromcamera[indexy]))||(!fData.bBackfaceOptimization))
							)
						{
							currentDomainIndex = overrideCache.cache[currentObjectIndex].getHighestSortedDomainIndex(
									amesh->fUVSpaceID[indexx], amesh->fUVSpaceID[indexy]); 
							ShadingDomainLineLevels sdSettings = 
								overrideCache.cache[currentObjectIndex].getDomainByIndex(currentDomainIndex);
							real32 fAngleThreshold = cos(sdSettings.vectorAngle / 360 * 6.283185307179586476925286766559 );

							real32 edgecos = facenormals[indexx] * facenormals[indexy];
							if (edgecos < fAngleThreshold)
							{
								drawedge[edgeindex] = true;
							}
							else if (sdSettings.doDomainBoundaryEdges && amesh->fUVSpaceID[indexx] != amesh->fUVSpaceID[indexy])
							{
								drawedge[edgeindex] = true;
							}
						}
						if ((!drawFacets[indexx] && indexy == -1)
							||(!drawFacets[indexx] && !drawFacets[indexy]))
						{
							drawedge[edgeindex] = false;
						}
						break;
					}//end switch

					//if the points are the same don't draw the line
					const TVector3& p1 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].x];
					const TVector3& p2 = amesh->fVertices[amesh->fEdgeList.fVertexIndices[edgeindex].y];
					//if the end points are the same, don't draw this edge
					if (p1 == p2)
					{
						drawedge[edgeindex] = false;
					}

				}//end edges

				if (fData.bSmartQuads)
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
							for (uint32 i = 0; i < 3; i++)
							{
								const FacetEdges& facetx = facetedges[indexx];
								if (facetx.lCurrentIndex > i)
								{
									if ((linemagnitude[facetx.lEdgeIndex[i]] > fMax)&&(facetx.lEdgeIndex[i]!=edgeindex))
										fMax = linemagnitude[facetx.lEdgeIndex[i]];
								}
								const FacetEdges& facety = facetedges[indexy];
								if (facety.lCurrentIndex > i)
								{
									if ((linemagnitude[facety.lEdgeIndex[i]] > fMax)&&(facety.lEdgeIndex[i]!=edgeindex))
										fMax = linemagnitude[facety.lEdgeIndex[i]];
								}
							}
							if (fMax * 1.01f < linemagnitude[edgeindex])
								drawedge[edgeindex] = false;
						}
						

					}
				}
			}//end do bDoVMEdges

			//shall we force silhouette edges
			if (fData.bSilhouetteEdges)
			{
				//for each edge, if it is along at least one facet that 
				//faces the camera, draw it.
				for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
				{
					int32 indexx = amesh->fEdgeList.fFaceIndices[edgeindex].x;
					int32 indexy = amesh->fEdgeList.fFaceIndices[edgeindex].y;
					if ((indexy != -1)&&(awayfromcamera[indexx]!=awayfromcamera[indexy]))
					{
						drawedge[edgeindex] = true;
					}
				}
			}
			linesDrawn = 0;

			//for each edge, draw it.
			for (uint32 edgeindex = 0; edgeindex < maxedges; edgeindex++)
			{
				gShellUtilities->SetProgressValue0to1(
					(static_cast<real32>(currentObjectIndex) + static_cast<real32>(edgeindex) / static_cast<real32>(maxedges))
					/ static_cast<real32>(maxobjects));		
				if (drawedge[edgeindex])
				{
					edgefacets = amesh->fEdgeList.fFaceIndices[edgeindex];
					DrawLine(amesh, amesh->fEdgeList.fVertexIndices[edgeindex].x, amesh->fEdgeList.fVertexIndices[edgeindex].y);
				}
				try
				{
					gShellThreadUtilities->YieldProcesses(1);
				}
				catch(TMCException& exception)
				{
					bDoAbort = true;
					edgeindex = maxedges;
				}

			}//end edges
			//TMCString255 alert;
			//alert.FromInt32(linesDrawn);
			//Alert(alert);
			currentObjectIndex++;
		}	
		gShellUtilities->EndProgress(calcKey);
		calcKey = NULL;
		if ((pass == 0)&&(!bDoAbort))
		{
			sparsePointSilo.SetCreateOnAccess(false);
			//allocate memory for all the points
			sparsePointSilo.BeginIter();
			POISilo* pointOfInterest = NULL;
			pointOfInterest = sparsePointSilo.Next();
			while (pointOfInterest != NULL) {
				pointOfInterest->points = 
						static_cast<PointOfInterest*>
						(MCcalloc(pointOfInterest->pointcount, sizeof(PointOfInterest)));
				pointOfInterest->pointcount = 0;

				pointOfInterest = sparsePointSilo.Next();
			}

			/*real32 chainLength = sparsePointSilo.GetAverageChainLength();
			TMCString255 mainMessage;
			mainMessage.FromReal32(chainLength, 6);
			Alert(mainMessage);*/
		}

	}//end pass

	//remove the hidden lines
	
	if ((fData.bRemoveHiddenLines)&&(!bDoAbort))
	{
		progressMsg = "Removing Hidden Lines";
		gShellUtilities->BeginProgress(progressMsg, &removeKey);
		gShellUtilities->SetProgressValue0to1(0);

		//clear out the stuff we don't need anymore
		facenormals.SetElemCount(0);
		facetedges.SetElemCount(0);
        linemagnitude.SetElemCount(0);
		drawedge.SetElemCount(0);

		//throw out points based on zbuffer
		currentObjectIndex = 0;
		for (const TRenderableAndTfm* current = iter.First() ; (iter.More())&&(!bDoAbort); current = iter.Next())
		{

			theKind = current->fInstance->GetInstanceKind();
			if((theKind == 3) || (theKind == 4)) 
				continue;

			L2G = current->fT ;		//In InstanceAndTransform fT is not a Translation as we may imagine
									//but a AFFINETRANSFORM cad Translation + Rotation
									//we get transformation from Local to Global system

			//Then we get our complete transformation from local to Camera Coordinate
			L2C = G2C*L2G;

			TMCArray<boolean> awayfromcamera;

			FillObjectCaches(&amesh, awayfromcamera, facenormals, facetedges
				, linemagnitude, drawedge, L2C, L2G, cameraTransform, 2, current->fInstance, bDoVMEdges
				, drawFacets);

			//spin through the faces and obscure the appropriate lines
			TFacet3D facet;
			uint32 maxfacets = amesh->fFacets.GetElemCount();
			for (currentFacetIndex = 0; currentFacetIndex < maxfacets; currentFacetIndex++ )
			{ 
				if ((fData.bBackfaceOptimization == false)||(!awayfromcamera[currentFacetIndex]))
				{
					uint32 domainIndex = amesh->fUVSpaceID[currentFacetIndex];
					ShadingDomainLineLevels& currentDomain = overrideCache.cache[currentObjectIndex].getDomainByIndex(domainIndex);
					if (currentDomain.removeHiddenLines) {

						uint32 behindcameratotal;
						
						facet.fVertices[0].fVertex  = amesh->fVertices[amesh->fFacets[currentFacetIndex].pt1];
						facet.fVertices[1].fVertex  = amesh->fVertices[amesh->fFacets[currentFacetIndex].pt2];
						facet.fVertices[2].fVertex  = amesh->fVertices[amesh->fFacets[currentFacetIndex].pt3];
						facet.fVertices[0].fNormal  = amesh->fNormals[amesh->fFacets[currentFacetIndex].pt1];
						facet.fVertices[1].fNormal  = amesh->fNormals[amesh->fFacets[currentFacetIndex].pt2];
						facet.fVertices[2].fNormal  = amesh->fNormals[amesh->fFacets[currentFacetIndex].pt3];
						behindcameratotal = (facet.fVertices[0].fNormal.z >= 0) 
							+ (facet.fVertices[1].fNormal.z >= 0) * 2 
							+ (facet.fVertices[2].fNormal.z >= 0) * 4;

						switch (behindcameratotal)
						{
						case 7:
							continue;
							break;
						case 6: //1 & 2 behind camera
							MovePointToCameraFront(amesh, amesh->fFacets[currentFacetIndex].pt2, amesh->fFacets[currentFacetIndex].pt1
								, facet.fVertices[1].fVertex);
							MovePointToCameraFront(amesh, amesh->fFacets[currentFacetIndex].pt3, amesh->fFacets[currentFacetIndex].pt1
								, facet.fVertices[2].fVertex);
							TestDepth(facet);			
							break;
						case 5: //0 & 2 behind camera
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt3, amesh->fFacets[currentFacetIndex].pt2
								, facet.fVertices[2].fVertex);
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt1, amesh->fFacets[currentFacetIndex].pt2
								, facet.fVertices[0].fVertex);
							TestDepth(facet);			
							break;
						case 4: //2 behind camera
							{
							TVector3 edge21;
							TVector3 edge20;
							
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt3, amesh->fFacets[currentFacetIndex].pt2
								, edge21);
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt3, amesh->fFacets[currentFacetIndex].pt1
								, edge20);

							facet.fVertices[2].fVertex = edge20;
							TestDepth(facet);
							facet.fVertices[0].fVertex = edge21;
							TestDepth(facet);
							}
							break;
						case 3://0 & 1 behind camera
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt2, amesh->fFacets[currentFacetIndex].pt3
								, facet.fVertices[1].fVertex);
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt1, amesh->fFacets[currentFacetIndex].pt3
								, facet.fVertices[0].fVertex);
							TestDepth(facet);			
							break;
						case 2://1 behind camera
							{
							TVector3 edge12;
							TVector3 edge10;
							
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt2, amesh->fFacets[currentFacetIndex].pt3
								, edge12);
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt2, amesh->fFacets[currentFacetIndex].pt1
								, edge10);

							facet.fVertices[1].fVertex = edge12;
							TestDepth(facet);
							facet.fVertices[2].fVertex = edge10;
							TestDepth(facet);
							}
							break;
						case 1://0 behind camera
							{
							TVector3 edge02;
							TVector3 edge01;
							
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt1, amesh->fFacets[currentFacetIndex].pt3
								, edge02);
							MovePointToCameraFront(amesh,amesh->fFacets[currentFacetIndex].pt1, amesh->fFacets[currentFacetIndex].pt2
								, edge01);

							facet.fVertices[0].fVertex = edge01;
							TestDepth(facet);
							facet.fVertices[1].fVertex = edge02;
							TestDepth(facet);
							}
							break; 
						case 0:
							TestDepth(facet);			
							break;


						}//end behindcameratotal switch
					}//end if remove hidden lines for this shading domain

					
				}
				if (currentFacetIndex % 100 == 0)
				{
					gShellUtilities->SetProgressValue0to1(
						(static_cast<real32>(currentObjectIndex) + static_cast<real32>(currentFacetIndex) / static_cast<real32>(maxfacets))
						/ static_cast<real32>(maxobjects));	
					try
					{
						gShellThreadUtilities->YieldProcesses(1);
					}
					catch(TMCException& exception)
					{
						bDoAbort = true;
						currentFacetIndex = maxfacets;
					}
				}

			}//end facet
			currentObjectIndex++;
		}	
		gShellUtilities->EndProgress(removeKey);
		removeKey = NULL;
	}//end remove hidden lines

}
#if VERSIONNUMBER >= 0x080000
MCCOMErr WireRenderer::PrepareDraw(
								const TMCPoint	&size,								// defines the size of the image to render
								const TBBox2D	&uvBox,
								const TBBox2D	&productionFrame,
								boolean			(*callback) (int16 vv, void* priv),
								void*			priv)

#elif VERSIONNUMBER >= 0x070000
MCCOMErr WireRenderer::PrepareDraw(
								const TMCPoint	&size,								// defines the size of the image to render
								const TBBox2D	&uvBox,
								const TBBox2D	&productionFrame,
								ISMPUtils*		smpUtils,
								boolean			(*callback) (int16 vv, void* priv),
								void*			priv)
#else
MCCOMErr WireRenderer::PrepareDraw (const TMCPoint& size,const TBBox2D& uvBox,const TBBox2D& productionFrame, boolean (*callback) (int16 vv, void *priv),void* priv)
#endif
{	
	bDoAbort = false;

	TMCPoint localsize = size * fData.iOversampling;



	fZoom[0] = real(localsize.x) / (uvBox.fMax[0] - uvBox.fMin[0]);
	fZoom[1] = real(localsize.y) / (uvBox.fMax[1] - uvBox.fMin[1]);

	fOffscrOffset[0] = real(localsize.x) * (-uvBox.fMin[0] / (uvBox.fMax[0] - uvBox.fMin[0]));
	fOffscrOffset[1] = real(localsize.y) * (uvBox.fMax[1] / (uvBox.fMax[1] - uvBox.fMin[1]));

	fImageArea.Set(0,0,localsize.x,localsize.y);


	buffersize = localsize.x * localsize.y;
	fUVMinMax = productionFrame;

	POISilo emptySilo;

	sparsePointSilo.SetCreateOnAccess(true);
	if (fData.lRenderStyle == rsToon) {
		sparsePointSilo.SetSparsity(1000 / fData.fLineWidth);
	}
	else {
		sparsePointSilo.SetSparsity(200 / fData.fLineWidth);
	}


	sparsePointSilo.SetElemCount(buffersize);
	sparsePointSilo.SetEmptyObject(emptySilo);

	fDepth = 8;				// For compatibility with Put32

	fTreeTop->QueryInterface(IID_I3DShTreeElement,(void**)&fTree);
	ThrowIfNil(fTree);
	
	fTree->BeginGetRenderables(fInstances);
	RenderScene(&fImageArea);

	if (!bDoAbort) {
#if VERSIONNUMBER >= 0x080000
		baserenderer->PrepareDraw(size, uvBox, productionFrame, callback, priv);
#elif VERSIONNUMBER >= 0x070000
		baserenderer->PrepareDraw(size, uvBox, productionFrame, smpUtils, callback, priv);
#else
		baserenderer->PrepareDraw(size, uvBox, productionFrame, callback, priv);
#endif
	}

	return MC_S_OK;
}

//must be thread safe
void WireRenderer::GetTileRenderer(I3DExTileRenderer** tileRenderer)
{
	TMCCountedPtr<WireTileRenderer> tile;
	WireTileRenderer::Create(&tile, sparsePointSilo);
	tile->fData = fData;
	tile->fImageArea = fImageArea;
	tile->overrideCache = &overrideCache;
	tile->fBackdrop = fBackdrop;
	tile->fCamera = fCamera;
	tile->fBackground  = fBackground;
	tile->fOffscrOffset = fOffscrOffset;
	tile->fZoom = fZoom;
	tile->fUVMinMax = fUVMinMax;

	baserenderer->GetTileRenderer(&tile->basetilerenderer);
	TMCCountedGetHelper<I3DExTileRenderer> result(tileRenderer);
	result = tile;

/*	baserenderer->GetTileRenderer(&basetilerenderer);
	TMCCountedGetHelper<I3DExTileRenderer> result(tileRenderer);
	result = this;
*/
}

//end must be thread safe

// drawing time is over, we have to delete temporary components

MCCOMErr WireRenderer::FinishDraw ()

{
	baserenderer->FinishDraw();
	fTree->EndGetRenderables();

	CleanUp();
	return MC_S_OK;
	
}	

void WireRenderer::TestDepth(const TFacet3D& facet)
{	

	TVector3 fDeltaScreenPoint[3];
	TVector2 fOneOverDeltaScreenPoint[3];

	//load the bounding box for our facet
	TBBox3D box;

	//load the bounding boxes with our lines
	TBBox2D lines[3];

	for(uint32 lineindex = 0; lineindex < 3; lineindex++)
	{
		box.AddPoint(facet.fVertices[lineindex].fVertex);
		lines[lineindex].AddPoint(TVector2(facet.fVertices[lineindexes[lineindex][0]].fVertex.x, facet.fVertices[lineindexes[lineindex][0]].fVertex.y));
		lines[lineindex].AddPoint(TVector2(facet.fVertices[lineindexes[lineindex][1]].fVertex.x, facet.fVertices[lineindexes[lineindex][1]].fVertex.y));
		fDeltaScreenPoint[lineindex] = facet.fVertices[lineindexes[lineindex][1]].fVertex - facet.fVertices[lineindexes[lineindex][0]].fVertex;
		if (fDeltaScreenPoint[lineindex].x != 0)
			fOneOverDeltaScreenPoint[lineindex].x = 1 / fDeltaScreenPoint[lineindex].x;
		if (fDeltaScreenPoint[lineindex].y != 0)
			fOneOverDeltaScreenPoint[lineindex].y = 1 / fDeltaScreenPoint[lineindex].y;
	}


	uint32 lineintersections[2];
	uint32 intersection = 0;
	TVector3 intersectPoints[2];

	//now spin through the points array and see if this polygon blocks any points
	
	int32 xlow =  max(floor(box.fMin.x - maxLineWidth), 0);
	int32 xhigh = min(ceil(box.fMax.x + maxLineWidth), fImageArea.right);
	int32 ylow =  max(floor(box.fMin.y - maxLineWidth), 0);
	int32 yhigh = min(ceil(box.fMax.y + maxLineWidth), fImageArea.bottom);
	for (int32 x = xlow; x < xhigh; x++)
	{	
		for (int32 y = ylow; y < yhigh; y++)
		{
			POISilo& silo = sparsePointSilo[bufferindex(x, y)];
			if (silo.pointcount > 0)
			{
				boolean hasactive = false;
				for (uint32 i = 0; i < silo.pointcount; i++)
				{
					PointOfInterest& point = silo.points[i]; 
					hasactive |= point.Active;

					//fast throw out if this point is out of the bounds of this facet
					//or closer to the camera
					if ((point.Active == false)//point has already been thrown out
						||(point.LinePoint.x < box.fMin.x)||(point.LinePoint.y < box.fMin.y)//out of lower bounds
						||(point.LinePoint.x > box.fMax.x)||(point.LinePoint.y > box.fMax.y)//out of upper bounds
						||((point.object == currentObjectIndex)&&((point.facets.x == currentFacetIndex)||(point.facets.y == currentFacetIndex)))//if this is the object/facet that created this point
						||(box.fMax.z <= 0)//the facet is behind the camera
						)
					{
					}
					else
					{


						//find two lines that are within the X range
						for(uint32 lineindex = 0, intersection = 0; (lineindex < 3)&&(intersection < 2); lineindex++)
						{
							if ((point.LinePoint.x >= lines[lineindex].fMin.x)&&(point.LinePoint.x <= lines[lineindex].fMax.x))
							{
								//intersects with this line
								lineintersections[intersection] = lineindex;
								intersection ++;
							}
						}

						boolean bDoMidPoint = true;
						//now that we have the two lines of intersection
						//do bilinear filtering to figure out where the Y is
						//and the depth
						for (uint32 intersection = 0; intersection < 2; intersection++)
						{
							uint32 line = lineintersections[intersection];
							const TVector3& pt1 = facet.fVertices[lineindexes[line][0]].fVertex;
							const TVector3& pt2 = facet.fVertices[lineindexes[line][1]].fVertex;

							if (lines[line].fMax.x == lines[line].fMin.x)
							{
								//vertical line
								if((point.LinePoint.y <= lines[line].fMax.y)&&(point.LinePoint.y >= lines[line].fMin.y))
								{
									//figure out where on the edge it is
									real32 u = (point.LinePoint.y - pt1.y) * fOneOverDeltaScreenPoint[line].y;
									real32 depth = pt1.z + u * fDeltaScreenPoint[line].z;
									if (depth > 0)
									{
										point.Active = (depth < point.LinePoint.z);
									}
								}
								//if we're on a vertical line we know the answer right away
								//get out of the loop and don't calc the midpoint
								bDoMidPoint = false;
								intersection = 2;
							}
							else
							{
									real32 u = (point.LinePoint.x - pt1.x) * fOneOverDeltaScreenPoint[line].x;
									intersectPoints[intersection] = pt1 + fDeltaScreenPoint[line] * u;

							}
						}//end intersections
						
						//now we have two end points on the facet edge
						//figure out where our point of interest is in there and get the depth
						if (bDoMidPoint)
						{
							if (intersectPoints[1].y == intersectPoints[0].y 
									&& intersectPoints[1].y == point.LinePoint.y)
							{
								real32 depth = min(intersectPoints[1].z, intersectPoints[0].z);
								if (depth > 0)
								{
									point.Active = (depth < point.LinePoint.z);
								}
							}
							else
							{

								real32 u = (point.LinePoint.y - intersectPoints[0].y) / (intersectPoints[1].y - intersectPoints[0].y);
								if ((u <= 1)&&(u >= 0))
								{
									real32 depth = intersectPoints[0].z + u * (intersectPoints[1].z - intersectPoints[0].z);
									
									if (depth > 0)
									{
										point.Active = (depth < point.LinePoint.z);
									}
								}//in u range
							}
						}//do midpoint
					}//not quick throw out

				}//end i
				if (hasactive == false)
				{
					MCfree(silo.points);
					silo.points = 0;
					silo.pointcount = 0;
				}
			}//has items
		}//end y
	}//end points

}

inline real32 WireRenderer::getBlend(const real32 distance, const real32 limit)
{
	return pow(1.0f - distance / (limit * fData.iOversampling), 2);
}

inline real32 magnitude(const TVector2& p1,const TVector2& p2)
{
	real32 x = p1.x - p2.x;
	real32 y = p1.y - p2.y;

	return (sqrt(static_cast<real32>(x * x + y * y)));
}

inline boolean WireRenderer::DistancePointLine
	(const TVector2& p, const TVector3& p1,const TVector3& p2
	, const ShadingDomainLineLevels sdSettings, real32& distance, TVector3& intersection, real32& limit, real32& factor)
{
	real32 lineMag = magnitude(TVector2(p1.x, p1.y), TVector2(p2.x, p2.y));
	real32 u;
	TVector3 screenCoordPoint;

	limit = sdSettings.lineWidth;
	u = (((p.x - p1.x) * (p2.x - p1.x)) + ((p.y - p1.y) * (p2.y - p1.y)))
		/ (lineMag * lineMag);


	//check to see if it's within linewidth pixels 
	//of a tip or the line
	if (u < 0.0f)
	{
		factor = 0.25f;
		intersection = p1;
		distance = magnitude(p, TVector2(p1.x, p1.y));

	}
	else if (u > 1.0f)
	{
		factor = 0.25f;
		intersection = p2;
		distance = magnitude(p, TVector2(p2.x, p2.y));

	}
	else
	{
		factor = 1.0f;
		intersection = p1 + u * (p2 - p1);

		distance = magnitude(p, TVector2(intersection.x, intersection.y));
	}
	
	//if (intersection.z < 0)
	//	return false;

	if (sdSettings.doDepthCueing)
	{
		limit *= pow(0.5f, ((1.0f / intersection.z) / sdSettings.depthCue));
	}


	return (distance <= limit * fData.iOversampling);

}

void WireRenderer::DrawLine(const TMCCountedPtr<FacetMesh>& amesh
							, const int32 p1index, const int32 p2index)
{

	//figure out what shading domain we are on
	if (amesh->fUVSpaceID[edgefacets.y] > 0)
	{
		currentDomainIndex = overrideCache.cache[currentObjectIndex].getHighestSortedDomainIndex(
				amesh->fUVSpaceID[edgefacets.x]
			, amesh->fUVSpaceID[edgefacets.y]); 
	}
	else
	{
		currentDomainIndex = amesh->fUVSpaceID[edgefacets.x];
	}
	ShadingDomainLineLevels sdSettings = 
		overrideCache.cache[currentObjectIndex].getDomainByIndex(currentDomainIndex);

	real32 lineWidthToUse = sdSettings.lineWidth;
	real32 overDrawToUse = sdSettings.overDraw;


	const TVector3& camerap1 = amesh->fNormals[p1index]; 
	const TVector3& camerap2 = amesh->fNormals[p2index]; 

	//if the line is completely behind the camera get out
	if ((camerap1.z > 0)&&(camerap2.z > 0))
		return;

	//p1 and p2 will be our boundaries
	//but we'll actually try to draw the line
	//from line width length extensions on each 
	//side 
	TVector3 p1 = amesh->fVertices[p1index]; 
	TVector3 p2 = amesh->fVertices[p2index];

	int32 outOfBounds = 0;

	real32 maximumUP1 = 1.0f;
	real32 maximumUP2 = 1.0f;

	//if p1 is behind the camera 
	if (camerap1.z > 0)
	{
		maximumUP1 = MovePointToCameraFront(amesh, p1index, p2index, p1);
	}

	//if p2 is behind the camera 
	if (camerap2.z > 0)
	{
		maximumUP2 = MovePointToCameraFront(amesh, p2index, p1index, p2);
	}

	//if the line is completely out of bounds get out
	//this check must be after the calls to move the point
	//to the front of the camera otherwise p1 and p2 have junk
	//in them.
	if (
		((p1.x + lineWidthToUse < 0)&&(p2.x + lineWidthToUse < 0))
		||
		((p1.x - lineWidthToUse >= fImageArea.right)&&(p2.x - lineWidthToUse >= fImageArea.right))
		||
		((p1.y + lineWidthToUse < 0)&&(p2.y + lineWidthToUse < 0))
		||
		((p1.y - lineWidthToUse >= fImageArea.bottom)&&(p2.y - lineWidthToUse >= fImageArea.bottom))
		)
	{
		return;
	}
	
	//crop any lines that are radically out of bounds
	if (!fImageArea.PtInside(TMCPoint(p1.x, p1.y)))
		outOfBounds += 1;
	if (!fImageArea.PtInside(TMCPoint(p2.x, p2.y)))
		outOfBounds += 2;
	switch (outOfBounds) {
		case 1: //p1 in, p2 out
			cropLine(amesh, p1index, p2index, maximumUP1, p1);
			break;
		case 2: //p2 in, p1 out
			cropLine(amesh, p2index, p1index, maximumUP2, p2);
			break;
		case 3: //both out, change mininum U to be in perspective of p1
			cropLine(amesh, p1index, p2index, (1.0f - maximumUP1), maximumUP2, p1, p2, lineWidthToUse);
			break;
	}


	//screen lines
	TVector3 overdrawslope(p2 - p1);
	if (overdrawslope.GetMagnitude() == 0) {
		//if after cropping they are same point
		//just leave
		return;
	}
	TVector3 overdrawp1(p1 - overdrawslope * overDrawToUse)
		, overdrawp2(p2 + overdrawslope * overDrawToUse);

	TVector2 innerp1(overdrawp1.x, overdrawp1.y), innerp2(overdrawp2.x, overdrawp2.y), slope(overdrawp2.x - overdrawp1.x, overdrawp2.y - overdrawp1.y);

	slope.Normalize(slope);

	int64 x,y,xi,yi,i,c,dx,dy;
	int32 aax, aay;
	real32 distance, limit, factor;
	TVector3 intersection;

	innerp1 -= slope * lineWidthToUse;
	innerp2 += slope * lineWidthToUse;
	


	linesDrawn++;

	x=innerp1.x;
	y=innerp1.y;


	if (innerp1.x < innerp2.x)
		xi=1;
	else 
		xi=-1;

	if (innerp1.y < innerp2.y)
		yi=1;
	else
		yi=-1;

	dx=fabs(real (innerp1.x-innerp2.x));
	dy=fabs(real (innerp1.y-innerp2.y));
	
	if (dy >= dx && dy != 0)
	{
		c=dy >> 1;
		int64 starti = 0;
		int64 stopi = dy;

		if (yi == 1)
		{
			if (y + dy > fImageArea.bottom)
			{
				stopi = fImageArea.bottom - y;
			}
			if (y < 0)
			{
				starti = 0 - y;
				y = y + (yi * starti);
				c = c + (dx * starti);
				int64 xsteps = c / dy;
				x = x + (xi * xsteps);
				c = c - (dy * xsteps);
			}
		}
		if (yi == -1)
		{
			if (y - dy < 0)
			{
				stopi = y;
			}
			if (y >= fImageArea.bottom)
			{
				starti = y - fImageArea.bottom;
				y = y + (yi * starti);
				c = c + (dx * starti);
				int64 xsteps = c / dy;
				x = x + (xi * xsteps);
				c = c - (dy * xsteps);
			}
		}


		for (i=starti;i<stopi;i++)
		{
	linesDrawn++;
			y=y+yi;
			c=c+dx;

			if (c >= dy)
			{
				c=c-dy;
				x=x+xi;
			}
			if ((y>=0)&&(y<fImageArea.bottom))
			{
				if ((x>=0)&&(x<fImageArea.right))
				{
					if (DistancePointLine(TVector2(x, y)
								, overdrawp1, overdrawp2
								, sdSettings, distance, intersection, limit, factor))
					{
						SavePoint(x, y, getBlend(distance, limit) * factor, intersection);
					}
				}
				//go up
				aax = 1;
				do
				{
					distance = 0;
					if (DistancePointLine(TVector2(x + aax, y)
							, overdrawp1, overdrawp2
							, sdSettings, distance, intersection, limit, factor))
					{
						SavePoint(x + aax, y, getBlend(distance, limit) * factor, intersection);
					}
					else
					{
						distance = lineWidthToUse + 1;
					}
					aax++;

				} while ((distance <= lineWidthToUse)&&(x + aax < fImageArea.right));
				//go down
				aax = -1;
				do
				{
					distance = 0;
					if (DistancePointLine(TVector2(x + aax, y)
							, overdrawp1, overdrawp2
							, sdSettings, distance, intersection, limit, factor))
					{
							SavePoint(x + aax, y, getBlend(distance, limit) * factor, intersection);
					}
					else
					{
						distance = lineWidthToUse + 1;
					}
					aax--;

				} while ((distance <= lineWidthToUse)&&(x + aax >= 0));
			}
		}//end for
	}	
	else if (dx > dy && dx != 0)
	{
		c=dx >> 1;

		int64 starti = 0;
		int64 stopi = dx;

		if (xi == 1) 
		{
			if (x + dx > fImageArea.right)
			{
				stopi = fImageArea.right - x;
			}
			if (x < 0) {
				starti = 0 - x;
				x = x + (xi * starti);
				c = c + (dy * starti);
				int64 ysteps = c / dx;
				y = y + (yi * ysteps);
				c = c - (dx * ysteps);
			}
		}
		if (xi == -1)
		{ 
			if (x - dx < 0)
			{
				stopi = x;
			}
			if (x >= fImageArea.right) 
			{
				starti = x - fImageArea.right;
				x = x + (xi * starti);
				c = c + (dy * starti);
				int64 ysteps = c / dx;
				y = y + (yi * ysteps);
				c = c - (dx * ysteps);
			}
		}

		for (i=starti;i<stopi;i++)
		{
	linesDrawn++;
			x=x+xi;
			c=c+dy;

			if (c >= dx)
			{
				c=c-dx;
				y=y+yi;
			}
			if ((x>=0)&&(x<fImageArea.right))
			{
				if ((y>=0)&&(y<fImageArea.bottom))
				{
					if (DistancePointLine(TVector2(x, y)
								, overdrawp1, overdrawp2
								, sdSettings, distance, intersection, limit, factor))
					{
							SavePoint(x, y, getBlend(distance, limit) * factor, intersection);
					}
				}
				//go up
				aay = 1;
				do
				{
					distance = 0;
					if (DistancePointLine(TVector2(x, y + aay)
							, overdrawp1, overdrawp2
							, sdSettings, distance, intersection, limit, factor))
					{
							SavePoint(x, y + aay, getBlend(distance, limit) * factor, intersection);
					}
					else
					{
						distance = lineWidthToUse + 1;
					}
					aay++;

				} while ((distance <= lineWidthToUse)&&(y + aay<fImageArea.bottom));
				//go down
				aay = -1;
				do
				{
					distance = 0;
					if (DistancePointLine(TVector2(x, y + aay)
							, overdrawp1, overdrawp2
							, sdSettings, distance, intersection, limit, factor))
					{
							SavePoint(x, y + aay, getBlend(distance, limit) * factor, intersection);
					}
					else
					{
						distance = lineWidthToUse + 1;
					}
					aay--;

				} while ((distance <= lineWidthToUse)&&(y + aay >= 0));
			}//end in x range
		}//end for
    }


}


void WireRenderer::SavePoint(int32 x, int32 y, real32 blend,const TVector3& intersection)
{
	if ((x>=0)&&(x<fImageArea.right)&&(y>=0)&&(y<fImageArea.bottom)&&(intersection.z > 0))
	{
		uint32 index = bufferindex(x, y);
		if (bJustCount)
		{
			sparsePointSilo[index].pointcount++;
		}
		else
		{
			PointOfInterest& newpoint = sparsePointSilo[index].points[sparsePointSilo[index].pointcount];
			newpoint.LinePoint = intersection;
			newpoint.blend = (blend 
				* overrideCache.cache[currentObjectIndex].getDomainByIndex(currentDomainIndex).lineEffect) * 65535;
			newpoint.Active = true;
			newpoint.object = currentObjectIndex;
			newpoint.domain = currentDomainIndex;
			newpoint.facets = edgefacets;
			sparsePointSilo[index].pointcount++;
		}

	}
}

//move the end point until it's just in front of the camera
//return how close along the line it is to the point in front
//of the camera
real32 WireRenderer::MovePointToCameraFront(
		const FacetMesh* amesh, const int32 behindindex, const int32 frontindex
		, TVector3& newPoint)
{
	const TVector3& behindpoint = amesh->fNormals[behindindex];
	const TVector3& frontpoint = amesh->fNormals[frontindex];

	TVector3 screenCoordPoint;
	real32 u;

	u =  (-BEHINDCAMERAFIX - behindpoint.z) / (frontpoint.z - behindpoint.z);
	screenCoordPoint = behindpoint + (frontpoint - behindpoint) * u;
	
	project3DTo2D(screenCoordPoint, newPoint);
	return 1.0f - u;

}

//crop the line to the screen coordinates
void WireRenderer::cropLine(
		const FacetMesh* amesh, const int32 outOfBoundsIndex, const int32 inBoundIndex, const real32 maximumU
		, TVector3& newPoint)
{
	const TVector3& outOfBoundsPoint = amesh->fNormals[outOfBoundsIndex];
	const TVector3& inBoundPoint = amesh->fNormals[inBoundIndex];

	//chop the U in half until we're totally in bounds and then add 10% until out of bounds
	real32 u = maximumU;
	TVector3 screenCoordPoint;

	do {
		u *= 0.5f;

		screenCoordPoint = inBoundPoint + (outOfBoundsPoint - inBoundPoint) * u;
		
		project3DTo2D(screenCoordPoint, newPoint);
	} while (!fImageArea.PtInside(TMCPoint(newPoint.x, newPoint.y)));
	do {
		u += 0.01f;

		screenCoordPoint = inBoundPoint + (outOfBoundsPoint - inBoundPoint) * u;
		
		project3DTo2D(screenCoordPoint, newPoint);
	} while (fImageArea.PtInside(TMCPoint(newPoint.x, newPoint.y)) && (u + 0.01f) < maximumU);

}

boolean spans(real32 val1, real32 val2, real32 span)
{
	return (val1 > span && val2 < span)
			||(val2 > span && val1 < span);
}

void WireRenderer::cropLine(
			const FacetMesh* amesh, const int32 p1Index, const int32 p2Index, real32 minimumU
			, real32 maximumU, TVector3& newP1, TVector3& newP2, real32 lineWidthToUse)
{

	const TVector3& p1 = amesh->fNormals[p1Index];
	const TVector3& p2 = amesh->fNormals[p2Index];

	TVector3 workingP1;
	TVector3 workingP2;
	TVector3 screenCoordPoint;
	real32* lowValue = NULL;
	real32* highValue = NULL;

	int32 Low;
	int32 High;
	real32 startingU, u;

	if (abs(p1.x - p2.x) > abs(p1.y - p2.y))
	{
		//x has the greatest range
		Low = fImageArea.left - lineWidthToUse;
		High = fImageArea.right + lineWidthToUse;

		lowValue = &newP1.x;
		highValue = &newP2.x;
		if (p1.x < p2.x)
		{
			workingP1 = p1;
			workingP2 = p2;
		}
		else
		{
			workingP1 = p2;
			workingP2 = p1;
			real32 tempU = 1.0f - maximumU;
			maximumU = 1.0f - minimumU;
			minimumU = tempU;
		}
		if (workingP1.x < 0 && workingP2.x > 0)
		{
			startingU = (0 - workingP1.x) / (workingP2.x - workingP1.x);
		}
		else
		{
			startingU = minimumU;
		}
	}
	else
	{
		//y has the greatest range
		Low = fImageArea.top - lineWidthToUse;
		High = fImageArea.bottom + lineWidthToUse;

		lowValue = &newP1.y;
		highValue = &newP2.y;
		if (p1.y < p2.y)
		{
			workingP1 = p1;
			workingP2 = p2;
		}
		else
		{
			workingP1 = p2;
			workingP2 = p1;
			real32 tempU = 1.0f - maximumU;
			maximumU = 1.0f - minimumU;
			minimumU = tempU;
		}
		if (workingP1.y < 0 && workingP2.y > 0)
		{
			startingU = (0 - workingP1.y) / (workingP2.y - workingP1.y);
		}
		else
		{
			startingU = minimumU;
		}

	}
	u = startingU;

	screenCoordPoint = workingP1 + (workingP2 - workingP1) * u;
	project3DTo2D(screenCoordPoint, newP1);

	newP2 = newP1;

	while ((*lowValue > Low && *lowValue < High) && (u - 0.01f) >= minimumU) {
		u -= .01f;

		screenCoordPoint = workingP1 + (workingP2 - workingP1) * u;
		project3DTo2D(screenCoordPoint, newP1);
	};

	u = startingU;
	while ((*highValue < High && *highValue > Low) && (u + 0.01f) <= maximumU) {
		u += .01f;

		screenCoordPoint = workingP1 + (workingP2 - workingP1) * u;
		project3DTo2D(screenCoordPoint, newP2);
	};



}


void WireRenderer::CleanUp()
{
	fTree=NULL;
	fInstances=NULL;
	sparsePointSilo.ArrayFree();
	overrideCache.CleanUp();
	if (calcKey)
		gShellUtilities->EndProgress(calcKey);
	if (removeKey)
		gShellUtilities->EndProgress(removeKey);
	if (drawKey)
		gShellUtilities->EndProgress(drawKey);
}

//WireTileRenderer

void WireTileRenderer::BlocCopy(const RTData* pixels)
{
	int32 toNextLine = pixels->rowBytes;

	if (fData.iOversampling == 1)
	{
		for (uint32 bufferx = fCurrentTileRect.left; bufferx < fCurrentTileRect.right; bufferx++)
		{
			for (uint32 buffery = fCurrentTileRect.top; buffery < fCurrentTileRect.bottom; buffery++)
			{

					int32 pixelindex = (buffery - fCurrentTileRect.top) * toNextLine + (bufferx - fCurrentTileRect.left);
					
					const POISilo& silo = sparsePointSilo[bufferindex(bufferx, buffery)];
					real32 sumBlend = 0;

					for (uint32 i = 0; i < silo.pointcount; i++)
					{
						const PointOfInterest& point = silo.points[i]; 
						if (point.Active)
						{
						
							real32 blend = static_cast<real32>(point.blend) / static_cast<real32>(65535);
							sumBlend += blend;
							
							if (blend > 0)
							{
								TMCColorRGBA& color = overrideCache->cache[point.object].getDomainByIndex(point.domain).color;
								pixels->red[pixelindex] = (1.0f - blend) * pixels->red[pixelindex] + blend * color.red * 255.0f;
								pixels->blue[pixelindex] = (1.0f - blend) * pixels->blue[pixelindex] + blend * color.blue * 255.0f;
								pixels->green[pixelindex] = (1.0f - blend) * pixels->green[pixelindex] + blend * color.green * 255.0f;
							}
						}//active point
					}
					if (pixels->alpha)
					{
						if (sumBlend > 1.0f)
							sumBlend = 1.0f;
						if (sumBlend < 0)
							sumBlend = 0;
						if (fData.bBaseRender && fData.alphaContent == acBoth)
						{
							pixels->alpha[pixelindex] = max(pixels->alpha[pixelindex], sumBlend * 255.0f);
						}
						else if (fData.alphaContent == acBoth || fData.alphaContent == acToonLines)
						{
							pixels->alpha[pixelindex] = sumBlend * 255.0f;
						}
					}
			}
		}
	}
	else
	{
		for (uint32 bufferx = fCurrentTileRect.left; bufferx < fCurrentTileRect.right; bufferx++)
		{
			for (uint32 buffery = fCurrentTileRect.top; buffery < fCurrentTileRect.bottom; buffery++)
			{
					TVector2 pt(bufferx, buffery);
					pt*=fData.iOversampling;

					int32 pixelindex = (buffery - fCurrentTileRect.top) * toNextLine + (bufferx - fCurrentTileRect.left);
					real32 sumBlend = 0;

					int32 minx = max(pt.x - fData.iOversampling, 0);
					int32 maxx = min(pt.x + fData.iOversampling, fImageArea.right);
					int32 miny = max(pt.y - fData.iOversampling, 0);
					int32 maxy = min(pt.y + fData.iOversampling, fImageArea.bottom);
					for (uint32 x = minx;x < maxx; x++)
					{
						for (uint32 y = miny; y < maxy; y++)
						{
							const POISilo& silo = sparsePointSilo[bufferindex(x, y)];
							for (uint32 i = 0; i < silo.pointcount; i++)
							{
								const PointOfInterest& point = silo.points[i]; 
								if (point.Active)
								{

									TVector2 pt2(x, y);
									
									real32 blend = static_cast<real32>(point.blend) / static_cast<real32>(65535);

									real32 dis = distance(pt, pt2);
									blend *= (1.0f - distance(pt, pt2) / fData.iOversampling) / (sqr(fData.iOversampling * 0.5f) * PI);
									sumBlend += blend;

									if (blend > 0)
									{
										TMCColorRGBA& color = overrideCache->cache[point.object].getDomainByIndex(point.domain).color;
										pixels->red[pixelindex] = (1.0f - blend) * pixels->red[pixelindex] + blend * color.red * 255.0f;
										pixels->blue[pixelindex] = (1.0f - blend) * pixels->blue[pixelindex] + blend * color.blue * 255.0f;
										pixels->green[pixelindex] = (1.0f - blend) * pixels->green[pixelindex] + blend * color.green * 255.0f;
									}
								}//active point
							}

						}
					}
					if (pixels->alpha)
					{
						if (sumBlend > 1.0f)
							sumBlend = 1.0f;
						if (sumBlend < 0)
							sumBlend = 0;
						if (fData.bBaseRender && fData.alphaContent == acBoth)
						{
							pixels->alpha[pixelindex] = max(pixels->alpha[pixelindex], sumBlend * 255.0f);
						}
						else if (fData.alphaContent == acBoth || fData.alphaContent == acToonLines)
						{
							pixels->alpha[pixelindex] = sumBlend * 255.0f;
						}
					}

				
			}
		}
	}//oversampling = 1
}

void WireTileRenderer::Init(const TMCRect& rect,const TBBox2D& uvBox)
{
	if (fData.bBaseRender)
	{
		basetilerenderer->Init(rect, uvBox);
	}
	fCurrentTileRect = rect;
	bSubTileDone = false;
}

boolean WireTileRenderer::GetNextSubTile(TMCRect& rect)
{
	if (fData.bBaseRender)
	{
		return basetilerenderer->GetNextSubTile(rect);
	}
	else if (bSubTileDone)
	{
		bSubTileDone = false;
		return false;
	}
	else
	{
		rect = fCurrentTileRect;
		return true;
	}
}

void WireTileRenderer::RenderSubTile(RTData& pixels)
{
	if (fData.bBaseRender)
	{
		basetilerenderer->RenderSubTile(pixels);
	}
	else
	{
		int32 toNextLine = pixels.rowBytes;
		for (uint32 bufferx = fCurrentTileRect.left; bufferx < fCurrentTileRect.right; bufferx++)
		{
			for (uint32 buffery = fCurrentTileRect.top; buffery < fCurrentTileRect.bottom; buffery++)
			{
				TMCColorRGBA color;
				TVector2 screenXY;
				screenXY[1] = (real)buffery * fData.iOversampling;
				screenXY[0] = (real)bufferx * fData.iOversampling;
				int32 pixelindex = (buffery - fCurrentTileRect.top) * toNextLine + (bufferx - fCurrentTileRect.left);
				if (fData.iOversampling == 1)
				{
					getBackgroundColor(screenXY,color);
					pixels.red[pixelindex] = color.red * 255.0f;
					pixels.blue[pixelindex] = color.blue * 255.0f;
					pixels.green[pixelindex] = color.green * 255.0f;
				}
				else
				{
					TMCColorRGBA coloraccumulator(0,0,0,0);
					real32 pixeltally = 0;
					int32 minx = max(screenXY.x, 0);
					int32 maxx = min(screenXY.x + fData.iOversampling, fImageArea.right);
					int32 miny = max(screenXY.y, 0);
					int32 maxy = min(screenXY.y + fData.iOversampling, fImageArea.bottom);
					for (uint32 x = minx;x < maxx; x++)
					{
						for (uint32 y = miny; y < maxy; y++)
						{
							screenXY[1] = (real)y;
							screenXY[0] = (real)x;

							getBackgroundColor(screenXY,color);
							coloraccumulator += color;
							pixeltally++;

						}
					}
					pixeltally = 1.0f / pixeltally;
					pixels.red[pixelindex] = coloraccumulator.red * pixeltally * 255.0f;
					pixels.blue[pixelindex] = coloraccumulator.blue * pixeltally * 255.0f;
					pixels.green[pixelindex] = coloraccumulator.green * pixeltally * 255.0f;

				}


			}
		}
		bSubTileDone = true;
	}
}

void WireTileRenderer::FinishRender(const RTData& pixels)
{
	//Be careful a slight difference exists between Camera Coord and Screen Coord. Basically,
	//both are the camera view, the only change is for Camera coord view axis is -z whereas for
	//screen coord view axis is y for further details see DataBase Overview in CookBook
	//For the following, Camera means Screen Coord
	if (fData.bBaseRender)
	{
		basetilerenderer->FinishRender(pixels);
	}
	BlocCopy(&pixels);
}

void WireTileRenderer::getBackgroundColor(const TVector2& theScreenUV, TMCColorRGBA& resultColor)

{

	TVector2	UVPoint;

	resultColor.R = (real)1.0f;
	resultColor.G = (real)1.0f;
	resultColor.B = (real)1.0f;

	UVPoint[0] = theScreenUV[0]-fOffscrOffset[0];
	UVPoint[0] /= fZoom[0];
	UVPoint[1] = -theScreenUV[1]+fOffscrOffset[1];
	UVPoint[1] /= fZoom[1];


	boolean fullAreaDone=false;

	if (fBackdrop)
	{
		fBackdrop->GetBackdropColor(UVPoint, fullAreaDone,fUVMinMax, resultColor);
	}
	else if (fBackground)
	{
		TVector3	rayOrigin, rayDirection;
		fCamera->CreateRay(&UVPoint, &rayOrigin, &rayDirection);
		fBackground->GetEnvironmentColor(rayDirection,fullAreaDone,resultColor);
	}

}
void WireRenderer::GetRenderStatistics(I3DRenderStatistics** renderstats)

{
	TMCCountedPtr<WireStatistics> stats;
	WireStatistics::Create(&stats);
	baserenderer->GetRenderStatistics(&stats->basestatistics);
	TMCCountedGetHelper<I3DRenderStatistics> result(renderstats);
	result = static_cast<I3DRenderStatistics*>(stats);

}


void WireStatistics::Clone(I3DRenderStatistics** clone)
{
	TMCCountedPtr<WireStatistics> stats;
	WireStatistics::Create(&stats);
	TMCCountedGetHelper<I3DRenderStatistics> result(clone);
	result = static_cast<I3DRenderStatistics*>(stats);
	stats->fData = this->fData;
	stats->basestatistics = this->basestatistics;
}

void* WireStatistics::GetDataBuffer()
{
	RealisticStatistics* baseData;
	baseData = (RealisticStatistics*)basestatistics->GetDataBuffer();
	fData.basedata.FacN = baseData->FacN;
	fData.basedata.RayN = baseData->RayN;
	return ((void*) &(fData));
}

int16 WireStatistics::GetResID()
{
	return 257;
}
