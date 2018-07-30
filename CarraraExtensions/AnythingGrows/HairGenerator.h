/*  Anything Grows - plug-in for Carrara
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
#ifndef __HairGenerator__
#define __HairGenerator__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "publicutilities.h" 
#include "I3DShShader.h"
#include "cTransformer.h"
#include "I3DShScene.h"
#include "IMFPart.h"
#include "I3DShObject.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix44.h"
#include "Matrix33.h"
#include "../AnythingGrows/AnythingGrowsDll.h"
#include "dcgdebug.h"
#include "../AnythingGrows/DefPoint.h"
#include "I3dShTreeElement.h"
#include "I3DShInstance.h"
#include "IAnythingGrowsStrandGenerator.h"

#define MAX_SEGMENTS 20
#define MAX_SIDES 10

struct AnythingGrowsData {
	real32	fWidth, fLength;
	real32 lAnimate;
	TMCString255 MasterShader; 
	TMCString255 sWidthShader; 
	TMCString255 Warnings; 
	boolean bRefresh;
	boolean bEnabled;
	boolean bIncludeBaseMesh;
	boolean bShowStrands;
	boolean bAtLeastOneStrand;
	boolean bFaceCamera;
	boolean bBoneMode;
	int32 lSpace;
	int32 lNumHair;
	int32 lSeed; 
	int32 lSegments;
	int32 lStartEffects;
	int32 lSides;
	real32 fGravity;
	real32 fGravityX;
	real32 fGravityY;
	real32 fGravityZ;
	real32 fTaper;
	real32 fWiggle;
	real32 fStiffness;
	real32 fTipScale;
	real32 fCustomVector;
	TMCString255 sObjectName; 
	TMCString255 sReferenceObjectName; 
	TMCString255 sTipObjectName; 
	TMCString255 sStiffnessShader; 
	TMCString255 sXWiggleShader; 
	TMCString255 sYWiggleShader;  
	TMCString255 sZWiggleShader;  
	TMCString255 sTipScaleShader; 
	TMCString255 sTipTwistShader; 
	TMCString255 sXCustomVectorShader; 
	TMCString255 sYCustomVectorShader;  
	TMCString255 sZCustomVectorShader;  
	TMCString255 sDomain;  
	int32 lTaperMode;
	int32 lPreviewMode;
	int32 lStrandDistributionMode;
	int32 lUVMapping;
	int32 lVRunsAlong;
	ActionNumber lLengthPicker;
	ActionNumber lWidthPicker;
	ActionNumber lObjectPicker;
	ActionNumber lReferenceObjectPicker;
	ActionNumber lStiffnessPicker;
	ActionNumber lXWigglePicker;
	ActionNumber lYWigglePicker;
	ActionNumber lZWigglePicker;
	ActionNumber lTipObjectPicker;
	ActionNumber lTipScalePicker;
	ActionNumber lTipTwistPicker;
	ActionNumber lXCustomVectorPicker;
	ActionNumber lYCustomVectorPicker;
	ActionNumber lZCustomVectorPicker;
	ActionNumber lDomainPicker;
	}; 

enum TaperMode {tmPossible = 0, tmActual = 1};
enum PreviewMode {pmUltraLow = 0, pmLow = 1, pmMedium = 2, pmHigh = 3, pmFull = 4};
enum SpaceMode{smLocal = 0, smGlobal = 1};
enum StrandDistributionMode{sdFacet = 0, sdTotalSurface = 1, sdPatterned = 2};
enum UVMappingMode{uvBase = 0, uvStrand = 1};
enum GenerationMode {GenerateMesh = 0, GenerateStrands = 1}; 

class HairGenerator: public cTransformer
{
protected:
	HairGenerator()
	{
		fData.fWidth  = .25;
		fData.fLength = 4;
		fData.lAnimate = 0;
		fData.bRefresh = false;
		fData.bEnabled = true;
		fData.lLengthPicker = -1;
		fData.lWidthPicker = -1;
		fData.lNumHair = 100;
		fData.bIncludeBaseMesh = true;
		fData.bAtLeastOneStrand = true;
		fData.bBoneMode = false;
		fData.lSeed = 15000;
		fData.lSegments = 4;
		fData.lStartEffects = 1;
		fData.fGravity = .5;
		fData.fTaper = .8f;
		fData.fStiffness = 0;
		fData.fCustomVector = 0;
		fData.lObjectPicker = -1;
		fData.lReferenceObjectPicker = -1;
		fData.lTipObjectPicker = -1;
		fData.lStiffnessPicker = -1;
		fData.lXWigglePicker = -1; 
		fData.lYWigglePicker = -1;
		fData.lZWigglePicker = -1;
		fData.lXCustomVectorPicker = -1; 
		fData.lYCustomVectorPicker = -1;
		fData.lZCustomVectorPicker = -1;
		fData.lTipScalePicker = -1;
		fData.lTipTwistPicker = -1;
		fData.lDomainPicker = -1;
		fData.bFaceCamera = false;
		fData.lTaperMode = tmActual;
		fData.lStrandDistributionMode = sdTotalSurface;
		fData.lUVMapping = uvBase;
		fData.lVRunsAlong = tmActual;
		fData.lSpace = smLocal;
		fData.fWiggle = .1f;
		fData.lPreviewMode = pmFull;
		fData.bShowStrands = true;
		fData.fTipScale = 1.0f;
		fData.fGravityX = 0;
		fData.fGravityY = 0;
		fData.fGravityZ = -1;
		fData.lSides = 2;
		points = NULL;
		lStrandSpaceCount = 0;
		lBaseSpaceCount = 0;
		lTipSpaceCount = 0;
		lStrandSpaceID = 0;
	};

	~HairGenerator()
	{
		scene = NULL;
		shader = NULL;
		widthshader = NULL;
		tipmesh = NULL;
		stiffnessshader = NULL;
		xwiggleshader = NULL;
		ywiggleshader = NULL;
		zwiggleshader = NULL;
		tipscaleshader = NULL;
		tiptwistshader = NULL;
		xcustomvectorshader = NULL;
		ycustomvectorshader = NULL;
		zcustomvectorshader = NULL;
	};

	void FillPoint(const real32& fFarAlong, const TVertex3D& pt1, const TVertex3D& pt2, TVertex3D& result)
	{
		//do the UV
		result.fUV.x = pt1.fUV.x + fFarAlong * (pt2.fUV.x - pt1.fUV.x);
		result.fUV.y = pt1.fUV.y + fFarAlong * (pt2.fUV.y - pt1.fUV.y);


		//do the 3D point
		result.fVertex.x = pt1.fVertex.x + fFarAlong * (pt2.fVertex.x - pt1.fVertex.x);
		result.fVertex.y = pt1.fVertex.y + fFarAlong * (pt2.fVertex.y - pt1.fVertex.y);
		result.fVertex.z = pt1.fVertex.z + fFarAlong * (pt2.fVertex.z - pt1.fVertex.z);

		result.fNormal.x = pt1.fNormal.x + fFarAlong * (pt2.fNormal.x - pt1.fNormal.x);
		result.fNormal.y = pt1.fNormal.y + fFarAlong * (pt2.fNormal.y - pt1.fNormal.y);
		result.fNormal.z = pt1.fNormal.z + fFarAlong * (pt2.fNormal.z - pt1.fNormal.z);
	};

	void FillPoint(const real32& fFarAlong,const TVector3& pt1,const TVector3& pt2, TVector3& result)
	{
		result.x = pt1.x + fFarAlong * (pt2.x - pt1.x);
		result.y = pt1.y + fFarAlong * (pt2.y - pt1.y);
		result.z = pt1.z + fFarAlong * (pt2.z - pt1.z);
	};

	MCCOMErr GrowHair(real lod, FacetMesh* in, FacetMesh* originalMesh, FacetMesh** outMesh, int32 lSingleDomain, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode);

	void FillShaderList(TMCCountedPtr<IMFPart> popuppart); 
	void FillObjectList(I3DShObject* myobject, I3DShInstance* myinstance, IMFPart* popuppart);
	void FillDomainList(TMCCountedPtr<I3DShInstance> instance, TMCCountedPtr<IMFPart> popuppart);

	MCCOMErr ExtensionDataChanged()
	{ 

		if ((fData.lUVMapping == uvBase)||(fData.lSides == 2))
		{
			lVertexCount = fData.lSides;
		}
		else
		{
			lVertexCount = fData.lSides + 1;
		}


		if (VBySegment.GetElemCount() != fData.lSegments + 1)
		{
			VBySegment.SetElemCount(fData.lSegments + 1);
			for (uint32 i = 0; i < fData.lSegments + 1; i++)
			{
				VBySegment.SetElem(i, static_cast<real32>(i) / static_cast<real32>(fData.lSegments));
			}
		}


		if (fData.lLengthPicker == -2) 
		{
			fData.MasterShader = "";
			fData.lLengthPicker = -1;
		}
		else if (fData.lLengthPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lLengthPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.MasterShader = name;
							
				}
			fData.lLengthPicker = -1;
			}

		if (fData.lWidthPicker == -2) 
		{
			fData.sWidthShader = "";
			fData.lWidthPicker = -1;
		}
		else if (fData.lWidthPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lWidthPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sWidthShader = name;
							
				}
			fData.lWidthPicker = -1;
			}

		if (fData.lStiffnessPicker == -2) 
		{
			fData.sStiffnessShader = "";
			fData.lStiffnessPicker = -1;
		}
		else if (fData.lStiffnessPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lStiffnessPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sStiffnessShader = name;
							
				}
			fData.lStiffnessPicker = -1;
			}

		if (fData.lXWigglePicker == -2) 
		{
			fData.sXWiggleShader = "";
			fData.lXWigglePicker = -1;
		}
		else if (fData.lXWigglePicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lXWigglePicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sXWiggleShader = name;
							
				}
			fData.lXWigglePicker = -1;
			}

		if (fData.lYWigglePicker == -2) 
		{
			fData.sYWiggleShader = "";
			fData.lYWigglePicker = -1;
		}
		else if (fData.lYWigglePicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lYWigglePicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sYWiggleShader = name;
							
				}
			fData.lYWigglePicker = -1;
			}

		if (fData.lZWigglePicker == -2) 
		{
			fData.sZWiggleShader = "";
			fData.lZWigglePicker = -1;
		}
		else if (fData.lZWigglePicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lZWigglePicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sZWiggleShader = name;
							
				}
			fData.lZWigglePicker = -1;
			}

		if (fData.lXCustomVectorPicker == -2) 
		{
			fData.sXCustomVectorShader = "";
			fData.lXCustomVectorPicker = -1;
		}
		else if (fData.lXCustomVectorPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lXCustomVectorPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sXCustomVectorShader = name;
							
				}
			fData.lXCustomVectorPicker = -1;
			}

		if (fData.lYCustomVectorPicker == -2) 
		{
			fData.sYCustomVectorShader = "";
			fData.lYCustomVectorPicker = -1;
		}
		else if (fData.lYCustomVectorPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lYCustomVectorPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sYCustomVectorShader = name;
							
				}
			fData.lYCustomVectorPicker = -1;
			}

		if (fData.lZCustomVectorPicker == -2) 
		{
			fData.sZCustomVectorShader = "";
			fData.lZCustomVectorPicker = -1;
		}
		else if (fData.lZCustomVectorPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lZCustomVectorPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sZCustomVectorShader = name;
							
				}
			fData.lZCustomVectorPicker = -1;
			}

		if (fData.lTipScalePicker == -2) 
		{
			fData.sTipScaleShader = "";
			fData.lTipScalePicker = -1;
		}
		else if (fData.lTipScalePicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lTipScalePicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sTipScaleShader = name;
							
				}
			fData.lTipScalePicker = -1;
			}

		if (fData.lTipTwistPicker == -2) 
		{
			fData.sTipTwistShader = "";
			fData.lTipTwistPicker = -1;
		}
		else if (fData.lTipTwistPicker != -1)  {
			if (scene != NULL) {
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
				scene->GetMasterShaderByIndex(&mastershader, fData.lTipTwistPicker);
				mastershader->GetName(name);
				mastershader = NULL;
				fData.sTipTwistShader = name;
							
				}
			fData.lTipTwistPicker = -1;
			}

		if (fData.lObjectPicker == -2) 
		{
			fData.sObjectName = "";
			fData.lObjectPicker = -1;
		}
		else if (fData.lObjectPicker != -1)  {
			TMCString255 name;
			TMCCountedPtr<I3DShInstance> instance;
			TMCCountedPtr<I3DShTreeElement> tree;
			if (scene != NULL) {
				scene->GetInstanceByIndex(&instance, fData.lObjectPicker);
				instance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
				tree->GetName(name);
				tree = NULL;
				instance = NULL;
				fData.sObjectName = name;
				}
			fData.lObjectPicker = -1;
			}

		if (fData.lReferenceObjectPicker == -2) 
		{
			fData.sReferenceObjectName = "";
			fData.lReferenceObjectPicker = -1;
		}
		else if (fData.lReferenceObjectPicker != -1)  {
			TMCString255 name;
			TMCCountedPtr<I3DShInstance> instance;
			TMCCountedPtr<I3DShTreeElement> tree;
			if (scene != NULL) {
				scene->GetInstanceByIndex(&instance, fData.lReferenceObjectPicker);
				instance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
				tree->GetName(name);
				tree = NULL;
				instance = NULL;
				fData.sReferenceObjectName = name;
				}
			fData.lReferenceObjectPicker = -1;
			}

		if (fData.lTipObjectPicker == -2) 
		{
			fData.sTipObjectName = "";
			fData.lTipObjectPicker = -1;
		}
		else if (fData.lTipObjectPicker != -1)  {
			TMCString255 name;
			TMCCountedPtr<I3DShInstance> instance;
			TMCCountedPtr<I3DShTreeElement> tree;
			if (scene != NULL) {
				scene->GetInstanceByIndex(&instance, fData.lTipObjectPicker);
				instance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
				tree->GetName(name);
				tree = NULL;
				instance = NULL;
				fData.sTipObjectName = name;
				}
			fData.lTipObjectPicker = -1;
			}
		return MC_S_OK;

	};

	MCCOMErr  GetTreeElementByName(TMCCountedPtr<I3DShTreeElement>& treeElement, const TMCString& searchname);
	AnythingGrowsData	fData;         
	TTransform3D	fTransform;      // Transformation Data (Local to Global and Global to Local)
	TMCCountedPtr<I3DShScene> scene;
	uint32 lBaseSpaceCount;
	uint32 lTipSpaceCount;
	uint32 lStrandSpaceCount;
	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShInstance> refinstance;
private:
	TMCArray<real32> VBySegment;
	uint32 lStrandSpaceID;
	uint32 lCurrentVertex;
	uint32 lCurrentFacet;
	TVector3 gravitydirection;
	TVertex3D* points;
	TVertex3D startpoints[MAX_SIDES];
	TVector3 linecenter[MAX_SEGMENTS + 1];
	real32 fZRotation[MAX_SEGMENTS + 1];
	real32 fYRotation[MAX_SEGMENTS + 1];
	TVector3 directionarray[MAX_SEGMENTS + 1];
	TVector3 vec3CameraCenter;
	TVector3 vec3LocalCameraCenter;
	boolean bFaceCamera;
	//the number of vertices around a strand
	uint32 lVertexCount;

	TMCCountedPtr<I3DShShader> shader;
	TMCCountedPtr<I3DShShader> widthshader;
	TMCCountedPtr<I3DShShader> stiffnessshader;
	TMCCountedPtr<I3DShShader> xwiggleshader;
	TMCCountedPtr<I3DShShader> ywiggleshader;
	TMCCountedPtr<I3DShShader> zwiggleshader;
	TMCCountedPtr<I3DShShader> tipscaleshader;
	TMCCountedPtr<I3DShShader> tiptwistshader;
	TMCCountedPtr<I3DShShader> xcustomvectorshader;
	TMCCountedPtr<I3DShShader> ycustomvectorshader;
	TMCCountedPtr<I3DShShader> zcustomvectorshader;
	//ShadingFlags shadingflags;
	int16 fUVSpace;
	ShadingIn shadingIn;
	ShadingIn originalshadingIn;

	void FillShadingIn(const TVertex3D& thePoint, const TVertex3D& theOriginalPoint)
	{
		shadingIn.fCurrentCompletionMask = 0;
		shadingIn.fUV = thePoint.fUV;
		shadingIn.fPointLoc = thePoint.fVertex;
		shadingIn.fNormalLoc = thePoint.fNormal;
		//if (shadingflags.fNeedsNormal) {
			LocalToGlobalVector(fTransform, shadingIn.fNormalLoc, shadingIn.fGNormal);
		//	}
		//if (shadingflags.fNeedsPoint) {
			LocalToGlobal(fTransform, shadingIn.fPointLoc, shadingIn.fPoint);
		//	}
		shadingIn.fUVSpaceID = fUVSpace;
		shadingIn.fInstance = instance;

		originalshadingIn.fCurrentCompletionMask = 0;
		originalshadingIn.fUV = theOriginalPoint.fUV;
		originalshadingIn.fPointLoc = theOriginalPoint.fVertex;
		originalshadingIn.fNormalLoc = theOriginalPoint.fNormal;
		//if (shadingflags.fNeedsNormal) {
			LocalToGlobalVector(fTransform, originalshadingIn.fNormalLoc, originalshadingIn.fGNormal);
		//	}
		//if (shadingflags.fNeedsPoint) {
			LocalToGlobal(fTransform, originalshadingIn.fPointLoc, originalshadingIn.fPoint);
		//	}
		originalshadingIn.fUVSpaceID = fUVSpace;
		originalshadingIn.fInstance = refinstance;
	};
	
	real32 GetLength()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			shader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			shader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetWidth()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			widthshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			widthshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetStiffness()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			stiffnessshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			stiffnessshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};
	
	real32 GetXWiggle()
	{
		boolean fullarea = false;
		real32 fValue;
		xwiggleshader->GetValue(fValue, fullarea, shadingIn);
		return fValue;
	};

	real32 GetYWiggle()
	{
		boolean fullarea = false;
		real32 fValue;
		ywiggleshader->GetValue(fValue, fullarea, shadingIn);
		return fValue;
	};
	
	real32 GetZWiggle()
	{
		boolean fullarea = false;
		real32 fValue;
		zwiggleshader->GetValue(fValue, fullarea, shadingIn);
		return fValue;
	};

	real32 GetXCustomVector()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			xcustomvectorshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			xcustomvectorshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetYCustomVector()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			ycustomvectorshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			ycustomvectorshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetZCustomVector()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			zcustomvectorshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			zcustomvectorshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetTipScale()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			tipscaleshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			tipscaleshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	real32 GetTipTwist()
	{
		boolean fullarea = false;
		real32 fValue;
		if (fData.bBoneMode)
		{
			tiptwistshader->GetValue(fValue, fullarea, originalshadingIn);
		}
		else
		{
			tiptwistshader->GetValue(fValue, fullarea, shadingIn);
		}
		return fValue;
	};

	void AddStrands(const TFacet3D& aFacet, const TFacet3D& originalFacet, const uint32 lNumFacets, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode, uint32& currentStrand, uint32& currentVertex);
	void FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3, TTransform3D& Transform);
	TMCCountedPtr<FacetMesh> tipmesh;
	TMCCountedPtr<FacetMesh> workingmesh;
	void BuildTransform(TTransform3D& transform, const real32& fZRotation, const real32& fYRotation);
	void CleanUp();
	void GetShaders();

	real32 fU[3];
	DefPoint Upt[3];
	uint32 lV, lU;
	bool bMidHigh;
	TVector2 fUVInterval;
	DefPointArray Deformed;

};

#endif