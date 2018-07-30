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
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicFinalRenderer.h"
#include "WireRendererDef.h"

#include "MCColorRGB.h"
#include "Transforms.h"
#include "I3DShEnvironment.h"
#include "I3DShInstance.h"
#include "I3DShGroup.h"
#include "I3DShCamera.h"
#include "RealAffine3.h"
#include "I3DShTreeElement.h"
#include "IShRasterLayer.h"
#include "DCGUtil.h"
#include "WireRendererData.h"
#include "ToonOverrideCache.h"
#include "Index.h"
#include "MCCountedPtrHelper.h"
#include "DCGSparseArray.h"

#include <algorithm>

#if VERSIONNUMBER >= 0x050000
#include "MCSMPCountedObject.h"
#endif

// define the WireRenderer CLSID

extern const MCGUID CLSID_WireRenderer;
extern const MCGUID CLSID_ToonRenderer;


struct PointOfInterest
{
	TVector3 LinePoint;
	uint16 blend;
	//real32 depth;
	boolean Active;
	uint16 object;
	uint16 domain;
	TIndex2 facets;
};

struct POISilo
{
	POISilo(void)
	{
		pointcount = 0;
		points = NULL;
	};
	~POISilo(void)
	{
		if (points)
			MCfree(points);
	};
	uint16 pointcount;
	//uint16 pointindex;
	PointOfInterest* points;

};

class WireStatistics: public I3DRenderStatistics, public TMCSMPCountedObject
{
	public:
		static void Create(WireStatistics ** a)
		{
			 TMCCountedCreateHelper<WireStatistics> result(a);
			 result = new WireStatistics ();
		}
	protected:
		WireStatistics()
		{
			fData.basedata.FacN = 0;
			fData.basedata.RayN = 0;
		};
		~WireStatistics(){};

		WireRendererStatistics fData;

	public:
		TMCCountedPtr<I3DRenderStatistics> basestatistics;
		virtual MCErr    MCCOMAPI QueryInterface		(const MCIID& riid, void** ppvObj)	
		
		{
		
			return TMCSMPCountedObject::QueryInterface(riid,ppvObj); 
		
		}
		virtual uint32 MCCOMAPI AddRef()			{ return TMCSMPCountedObject::AddRef(); }
		virtual uint32 MCCOMAPI Release()			{return TMCSMPCountedObject::Release();};
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(WireRendererStatistics); }

		virtual void     MCCOMAPI Clone					(I3DRenderStatistics** clone);
		virtual void*	 MCCOMAPI GetDataBuffer			();
		virtual int16	 MCCOMAPI GetResID				();

};

class WireTileRenderer: public I3DExTileRenderer, public TMCSMPCountedObject 
{

public:
static void Create(WireTileRenderer ** a
				   , DCGSparseArray<POISilo>& rendererSparsePointSilo)
{
         TMCCountedCreateHelper<WireTileRenderer> result(a);
         result = new WireTileRenderer (rendererSparsePointSilo);
}

protected:
	WireTileRenderer(DCGSparseArray<POISilo>& rendererSparsePointSilo)
		: sparsePointSilo(rendererSparsePointSilo) {};
	~WireTileRenderer(){};

public:
		virtual MCErr    MCCOMAPI QueryInterface		(const MCIID& riid, void** ppvObj)	
		
		{
		
			return TMCSMPCountedObject::QueryInterface(riid,ppvObj); 
		
		}
		virtual uint32   MCCOMAPI AddRef()			{ return TMCSMPCountedObject::AddRef(); }
		virtual uint32 MCCOMAPI Release()			{return TMCSMPCountedObject::Release();};
		//STANDARD_RELEASE;
		// I3DExTileRenderer
		virtual void	MCCOMAPI Init					(const TMCRect& rect,const TBBox2D& uvBox);
		virtual boolean MCCOMAPI GetNextSubTile			(TMCRect& rect);
		virtual void	MCCOMAPI RenderSubTile			(RTData& pixels);
		virtual void	MCCOMAPI FinishRender			(const RTData& pixels);

