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
#ifndef __Bevel__
#define __Bevel__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "BevelDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "I3DShFacetMesh.h"
#include "MCClassArray.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "IMeshTree.h"
#include "DCGSharedCache.h"

#include "DCGBezier.h"

#define SPACE_LOCAL 0
#define SPACE_GLOBAL 1

struct BevelPublicData
{
	real32 fWireSize;
	int32 iSpace;
	real32 fVectorAngle;
	ActionNumber lFalloff;
	boolean bEdgeInner;
	boolean bEdgeOuter;
	boolean bGrowsSafe;
	boolean bSoftenPoints;
	TMCCountedPtr<IShParameterComponent> param;	
};

struct BevelCache
{
	TMCArray<real32>	linemagnitude;
	TMCArray<TVector3>	edgenormal;
	TMCArray<TVector3>	pointnormal;
	TMCArray<boolean> usepoint;
	TMCArray<boolean> drawedge;
	IMeshTree* meshtree;
	TMCCountedPtr<FacetMesh> mesh;		
	BevelCache()
	{
		meshtree = NULL;
		pointnormal.SetZeroMem(true);
	};
	~BevelCache()
	{
		cleanup();
	};
	void cleanup()
	{
		linemagnitude.ArrayFree();
		edgenormal.ArrayFree();
		pointnormal.ArrayFree();
		usepoint.ArrayFree();
		drawedge.ArrayFree();
		if (meshtree != NULL)
		{
			delete meshtree;
			meshtree = NULL;
		}
		mesh = NULL;
	};

};

struct BevelKey {
	void* instance;
	int32 iSpace;
	real32 fVectorAngle;
	boolean bEdgeInner;
	boolean bEdgeOuter;
	boolean bGrowsSafe;
	real currentTime;

	BevelKey()
	{
		this->instance = NULL;
	};

	void fill(void* instance, BevelPublicData fData, real currentTime)
	{
		this->instance = instance;
		this->iSpace = fData.iSpace;
		this->fVectorAngle = fData.fVectorAngle;
		this->bEdgeInner = fData.bEdgeInner;
		this->bEdgeOuter = fData.bEdgeOuter;
		this->bGrowsSafe = fData.bGrowsSafe;
		this->currentTime = currentTime;
	};

	boolean  operator==	(const BevelKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->iSpace == rhs.iSpace
			&& this->fVectorAngle == rhs.fVectorAngle
			&& this->bEdgeInner == rhs.bEdgeInner
			&& this->bEdgeOuter == rhs.bEdgeOuter
			&& this->bGrowsSafe == rhs.bGrowsSafe
			&& this->currentTime == rhs.currentTime);
	};
};

extern DCGSharedCache<BevelCache, LightingContext, BevelKey> bevelCache;


enum FalloffStyle {foAbsolute = 1, foLinear = 2, foRadial = 3, foInvRadial = 4, foCustom = 5};

extern const MCGUID CLSID_Bevel;
extern const MCGUID IID_Bevel;

class Bevel : public TBasicShader, public cTransformer
	, public IExStreamIO , public DCGBezier
	, public DCGSharedCacheClient<BevelCache, LightingContext, BevelKey>
{
	public:
		Bevel();
		~Bevel();
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(BevelPublicData); }
		STANDARD_RELEASE;
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual boolean			MCCOMAPI	HasExplicitLightingModel			(const LightingContext& lightingContext);
		virtual MCCOMErr		MCCOMAPI	DoShade								(ShadingOut& result,ShadingIn& shadingIn);
#if VERSIONNUMBER >= 0x070000
		virtual MCCOMErr MCCOMAPI ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction);
#elif VERSIONNUMBER >= 0x050000
		virtual MCCOMErr		MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel);
#else
		virtual void			MCCOMAPI	ShadeAndLight						(LightingDetail& result,const LightingContext& lightingContext, I3DShShader* inShader);
#endif
#if VERSIONNUMBER >= 0x070000
		virtual void MCCOMAPI CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading);
#elif VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#else
		virtual void			CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading);
#endif
#if VERSIONNUMBER >= 0x050000
		virtual void			MCCOMAPI	CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#else
		virtual void			CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext);
#endif
		
		#if (VERSIONNUMBER >= 0x040000)
			virtual boolean			MCCOMAPI WantsTransform						() { return false; };
		#endif

		// IExStreamIO calls
		virtual MCCOMErr		MCCOMAPI	HandleEvent(MessageID message, IMFResponder* source, void* data);  
		virtual MCCOMErr		MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
		{	
			return DCGBezier::Read(stream, readUnknown, privData);
		};
		virtual MCCOMErr		MCCOMAPI Write(IShTokenStream* stream)
		{
			return DCGBezier::Write(stream);
		};
#if (VERSIONNUMBER >= 0x060000)
		virtual MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext) {return MC_S_OK;};
#endif
 		virtual uint32 			MCCOMAPI	AddRef();
		virtual MCCOMErr 		MCCOMAPI	QueryInterface(const MCIID &riid, void** ppvObj);
		virtual void			MCCOMAPI	Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter);
		
		void fillElement(BevelCache& newElement, const LightingContext& lightingContext);
		void emptyElement(BevelCache& oldElement);

	protected:
		BevelPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
		meshTreeCallLocalStorage callLocalStorage;
		
		#if (VERSIONNUMBER >= 0x030000)
			void InitCurve();
			void CopyDataFrom(const Bevel* source);
		#endif
		
		void BuildCache(BevelCache& instance, const LightingContext& lightingContext);
		void LoadMeshTree(BevelCache& instance, const TMCCountedPtr<FacetMesh>& amesh
				, const int32 Option, const TMCArray<TIndex3>& facetEdges, const TBBox3D& bbox);
		void CheckNode(const BevelCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point	, int32& lEdgeFound, real32& fAlongEdge
				, boolean& usePoint);
		void CheckMeshTreeItem(const BevelCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point, int32& lEdgeFound, real32& fAlongEdge
				, MeshTreeItem* Item, boolean& usePoint);
		TVector3 newnormal;
		TVector3 newgnormal;

		#if (VERSIONNUMBER >= 0x050000)
			I3DShLightingModel* defaultlightingmodel;
		#endif
	};

#endif