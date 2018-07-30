/*  Ground Control - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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

#ifndef __GCShapefile__
#define __GCShapefile__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GCShapefileDef.h"
#include "BasicShader.h"
#include "IShComponent.h"
#include "MCCountedPtr.h"
#include "copyright.h"
#include "MCArray.h"
#include "TBBox.h"
#include "IShTokenStream.h"

struct GCShapefilePublicData
{
	boolean bInvert;
	TVector2 longitude;
	TVector2 latitude;
};

extern const MCGUID CLSID_GCShapefile;

enum spShapeType{NullShape = 0, PointShape = 1, PolyLine = 3, PolygonShape = 5
, MultiPoint = 8, PointZ = 11, PolyLineZ = 13, PolygonZ = 15
, MultiPointZ = 18, PointM = 21, PolyLineM = 23, PolygonM = 25
, MulitPointM = 28, MultiPatch = 31};

struct PolygonType
{
	TBBox2D range;
	int32 NumParts;
	int32 NumPoints;
	TMCArray<int32> Parts;
	TMCArray<TVector2> Points;
};
struct ShapeFileEdge
{
	TVector2 p1, p2;
};

struct ShapeFileData
{
	TBBox2D range;
	spShapeType shapeType;
	TMCArray<ShapeFileEdge> Edges;
};


class EdgeNode
{
private:
	TVector2 range;
	real32 midPoint;
	EdgeNode *left, *right;
	uint32 elementCount;
	TMCArray<ShapeFileEdge> edges;

public:
	EdgeNode()
	{
		left = NULL;
		right = NULL;
		elementCount = 0;
	};

	~EdgeNode()
	{
		if (left != NULL)
		{
			delete left;
		}
		if (right != NULL)
		{
			delete right;
		}
	};

	void setRange(real32 min, real32 max)
	{
		range.SetValues(min, max);
		midPoint = (min + max) * 0.5f;
	};

	void countEdge(const TVector2& p1, const TVector2& p2)
	{
		if (p1.y < midPoint && p2.y < midPoint)
		{
			if (left == NULL)
			{
				left = new EdgeNode();
				left->setRange(range.x, midPoint);
			}
			left->countEdge(p1, p2);
		}
		else if (p1.y > midPoint && p2.y > midPoint)
		{
			if (right == NULL)
			{
				right = new EdgeNode();
				right->setRange(midPoint, range.y);
			}
			right->countEdge(p1, p2);
		}
		else
		{
			elementCount++;
		}
	};

	void allocNodes()
	{
		
		if (left != NULL)
		{
			left->allocNodes();
		}
		if (right != NULL)
		{
			right->allocNodes();
		}
		edges.SetElemCount(elementCount);
		elementCount = 0;

	};

	void clearNodes()
	{
		if (left != NULL)
		{
			delete left;
			left = NULL;
		}
		if (right != NULL)
		{
			delete right;
			right = NULL;
		}
		elementCount = 0;
		edges.SetElemCount(elementCount);
	};

	void addEdge(const TVector2& p1, const TVector2& p2)
	{
		if (p1.y < midPoint && p2.y < midPoint)
		{
			left->addEdge(p1, p2);
		}
		else if (p1.y > midPoint && p2.y > midPoint)
		{
			right->addEdge(p1, p2);
		}
		else
		{
			edges[elementCount].p1 = p1;
			edges[elementCount].p2 = p2;
			elementCount++;
		}
	
	};

	uint32 tallyIntersect(const TVector2& p1, const TVector2& p2);

};

class GCShapefile: public TBasicShader, public IExStreamIO {
public:
	GCShapefile();
	~GCShapefile();
	virtual MCCOMErr MCCOMAPI QueryInterface (const MCIID &riid, void** ppvObj);
	virtual uint32 MCCOMAPI AddRef ();
	STANDARD_RELEASE;
	virtual int32 MCCOMAPI GetParamsBufferSize ()
		const { return sizeof(GCShapefilePublicData); }
	virtual void* MCCOMAPI GetExtensionDataBuffer();
	virtual MCCOMErr MCCOMAPI ExtensionDataChanged();
	virtual boolean MCCOMAPI IsEqualTo(I3DExShader* aShader);
	virtual MCCOMErr MCCOMAPI GetShadingFlags(ShadingFlags& theFlags);
	virtual EShaderOutput MCCOMAPI GetImplementedOutput();
	virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);

#if VERSIONNUMBER >= 0x030000
	virtual real MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
#else
	virtual MCCOMErr MCCOMAPI GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
#endif
	virtual boolean MCCOMAPI WantsTransform() { return false; };
	virtual void MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter);
	// IExStreamIO calls
	virtual MCCOMErr MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr MCCOMAPI Write(IShTokenStream* stream);
#if (VERSIONNUMBER >= 0x060000)
	virtual MCCOMErr MCCOMAPI FinishRead (IStreamContext* streamContext) {return MC_S_OK;};
#endif
	virtual int16 MCCOMAPI GetResID()
		{return 305;};

protected:
	GCShapefilePublicData fData;
	ShapeFileData shapefile;
	EdgeNode edgeTree;
	boolean bDirty;
	void readFile(TMCDynamicString sFilename);
	void CopyDataFrom(const GCShapefile* source);
	void fillEdgeTree();

};

#endif