		TMCCountedPtr<I3DExTileRenderer> basetilerenderer;
		TMCCountedPtr<I3DShCamera>			fCamera;		// Camera used for the rendering
		TMCCountedPtr<I3DShEnvironment>		fBackground;	// Environment image used for the rendering
		TMCCountedPtr<I3DShEnvironment>		fBackdrop;		// Screen Background image used for the rendering
		TVector2 			fZoom;			// Zoom factor Univers / OffScreen
		TVector2			fOffscrOffset;	// Indicates a distance between the offscreen and the bitmapArea
		TBBox2D				fUVMinMax;		// Dimension of backdrop
		WireRendererPublicData fData;
		DCGSparseArray<POISilo>& sparsePointSilo;	
		cOverrideCache* overrideCache;
		TMCRect				fImageArea;		// The full image obtained by the rendering process

private:
		void	BlocCopy	(const RTData* pixels);
		TMCRect				fCurrentTileRect;	// rectangle of the current tile
		TMCRect				fCurrentSubRect;
		boolean bSubTileDone;
		uint32 bufferindex(const uint32 x,const uint32 y)
		{
			return x * fImageArea.bottom + y;
		};

		void getBackgroundColor(const TVector2& theScreenUV, TMCColorRGBA& resultColor);
};

class WireRenderer : public TBasicFinalRenderer, public I3DExGBufferRenderer
{
	public:

		WireRenderer(RenderStyle renderStyle);
		~WireRenderer();

		// IUnknown methods
		virtual MCErr    MCCOMAPI QueryInterface		(const MCIID& riid, void** ppvObj)	
		
		{
			if (MCIsEqualIID(riid, IID_I3DExGBufferRenderer))
			{
				TMCCountedGetHelper<I3DExGBufferRenderer> result(ppvObj);
				result = (I3DExGBufferRenderer*)this;
				return MC_S_OK;
			}
			return TBasicFinalRenderer::QueryInterface(riid,ppvObj); 
		
		}
		virtual uint32   MCCOMAPI AddRef				()									{ return TBasicFinalRenderer::AddRef(); }
		//virtual uint32 MCCOMAPI Release()			{return TBasicFinalRenderer::Release();};
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(WireRendererPublicData); }

		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer	();
		virtual int16	 MCCOMAPI GetResID();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual MCCOMErr 			MCCOMAPI HandleEvent			(MessageID message, IMFResponder* source, void* data);

		// TBasicFinalRenderer methods
		virtual MCCOMErr MCCOMAPI SetTreeTop			(I3DShGroup* treeTop);
		virtual MCCOMErr MCCOMAPI SetCamera				(I3DShCamera* camera);
		virtual MCCOMErr MCCOMAPI SetEnvironment		(I3DShEnvironment* environment);
		virtual MCCOMErr MCCOMAPI SetAmbientLight		(const TMCColorRGB& ambiantColor);
		virtual MCCOMErr MCCOMAPI Dehydrate				(int16 level);
		virtual uint32   MCCOMAPI GetRenderingTime		();
		virtual MCCOMErr MCCOMAPI SetFieldRenderingData	(int32 useFieldRendering,int16 firstFrame);
		virtual void	 MCCOMAPI GetRenderStatistics	(I3DRenderStatistics** renderstats);

#if VERSIONNUMBER >= 0x080000
		virtual MCCOMErr MCCOMAPI PrepareDraw(
								const TMCPoint	&size,								// defines the size of the image to render
								const TBBox2D	&uvBox,
								const TBBox2D	&productionFrame,
								boolean			(*callback) (int16 vv, void* priv),
								void*			priv);
#elif VERSIONNUMBER >= 0x070000
		virtual MCCOMErr MCCOMAPI PrepareDraw(
								const TMCPoint	&size,								// defines the size of the image to render
								const TBBox2D	&uvBox,
								const TBBox2D	&productionFrame,
								ISMPUtils*		smpUtils,
								boolean			(*callback) (int16 vv, void* priv),
								void*			priv);
#else

		virtual MCCOMErr MCCOMAPI PrepareDraw(const TMCPoint& size,const TBBox2D& uvBox,const TBBox2D& productionFrame, boolean (*callback) (int16 vv, void* priv),void* priv);
#endif
		virtual MCCOMErr MCCOMAPI FinishDraw			();

		// called by the shell (renderingmodule) to tell the renderer to abort
		// this allow the renderer to cleanly abort and to release everything
		virtual MCCOMErr MCCOMAPI Abort					();
		virtual void	 MCCOMAPI GetTileRenderer		(I3DExTileRenderer** tileRenderer);

		//I3DExGBufferRenderer
#if VERSIONNUMBER >= 0x070000
		virtual MCCOMErr MCCOMAPI SetGBufferNeed		(const TMCArray<TGBufferInfo>& bufferInfos);
#else
		virtual MCCOMErr MCCOMAPI SetGBufferNeed		(uint32 bufferNbr,uint32* bufferID);
