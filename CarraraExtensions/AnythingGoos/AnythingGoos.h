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
#ifndef __AnythingGoos__
#define __AnythingGoos__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "AnythingGoosDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "I3DShFacetMesh.h"
#include "IEdgeTree.h"
#include "MCClassArray.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "DCGSharedCache.h"

#if (VERSIONNUMBER >= 0x030000)
#include "DCGBezier.h"
#endif

#define SPACE_LOCAL 0
#define SPACE_GLOBAL 1

struct AnythingGoosPublicData
{
	real32 fWireSize;
	int32 iSpace;
	real32 fVectorAngle;
	ActionNumber lFalloff;
	boolean bInvert;
	boolean bEdgePerimeter;
	boolean bAdvancedEdgePerimeter;
	boolean bEdgeInner;
	boolean bEdgeOuter;
	TVector2 fMinMax;
	boolean bShadingDomains;
	real32 fallOffAngle;
	TMCCountedPtr<IShParameterComponent> param;	
};

struct AnythingGoosCache 
{
	IEdgeTree* edgetree;
	TMCArray<real32> oneoverlinemagnitude;
	TMCArray<TVector3>	p2minusp1;
	TMCArray<TVector3>	edgeNormal;
	TMCCountedPtr<FacetMesh> mesh;		

	AnythingGoosCache()
	{
		edgetree = NULL;
	};
	~AnythingGoosCache()
	{
		cleanup();
	};
	void cleanup()
	{
		if (edgetree != NULL)
		{
			delete edgetree;
			edgetree = NULL;
		}
		oneoverlinemagnitude.ArrayFree();
		p2minusp1.ArrayFree();
		edgeNormal.ArrayFree();
		mesh = NULL;
	};

};

struct AnythingGoosKey {
	void* instance;
	int32 iSpace;
	real32 fVectorAngle;
	boolean bEdgePerimeter;
	boolean bAdvancedEdgePerimeter;
	boolean bEdgeInner;
	boolean bEdgeOuter;
	boolean bShadingDomains;
	real currentTime;

	AnythingGoosKey()
	{
		this->instance = NULL;
	};

	void fill(void* instance, AnythingGoosPublicData fData, real currentTime)
	{
		this->instance = instance;
		this->iSpace = fData.iSpace;
		this->fVectorAngle = fData.fVectorAngle;
		this->bEdgePerimeter = fData.bEdgePerimeter;
		this->bAdvancedEdgePerimeter = fData.bAdvancedEdgePerimeter;
		this->bEdgeInner = fData.bEdgeInner;
		this->bEdgeOuter = fData.bEdgeOuter;
		this->bShadingDomains = fData.bShadingDomains;
		this->currentTime = currentTime;
	};

	boolean  operator==	(const AnythingGoosKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->iSpace == rhs.iSpace
			&& this->fVectorAngle == rhs.fVectorAngle
			&& this->bEdgePerimeter == rhs.bEdgePerimeter
			&& this->bAdvancedEdgePerimeter == rhs.bAdvancedEdgePerimeter
			&& this->bEdgeInner == rhs.bEdgeInner
			&& this->bEdgeOuter == rhs.bEdgeOuter
			&& this->bShadingDomains == rhs.bShadingDomains
			&& this->currentTime == rhs.currentTime);
	};
};

extern DCGSharedCache<AnythingGoosCache, ShadingIn, AnythingGoosKey> agoosCache;

enum FalloffStyle {foAbsolute = 1, foLinear = 2, foRadial = 3, foInvRadial = 4, foCustom = 5};

extern const MCGUID CLSID_AnythingGoos;
extern const MCGUID CLSID_AnythingGoosTerrain;
extern const MCGUID IID_AnythingGoos;

class AnythingGoos : public TBasicShader, public cTransformer
		, public DCGSharedCacheClient<AnythingGoosCache, ShadingIn, AnythingGoosKey>
#if (VERSIONNUMBER >= 0x030000)
	, public DCGBezier
#endif

#if (VERSIONNUMBER >= 0x030000)
	, public IExStreamIO 
#endif
{
	public:
		AnythingGoos();
		~AnythingGoos();
		STANDARD_RELEASE;
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 

		#if (VERSIONNUMBER >= 0x030000)
			virtual real			MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		#else
			virtual MCCOMErr		MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		#endif
		virtual void			MCCOMAPI GetShaderApproxColor				(TMCColorRGBA& result,ShadingIn& shadingIn)
		{
			result.Set(0.5f, 0.5f, 0.5f, 0);
		};
		
		#if (VERSIONNUMBER >= 0x040000)
			virtual boolean			MCCOMAPI WantsTransform						() { return false; };
		#endif
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(AnythingGoosPublicData); }

		// IExStreamIO calls
		#if (VERSIONNUMBER >= 0x030000)
			virtual MCCOMErr		MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  
			virtual MCCOMErr		MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
			{	
				return DCGBezier::Read(stream, readUnknown, privData);
			};
			virtual MCCOMErr		MCCOMAPI Write(IShTokenStream* stream)
			{
				return DCGBezier::Write(stream);
			};
 			virtual uint32 			MCCOMAPI AddRef();
			virtual MCCOMErr 		MCCOMAPI QueryInterface(const MCIID &riid, void** ppvObj);
			virtual void			MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter);
		#endif
#if (VERSIONNUMBER >= 0x060000)
			virtual MCCOMErr MCCOMAPI FinishRead		(IStreamContext* streamContext) {return MC_S_OK;};
#endif
		virtual int16 			MCCOMAPI	GetResID(){
#if (VERSIONNUMBER >= 0x030000)
				return 327;
#else
				return 328;
#endif
			};
private:
		void fillElement(AnythingGoosCache& newElement, const ShadingIn& shadingIn);
		void emptyElement(AnythingGoosCache& oldElement);

		real32 fFalloffAngleThreshold;


	protected:

		AnythingGoosPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
		edgeTreeCallLocalStorage callLocalStorage;

		#if (VERSIONNUMBER >= 0x030000)
			void InitCurve();
			void CopyDataFrom(const AnythingGoos* source);
		#endif
		
		void LoadEdgeTree(AnythingGoosCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox, const TMCArray<boolean>& drawedge);
		void CheckNode(const AnythingGoosCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge);
		void BuildCache(AnythingGoosCache& instance, const ShadingIn& shadingIn);


	};

#endif