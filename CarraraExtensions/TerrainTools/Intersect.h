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
#ifndef __Intersect__
#define __Intersect__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "IntersectDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "I3DShFacetMesh.h"
#include "IEdgeTree.h"
#include "MCClassArray.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "TerrainToolsCommon.h"
#include "MCBezierPath.h"
#include "DCGShaderHelper.h"
#include "InfinitePlaneEdgeHelper.h"
#include "DCGSharedCache.h"
#include "DCGBezier.h"

extern void InvertTransform(const TTransform3D& in, TTransform3D& out);


struct IntersectPublicData
{
	real32 fWireSize;
	ActionNumber lFalloff;
	boolean bInvert;
	TVector2 fMinMax;
	TMCCountedPtr<IShParameterComponent> param;
	int32 lEffect;
	TMCDynamicString sList;
};

struct IntersectKey {
	void* instance;
	int32 lEffect;
	TMCDynamicString sList;
	real currentTime;
	TTransform3D ipL2G;

	IntersectKey()
	{
		this->instance = NULL;
		currentTime = FPOSINF;
	};

	void fill(void* instance, IntersectPublicData fData, real currentTime, const TTransform3D& ipL2G)
	{
		this->instance = instance;
		this->sList = fData.sList;
		this->lEffect = fData.lEffect;
		this->currentTime = currentTime;
		this->ipL2G = ipL2G;
	};

	boolean  operator==	(const IntersectKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->sList == rhs.sList
			&& this->lEffect == rhs.lEffect
			&& this->currentTime == rhs.currentTime
#if VERSIONNUMBER >= 0x060000
			&& this->ipL2G.IsEqual(rhs.ipL2G, 0));
#else
			&& this->ipL2G.fRotationAndScale != rhs.ipL2G.fRotationAndScale
			&& this->ipL2G.fTranslation != rhs.ipL2G.fTranslation);
#endif
	};
};

extern DCGSharedCache<TerrainToolsCache, ShadingIn, IntersectKey> intersectCache;


extern const MCGUID CLSID_Intersect;
extern const MCGUID IID_Intersect;

class Intersect : public TBasicShader, public cTransformer, public IExStreamIO, public DCGShaderHelper
	, public InfinitePlaneEdgeHelper, public DCGBezier
	, public DCGSharedCacheClient<TerrainToolsCache, ShadingIn, IntersectKey>
{
	public:
		Intersect();
		~Intersect();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(IntersectPublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput();
		virtual real			MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real			MCCOMAPI	GetVector(TVector3& result,ShadingIn& shadingIn);
		virtual void			MCCOMAPI	GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
		{
			result.Set(0.5f, 0.5f, 0.5f, 0);
		};
		virtual boolean			MCCOMAPI	WantsTransform						() { return false; };
		virtual MCCOMErr		MCCOMAPI	HandleEvent(MessageID message, IMFResponder* source, void* data);
#if VERSIONNUMBER < 0x050000
		virtual int16 			MCCOMAPI GetResID() {return 341;};
#endif
		// custom fall off code
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

		void fillElement(TerrainToolsCache& newElement, const ShadingIn& shadingIn);
		void emptyElement(TerrainToolsCache& oldElement);

	protected:
		IntersectPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
		edgeTreeCallLocalStorage callLocalStorage;

		void CopyDataFrom(const Intersect* source);

		void LoadEdgeTree(TerrainToolsCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox, TMCArray<boolean> drawedge);
		void CheckNode(const TerrainToolsCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge);
		void BuildCache(TerrainToolsCache& instance, const ShadingIn& shadingIn);


	};

#endif