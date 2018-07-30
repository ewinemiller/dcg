/*  Wireframe Pro - plug-in for Carrara
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
#ifndef __WIREFRAME__
#define __WIREFRAME__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "WireframeDef.h"
#include "BasicShader.h"
#include "copyright.h"
#include "I3DShFacetMesh.h"
#include "IMeshTree.h"
#include "IEdgeTree.h"
#include "MCClassArray.h"
#include "DCGSharedCache.h"

#define SPACE_LOCAL 0
#define SPACE_GLOBAL 1

#define OPT_AUTO 0
#define OPT_OCTAL_TREE 1
#define OPT_XY_QUADRANT 2
#define OPT_XZ_QUADRANT 3
#define OPT_YZ_QUADRANT 4
#define OPT_LONG_SIDE 5
#define OPT_NONE 6

struct WireframePublicData
{
	real32 fWireSize;
	int32 iSpace;
	real32 fVectorAngle;
	boolean bSmartQuads;
	boolean bFilterEdges;
	ActionNumber lFalloff;
	boolean bInvert;
	boolean bVertexEdges;
};

struct WireframeCache
{
	//common
	TMCArray<boolean> drawedge;

	//if it's a normal mesh
	IMeshTree* meshtree;
	TMCArray<boolean> drawpoint;

	//if it's a sphere
	boolean isRayTraced;
	IEdgeTree* edgetree;
	TMCArray<real32>	oneoverlinemagnitude;
	TMCArray<TVector3>	p2minusp1;
	TMCCountedPtr<FacetMesh> mesh;		


	WireframeCache()
	{
		isRayTraced = false;
		meshtree = NULL;
		edgetree = NULL;
	};
	~WireframeCache()
	{
		cleanup();

	};

	void cleanup() {
		if (meshtree != NULL)
		{
			delete meshtree;
			meshtree = NULL;
		}
		if (edgetree != NULL)
		{
			delete edgetree;
			edgetree = NULL;
		}
		drawpoint.ArrayFree();
		oneoverlinemagnitude.ArrayFree();
		p2minusp1.ArrayFree();
		mesh = NULL;

	};

};

struct WireframeKey {
	void* instance;
	int32 iSpace;
	real32 fVectorAngle;
	boolean bSmartQuads;
	boolean bFilterEdges;
	boolean bVertexEdges;
	real currentTime;

	WireframeKey()
	{
		this->instance = NULL;
		currentTime = FPOSINF;

		iSpace = SPACE_LOCAL;
		fVectorAngle = 1;
		bSmartQuads = true;
		bFilterEdges = true;
		bVertexEdges = true;
	};

	void fill(void* instance, WireframePublicData fData, real currentTime)
	{
		this->instance = instance;
		this->iSpace = fData.iSpace;
		this->fVectorAngle = fData.fVectorAngle;
		this->bSmartQuads = fData.bSmartQuads;
		this->bFilterEdges = fData.bFilterEdges;
		this->bVertexEdges = fData.bVertexEdges;
		this->currentTime = currentTime;
	};

	boolean  operator==	(const WireframeKey& rhs) 
	{ 
		return (this->instance == rhs.instance
			&& this->iSpace == rhs.iSpace
			&& this->fVectorAngle == rhs.fVectorAngle
			&& this->bSmartQuads == rhs.bSmartQuads
			&& this->bFilterEdges == rhs.bFilterEdges
			&& this->bVertexEdges == rhs.bVertexEdges
			&& this->currentTime == rhs.currentTime);
	};
};

extern DCGSharedCache<WireframeCache, ShadingIn, WireframeKey> wireframeCache;

enum FalloffStyle {foAbsolute = 1, foLinear = 2, foRadial = 3};

extern const MCGUID CLSID_Wireframe;

class Wireframe : public TBasicShader, public DCGSharedCacheClient<WireframeCache, ShadingIn, WireframeKey> {
	public:
		Wireframe();
		~Wireframe();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(WireframePublicData); }
		virtual void*			MCCOMAPI    GetExtensionDataBuffer();
		virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged();
		virtual boolean			MCCOMAPI    IsEqualTo(I3DExShader* aShader);  
		virtual MCCOMErr		MCCOMAPI	GetShadingFlags(ShadingFlags& theFlags);
		virtual EShaderOutput	MCCOMAPI	GetImplementedOutput(); 
		virtual real			MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn);
		virtual real			MCCOMAPI	GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn
				, const boolean bDoVector, TVector3& normal);
		virtual real			MCCOMAPI	GetVector(TVector3& result,ShadingIn& shadingIn);
		virtual boolean			MCCOMAPI	WantsTransform() { return false; };

		void fillElement(WireframeCache& newElement, const ShadingIn& shadingIn);
		void emptyElement(WireframeCache& oldElement);
		virtual void			MCCOMAPI GetShaderApproxColor				(TMCColorRGBA& result,ShadingIn& shadingIn)
		{
			result.Set(0.5f, 0.5f, 0.5f, 0);
		};


	protected:
		WireframePublicData fData;
		edgeTreeCallLocalStorage edgeLocalStorage;
		meshTreeCallLocalStorage meshLocalStorage;

		void LoadMeshTree(WireframeCache& instance, const TMCCountedPtr<FacetMesh>& amesh
				, const int32 Option, const TMCArray<TIndex3>& facetEdges, const TBBox3D& bbox);
		void LoadEdgeTree(WireframeCache& instance, const TMCCountedPtr<FacetMesh>& amesh
							 , int32 Option, const TBBox3D& bbox);
		void CheckNode(const WireframeCache& instance, IEdgeTree* Node, real32& mindistance
				, const TVector3& point, int32& closestedge);
		void BuildCache(WireframeCache& instance, const ShadingIn& shadingIn);

		void CheckNode(const WireframeCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point	, int32& lEdgeFound, real32& fAlongEdge);
		void CheckMeshTreeItem(const WireframeCache& instance, IMeshTree* Node, real32& mindistance
				, const TVector3& point, int32& lEdgeFound, real32& fAlongEdge
				, MeshTreeItem* Item);


	};

#endif