#endif
		virtual boolean				MCCOMAPI IsSMPAware				(){return true;};

	protected:
		real32 maxLineWidth;

		// Methods of the wire renderer.
		void	DrawLine	(const TMCCountedPtr<FacetMesh>& amesh, const int32 p1index, const int32 p2index);
		void	TestDepth	(const TFacet3D& facet);
		void	RenderScene	(const TMCRect* area);
		void	SavePoint	(int32 x, int32 y, real32 blend,const TVector3& intersection);
		void	FillObjectCaches(FacetMesh **inoutmesh
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
									, TMCArray<boolean> &drawFacet);
		// Datas of the renderer
		TMCCountedPtr<I3DShTreeElement>		fTree;
		TMCCountedPtr<I3DShGroup>			fTreeTop;
		const TRenderableAndTfmArray*		fInstances;
			
		TMCCountedPtr<IShRasterLayer>		fImage;			// Image to render
		TMCCountedPtr<I3DShCamera>			fCamera;		// Camera used for the rendering
		TMCCountedPtr<I3DShEnvironment>		fBackground;	// Environment image used for the rendering
		TMCCountedPtr<I3DShEnvironment>		fBackdrop;		// Screen Background image used for the rendering
		TMCCountedPtr<I3DShEnvironment>		fAtmosphere;	// Atmosphere effect used for the rendering

		TVector2 			fZoom;			// Zoom factor Univers / OffScreen
		TMCRect				fImageArea;		// The full image obtained by the rendering process
		TMCRect				fDrawingArea;	// An area, used by the renderer, which can be a tile of the fImageArea
		int16				fDepth;			// Number of bits to code the color
		TVector2			fOffscrOffset;	// Indicates a distance between the offscreen and the bitmapArea
		TBBox2D				fUVMinMax;		// Dimension of backdrop

		TMCRect				fCurrentTileRect;	// rectangle of the current tile
		TMCCountedPtr<I3DExFinalRenderer> baserenderer;
		TMCCountedPtr<I3DExTileRenderer> basetilerenderer;
		TMCCountedPtr<I3DExGBufferRenderer> basegbufferrenderer;
		WireRendererPublicData fData;
		//TVector3			fCameraPos;

		real32 getBlend(const real32 distance, const real32 limit);

		boolean DistancePointLine(const TVector2& p, const TVector3& p1,const TVector3& p2
			, const ShadingDomainLineLevels sdSettings, real32& distance, TVector3& intersection, real32& limit, real32& factor);
		real32 MovePointToCameraFront(
			const FacetMesh* amesh, const int32 behindindex, const int32 frontindex
			, TVector3& newPoint);
		void cropLine(
			const FacetMesh* amesh, const int32 outOfBoundsIndex, const int32 inBoundIndex, const real32 maximumU
			, TVector3& newPoint);
		void cropLine(
			const FacetMesh* amesh, const int32 p1Index, const int32 p2Index, real32 minimumU
			, real32 maximumU, TVector3& newP1, TVector3& newP2, real32 lineWidthToUse);

		uint16 currentObjectIndex;
		uint32 currentDomainIndex;
		uint32 currentFacetIndex;
		TIndex2 edgefacets;
		TMCCountedPtr<IMCUnknown> calcKey;
		TMCCountedPtr<IMCUnknown> removeKey;
		TMCCountedPtr<IMCUnknown> drawKey;

		boolean bDoAbort;
		void CleanUp();

		DCGSparseArray<POISilo> sparsePointSilo;
		boolean bJustCount;
		uint32 buffersize;
		cOverrideCache overrideCache;
		uint32 bufferindex(const uint32 x,const uint32 y)
		{
			return x * fImageArea.bottom + y;
		};

		void project3DTo2D(const TVector3& screenCoordPoint, TVector3& screen2DPoint)
		{
			TVector2 screenVertex;
			real		zOut;

			fCamera->Project3DTo2D(&screenCoordPoint, &screenVertex, &zOut);

			//Before displaying each pixel, we have to apply Zoom transformation, and to center
			screen2DPoint.x = fOffscrOffset.x + (screenVertex.x * fZoom.x) ;
			screen2DPoint.y = fOffscrOffset.y - (screenVertex.y * fZoom.y) ;
			screen2DPoint.z = 1/(-screenCoordPoint.z); //We just get the z coordinate
		};


		//used for calculating direction to camera
		TStandardCameraInfo cameraInfo;
		//only filled if camera is orthogonal
		TVector3 cameraDirection;
		int32 linesDrawn;
		boolean dontDraw;

};
