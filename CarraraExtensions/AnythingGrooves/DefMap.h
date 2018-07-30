/*  Anything Grooves - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#ifndef __DEFMAP__
#define __DEFMAP__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include	"BasicModifiers.h"   
#include	"I3dExModifier.h"
#include	"I3dWireFrame.h"
#include	"DefMapDef.h"
#include	"PublicUtilities.h"
#include	"ISHTextureMap.h"
#include	"defpoint.h"
#include	"IMFDialogPart.h"
#include	"I3DShShader.h"
#include	"IMFPart.h"
#include	"IMFResponder.h"
#include "I3DShScene.h"
#include "cTransformer.h"
#include "DCGFacetMeshAccumulator.h"

//****** Globally Unique Id *************************************************

//MCDEFINE_GUID2 (CLSID_DefMap, R_CLSID_DefMap);
//MCDEFINE_GUID2 (IID_DefMap, R_IID_DefMap);
extern const MCGUID CLSID_DefMap;
extern const MCGUID IID_DefMap;

//***************************************************************************
#define SPLIT_DUMB 0
#define SPLIT_SHORT 1
#define SPLIT_SMART 2

struct DefMapData {
	real32	fStart, fStop;
	real32 fSmoothAngle;
	real32 lAnimate;
	int32  lU, lV;
	boolean bPreSmooth;
	TMCString255 MasterShader; 
	TMCString255 Warnings; 
	boolean bRefresh;
	boolean bAdaptiveMesh;
	boolean bEnabled;
	int32 lSplitMethod;
	boolean bEmptyZero;
	ActionNumber			lObjectPicker;
	};

// Deformer Object :
class DefMap : public TBasicDeformModifier, public cTransformer
{

public :  

	DefMap();
  
	// IUnknown Interface :
	STANDARD_RELEASE;
	virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	
  	// I3DExDataExchanger methods :
	virtual void*		MCCOMAPI GetExtensionDataBuffer();
	virtual int16		MCCOMAPI GetResID();
	virtual MCCOMErr	MCCOMAPI ExtensionDataChanged();
	virtual MCCOMErr	MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  
	virtual void		MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(DefMapData); }

	// I3DExDeformer methods :
	virtual void			MCCOMAPI SetBoundingBox		(const TBBox3D& bbox);
	virtual MCCOMErr		MCCOMAPI DeformPoint		(const TVector3& point, TVector3& result);
	virtual MCCOMErr		MCCOMAPI DeformFacetMesh	(real lod,FacetMesh* in, FacetMesh** outMesh);
	virtual MCCOMErr		MCCOMAPI DeformBBox			(const TBBox3D& in, TBBox3D& out);
	virtual TModifierFlags	MCCOMAPI GetModifierFlags	()	{return fFlags; }


	TMCString255 GetMasterShaderName();
private:
	TMCCountedPtr<I3DShScene> scene;

	void CopyData(DefMap* dest) const;

	DefMapData	fData;         // Deformer Data
	int32 AddOrder[3];
	DefPoint* Upt[3];
	real32 fU[3];
	int32 UOrder[3];
	bool bMidHigh;
	int32 mlBadFacets;
	TTransform3D	fTransform;      // Transformation Data (Local to Global and Global to Local)
	DCGFacetMeshAccumulator* accu;
	DefPointArray Deformed;
	TMCCountedPtr<I3DShShader> shader;
	TVector2 fUVInterval;
	ShadingFlags shadingflags;
	void ApplyDeformation(const TFacet3D* aFacet);
	void FillPoint(const real32& fFarAlong,TVertex3D& pt1,TVertex3D& pt2, TVertex3D& result);
	void SortPointsV(const TVertex3D pt[3], int32* VOrder);
	void BuildEndpointsU(const TVertex3D pt[3]);
	void ApplyDeformationToUStrip(int32 U
		, boolean bMidPoint);
	void AddQuad(DefPoint* bottomright, DefPoint* bottomleft
		 , DefPoint* topleft, DefPoint* topright);
	void AddFacet(DefPoint* pt1,DefPoint* pt2,DefPoint* pt3);
	void DoLeftTriangle(int32 U, boolean bMidPoint);
	void DoRightTriangle(int32 U, boolean bMidPoint);
	void DoTriangle(int32 U, DefPoint* pt1, DefPoint* pt2, DefPoint* pt3);
	void SetUpGrid();
	void DeformPoint (DefPoint* thePoint);
	void GetBlockSize(const uint32& u, const uint32& v, uint32& usize, uint32& vsize,const uint32& maxv);

	
	//special shapes
	void FillTwoPointLeftHand(int32 U, int32 V
			, DefPoint* bottomleft, DefPoint* topleft
			, boolean bSwingsUp, boolean bSwingsDown
			, boolean bOnTopEdge, boolean bOnBottomEdge);
	void FillSinglePointLeftHand(int32 U, int32 V
			, DefPoint* left, boolean bCloserToBottom
			, boolean bOnTopEdge, boolean bOnBottomEdge);
	void FillSinglePointRightHand(int32 U, int32 V
			, DefPoint* left, boolean bCloserToBottom
			, boolean bOnTopEdge, boolean bOnBottomEdge);
	void FillTwoPointRightHand(int32 U, int32 V
			, DefPoint* bottomright, DefPoint* topright
			, boolean bSwingsUp, boolean bSwingsDown
			, boolean bOnTopEdge, boolean bOnBottomEdge);
	void FillTwoPoint(int32 U, int32 V
			, DefPoint* right, DefPoint* left
			, boolean bRightOnTopEdge, boolean bRightOnBottomEdge
			, boolean bLeftOnTopEdge, boolean bLeftOnBottomEdge);
	void AddBlock(const uint32& u, const uint32& v
				,const  uint32& usize,const uint32& vsize);
	int AddPosition(int pt);
	boolean bFlat;
	ShadingIn shadingIn;

	};

#endif