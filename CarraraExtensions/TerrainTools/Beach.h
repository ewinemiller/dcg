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
#ifndef __Beach__
#define __Beach__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "BeachDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "I3DShFacetMesh.h"
#include "IEdgeTree.h"
#include "MCClassArray.h"
#include "IShComponent.h"
#include "cTransformer.h"
#include "TerrainToolsCommon.h"
#include "DCGShaderHelper.h"
#include "InfinitePlaneEdgeHelper.h"
#include "DCGSharedCache.h"
#include "DCGBezier.h"

extern void InvertTransform(const TTransform3D& in, TTransform3D& out);

struct BeachPublicData
{
	real32 fWireSize;
	ActionNumber lFalloff;
	boolean bInvert;
	TVector2 fMinMax;
	TMCCountedPtr<IShParameterComponent> param;
	TMCString255 infiniteplane;
};

struct BeachKey {
	void* instance;
	TMCString255 infiniteplane;
	real currentTime;
	TTransform3D ipL2G;

	BeachKey()
	{
		this->instance = NULL;
	};

	void fill(void* instance, BeachPublicData fData, real currentTime, const TTransform3D& ipL2G)
	{
		this->instance = instance;
		this->infiniteplane = fData.infiniteplane;
		this->currentTime = currentTime;
		this->ipL2G = ipL2G;
	};

	boolean  operator==	(const BeachKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->infiniteplane == rhs.infiniteplane
			&& this->currentTime == rhs.currentTime
#if VERSIONNUMBER >= 0x060000
			&& this->ipL2G.IsEqual(rhs.ipL2G, 0));
#else
			&& this->ipL2G.fRotationAndScale != rhs.ipL2G.fRotationAndScale
			&& this->ipL2G.fTranslation != rhs.ipL2G.fTranslation);
#endif

	};
};

extern DCGSharedCache<TerrainToolsCache, ShadingIn, BeachKey> beachCache;

extern const MCGUID CLSID_Beach;
extern const MCGUID CLSID_BeachTerrain;
extern const MCGUID IID_Beach;

class Beach : public TBasicShader, public cTransformer, public IExStreamIO, public DCGShaderHelper
	, public InfinitePlaneEdgeHelper, public DCGBezier
	, public DCGSharedCacheClient<TerrainToolsCache, ShadingIn, BeachKey>
{
	public:
		Beach();
		~Beach();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(BeachPublicData); }
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
#if VERSIONNUMBER < 0x050000
		virtual int16 			MCCOMAPI GetResID() {return 327;};
#endif
		void fillElement(TerrainToolsCache& newElement, const ShadingIn& shadingIn);
		void emptyElement(TerrainToolsCache& oldElement);

	protected:
		BeachPublicData fData;
		ShadingFlags ChildFlags;
		EShaderOutput ChildOutput;
		TMCCountedPtr<I3DExShader> shader;
		edgeTreeCallLocalStorage callLocalStorage;

		void CopyDataFrom(const Beach* source);

		void LoadEdgeTree(TerrainToolsCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox, TMCArray<boolean> drawedge);
		void CheckNode(const TerrainToolsCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge);
		void BuildCache(TerrainToolsCache& instance, const ShadingIn& shadingIn);


	};

#endif