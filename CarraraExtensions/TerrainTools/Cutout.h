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
#ifndef __Cutout__
#define __Cutout__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "CutoutDef.h"
#include "BasicShader.h"
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

struct CutoutPublicData
{
	boolean bInvert;
	int32 lEffect;
	TMCDynamicString sList;

};


struct CutoutKey {
	void* instance;
	int32 lEffect;
	TMCDynamicString sList;
	real currentTime;
	TTransform3D ipL2G;

	CutoutKey()
	{
		this->instance = NULL;
	};

	void fill(void* instance, CutoutPublicData fData, real currentTime, const TTransform3D& ipL2G)
	{
		this->instance = instance;
		this->sList = fData.sList;
		this->lEffect = fData.lEffect;
		this->currentTime = currentTime;
		this->ipL2G = ipL2G;
	};

	boolean  operator==	(const CutoutKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->sList == rhs.sList
			&& this->lEffect == rhs.lEffect
			&& this->currentTime == rhs.currentTime
			&& this->ipL2G.IsEqual(rhs.ipL2G, 0));
	};
};


struct ShapeFileEdge
{
	TVector2 p1, p2;
};

class EdgeNode
{
private:
	TBBox2D range;
	TVector2 midPoint;
	EdgeNode *lowYlowX, *highYlowX, *lowYhighX, *highYhighX;
	uint32 elementCount;
	TMCArray<ShapeFileEdge> edges;

public:
	EdgeNode()
	{
		lowYlowX = NULL;
		highYlowX = NULL;
		lowYhighX = NULL;
		highYhighX = NULL;
		elementCount = 0;
	};

	~EdgeNode()
	{
		if (lowYlowX != NULL)
		{
			delete lowYlowX;
		}
		if (highYlowX != NULL)
		{
			delete highYlowX;
		}
		if (lowYhighX != NULL)
		{
			delete lowYhighX;
		}
		if (highYhighX != NULL)
		{
			delete highYhighX;
		}
	};

	void setRange(TVector2 min, TVector2 max)
	{
		range.SetMax(max);
		range.SetMin(min);
		range.GetCenter(midPoint);
	};

	void countEdge(const TVector2& p1, const TVector2& p2, uint32 maxDepth)
	{
		if (maxDepth > 0)
		{
			if (p1.y < midPoint.y && p2.y < midPoint.y)
			{
				if (p1.x < midPoint.x && p2.x < midPoint.x)
				{
					if (lowYlowX == NULL)
					{
						lowYlowX = new EdgeNode();
						lowYlowX->setRange(range.fMin, midPoint);
					}
					lowYlowX->countEdge(p1, p2, maxDepth - 1);
					return;
				}
				else if (p1.x > midPoint.x && p2.x > midPoint.x)
				{
					if (lowYhighX == NULL)
					{
						lowYhighX = new EdgeNode();
						lowYhighX->setRange(TVector2(midPoint.x, range.fMin.y), TVector2(midPoint.x, range.fMax.y));
					}
					lowYhighX->countEdge(p1, p2, maxDepth - 1);
					return;
				}
			}
			else if (p1.y > midPoint.y && p2.y > midPoint.y)
			{
				if (p1.x < midPoint.x && p2.x < midPoint.x)
				{
					if (highYlowX == NULL)
					{
						highYlowX = new EdgeNode();
						highYlowX->setRange(TVector2(range.fMin.x, midPoint.y), TVector2(range.fMin.x, midPoint.y));
					}
					highYlowX->countEdge(p1, p2, maxDepth - 1);
					return;
				}
				else if (p1.x > midPoint.x && p2.x > midPoint.x)
				{
					if (highYhighX == NULL)
					{
						highYhighX = new EdgeNode();
						highYhighX->setRange(midPoint, range.fMax);
					}
					highYhighX->countEdge(p1, p2, maxDepth - 1);
					return;
				} 
			}
		}
		elementCount++;
		
	};

	void allocNodes()
	{
		if (lowYlowX != NULL)
		{
			lowYlowX->allocNodes();
		}
		if (highYlowX != NULL)
		{
			highYlowX->allocNodes();
		}
		if (lowYhighX != NULL)
		{
			lowYhighX->allocNodes();
		}
		if (highYhighX != NULL)
		{
			highYhighX->allocNodes();
		}
		
		edges.SetElemCount(elementCount);
		elementCount = 0;

	};

	void clearNodes()
	{
		if (lowYlowX != NULL)
		{
			delete lowYlowX;
			lowYlowX = NULL;
		}
		if (highYlowX != NULL)
		{
			delete highYlowX;
			highYlowX = NULL;
		}
		if (lowYhighX != NULL)
		{
			delete lowYhighX;
			lowYhighX = NULL;
		}
		if (highYhighX != NULL)
		{
			delete highYhighX;
			highYhighX = NULL;
		}
		elementCount = 0;
		edges.SetElemCount(elementCount);
	};

	void addEdge(const TVector2& p1, const TVector2& p2, uint32 maxDepth)
	{
		if (maxDepth > 0)
		{
			if (p1.y < midPoint.y && p2.y < midPoint.y)
			{
				if (p1.x < midPoint.x && p2.x < midPoint.x)
				{
					lowYlowX->addEdge(p1, p2, maxDepth - 1);
					return;
				}
				else if (p1.x > midPoint.x && p2.x > midPoint.x)
				{
					lowYhighX->addEdge(p1, p2, maxDepth - 1);
					return;
				}
			}
			else if (p1.y > midPoint.y && p2.y > midPoint.y)
			{
				if (p1.x < midPoint.x && p2.x < midPoint.x)
				{
					highYlowX->addEdge(p1, p2, maxDepth - 1);
					return;
				}
				else if (p1.x > midPoint.x && p2.x > midPoint.x)
				{
					highYhighX->addEdge(p1, p2, maxDepth - 1);
					return;
				} 
			}
		}
	
		edges[elementCount].p1 = p1;
		edges[elementCount].p2 = p2;
		elementCount++;
	};

	uint32 tallyIntersect(const TVector2& p1, const TVector2& p2);

};


struct EdgeCache2D
{
	TBBox2D range;
	TVector2 targetPoint;
	EdgeNode edgeTree;

	EdgeCache2D()
	{
	};
	~EdgeCache2D()
	{
		cleanup();
	};
	void cleanup()
	{
		edgeTree.clearNodes();
	};

};

extern DCGSharedCache<EdgeCache2D, ShadingIn, CutoutKey> cutoutCache;

extern const MCGUID CLSID_Cutout;
extern const MCGUID IID_Cutout;

class Cutout : public TBasicShader, public cTransformer, public DCGShaderHelper
	, public InfinitePlaneEdgeHelper
	, public DCGSharedCacheClient<EdgeCache2D, ShadingIn, CutoutKey>
{
	public:
		Cutout();
		~Cutout();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(CutoutPublicData); }
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
		void fillElement(EdgeCache2D& newElement, const ShadingIn& shadingIn);
		void emptyElement(EdgeCache2D& oldElement);

	protected:
		CutoutPublicData fData;

		void fillEdgeTree(EdgeCache2D& instance, const TMCArray<ShapeFileEdge>& edges);
		void BuildCache(EdgeCache2D& instance, const ShadingIn& shadingIn);
	};

#endif