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
#include "ShaderFilter.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "MFPartMessages.h"
#include "COMUtilities.h"
#include "IMCFile.h"
#include "IShUtilities.h"
#include "I3DShTerrainFilter.h"
#include "IMFTextPopupPart.h" 
#include "MFPartMessages.h"
#include "commessages.h"
#include "shadertypes.h"
#include "I3DShShader.h"
#include "DEMDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_ShaderFilter(R_CLSID_ShaderFilter);
#else
const MCGUID CLSID_ShaderFilter={R_CLSID_ShaderFilter};
#endif

const real32 ONE_THIRD = 1.0f / 3.0f;

inline uint32 VertexIndex(uint32 u, uint32 v, uint32 maxv)
{
	return u * (maxv) + v;
};

inline real32 sqr(real32 value)
{
	return value * value;
};


void ShaderFilter::Filter(TMCArray<real>& heightField, TVector2& heightBound,
								 const TIndex2& size, const TVector2& cellSize)

{
	TMCCountedPtr<I3DShMasterShader> mastershader;
	TMCCountedPtr<I3DShShader> shader;
	TMCCountedPtr<I3DShShader> effectshader;
	ShadingFlags shadingflags;
	TTransform3D	fTransform;      // Transformation Data (Local to Global and Global to Local)

	TMCCountedPtr<I3DShObject> object;
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShTerrainFilter> filter;
	TMCCountedPtr<I3DShPrimitiveComponent> primitivecomp;
	TMCCountedPtr<I3DShPrimitive> primitive;

	if (QueryInterface(IID_I3DShTerrainFilter, (void**)&filter) == MC_S_OK) 
	{
		primitivecomp = filter->GetTerrain();
		primitivecomp->GetPrimitive(&primitive);
		primitive->QueryInterface(IID_I3DShObject, reinterpret_cast<void**>(&object));
		object->GetScene(&scene);
		if (object->GetInstancesCount() > 0)
		{
			TMCCountedPtrArray<I3DShInstance> myinstances;
			TMCCountedPtr<I3DShTreeElement>	myfirsttree;
			TMCCountedPtr<I3DShInstance> myfirstinstance;
			object->GetInstanceArray(myinstances);
			myfirstinstance = myinstances[0];
			if (myfirstinstance->QueryInterface(IID_I3DShTreeElement, (void**)&myfirsttree)== MC_S_OK)  
			{
				fTransform = myfirsttree->GetGlobalTransform3D();
			}
		}


	}	

	if (!scene)
	{
		return;
	}
	scene->GetMasterShaderByName(&mastershader, fData.sShader);
	if (!mastershader) 
	{
		return;
	}
	mastershader->GetShader(&shader);
	shader->GetShadingFlags(shadingflags);

	scene->GetMasterShaderByName(&mastershader, fData.sEffectShader);
	if (mastershader != NULL) 
	{
		mastershader->GetShader(&effectshader);
		if (effectshader != NULL)
		{
			ShadingFlags effectshadingflags;
			effectshader->GetShadingFlags(effectshadingflags);
			shadingflags.CombineFlagsWith(effectshadingflags);
		}
	}

	TMCArray<real32> ULines;
	TMCArray<real32> VLines;
	TMCArray<real32> XLines;
	TMCArray<real32> YLines;

	uint32 u, v;

	real32 oneoverU = 1.0 / static_cast<real32>(size.y - 1);
	real32 oneoverV = 1.0 / static_cast<real32>(size.x - 1);
	uint32 lIndex;

	ULines.SetElemCount(size.x); 
	VLines.SetElemCount(size.y);
	XLines.SetElemCount(size.x);
	YLines.SetElemCount(size.y);
	const real32 fTerrainScaling = 1.0f/12000.0f;

	TVector2 localscale((size.x - 1) * cellSize.x, (size.y - 1) * cellSize.y); 

	localscale *= fTerrainScaling;

	//calculate XY and UV slices
	for (u = 0; u < size.y; u ++){
		ULines[u] = u * oneoverU;
		XLines[u] = -localscale.x * 0.5f + localscale.x * ULines[u];
		}

	for (v = 0; v < size.x; v++) {
		VLines[v] = 1.0f - v * oneoverV;
		YLines[v] = -localscale.y * 0.5f + localscale.y * VLines[v];
		}

	ShadingIn shadingIn;
	boolean fullarea = false;
	boolean bUseBuffer = false;
	shadingIn.fUVSpaceID = 0;
	shadingIn.fCurrentCompletionMask = 0;
	if (fData.bUseExistingHeight)
	{
		fData.fHeight = heightBound.y - heightBound.x;
	}

	int32 lPasses = 1;

	switch (fData.lOperator)
	{
	case opAdd:
		heightBound.y += fData.fHeight;
		break;
	case opMax:
		if (heightBound.y < fData.fHeight)
		{
			heightBound.y = fData.fHeight;
		}
		break;
	case opMin:
		if (heightBound.x > 0)
		{
			heightBound.x = 0;
		}
		break;
	case opMultiply:
		break;
	case opReplace:
		heightBound.x = 0;
		heightBound.y = fData.fHeight;
		break;
	case opSubtract:
		heightBound.x -= fData.fHeight;
		break;
	case opFilter:
		if (heightBound.y < fData.fHeight)
		{
			heightBound.y = fData.fHeight;
		}
		if (heightBound.x > 0)
		{
			heightBound.x = 0;
		}
		break;
	case opSmooth:
		lPasses = fData.lPasses;
		break;
	case opAverage:
		lPasses = fData.lPasses;
		break;
	}
	
	TMCArray<real> buffer;

	if ((fData.lOperator == opSmooth)||(shadingflags.fNeedsNormalLoc)||(shadingflags.fNeedsNormal))
	{
		buffer.SetElemCount(size.x * size.y);
		bUseBuffer = true;
	}

	for (uint32 lPass = 0; lPass < lPasses; lPass++)
	{
		for (u = 0; u < size.x; u ++)
		{

			for (v = 0; v < size.y; v++) 
			{
				lIndex = VertexIndex(v, u, size.y);
				real32 fValue;
				real32 fEffect = 1.0f;
				real32 temp;
				shadingIn.fUV.x = ULines[u];
				shadingIn.fUV.y = VLines[v];
				shadingIn.fPointLoc.SetValues(XLines[u], YLines[v], heightField[lIndex] * fTerrainScaling);
				if ((shadingflags.fNeedsNormalLoc)||(shadingflags.fNeedsNormal))
				{
					//figure out local normal
					if ((u > 0)&&(u < size.x - 1)&&(v > 0)&&(v < size.y - 1))
					{
						TVector3 up, down, left, right;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (up + down + left + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == 0)&&(v > 0)&&(v < size.y - 1))
					{
						TVector3 down, left, right;
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (down + left + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == size.x - 1)&&(v > 0)&&(v < size.y - 1))
					{
						TVector3 up, left, right;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (up + left + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u > 0)&&(u < size.x - 1)&&(v == size.y - 1))
					{
						TVector3 up, down, left;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);

						shadingIn.fNormalLoc = (up + down + left);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u > 0)&&(u < size.x - 1)&&(v == 0))
					{
						TVector3 up, down, right;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (up + down + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == 0)&&(v == 0))
					{
						TVector3 down, right;
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (down + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == size.x - 1)&&(v == 0))
					{
						TVector3 up, right;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						right.SetValues(0, heightField[VertexIndex(v + 1, u, size.y)] - heightField[lIndex], cellSize.y);

						shadingIn.fNormalLoc = (up + right);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == size.x - 1)&&(v ==  size.y - 1))
					{
						TVector3 up, left;
						up.SetValues(heightField[VertexIndex(v, u - 1, size.y)] - heightField[lIndex], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);
						shadingIn.fNormalLoc = (up + left);
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					else if ((u == 0)&&(v ==  size.y - 1))
					{
						TVector3 down, left;
						down.SetValues(heightField[lIndex] - heightField[VertexIndex(v, u + 1, size.y)], 0, cellSize.x);
						left.SetValues(0, heightField[lIndex] - heightField[VertexIndex(v - 1, u, size.y)], cellSize.y);

						shadingIn.fNormalLoc = down + left;
						shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
					if (shadingflags.fNeedsNormal)
					{
						//figure out global normal
						LocalToGlobalVector(fTransform, shadingIn.fNormalLoc, shadingIn.fGNormal);
					}
				}
				if (shadingflags.fNeedsPoint)
				{
					//fill the global point
					LocalToGlobal(fTransform, shadingIn.fPointLoc, shadingIn.fPoint);
				}
				shader->GetValue(fValue, fullarea, shadingIn);
				if (effectshader != NULL)
				{
					effectshader->GetValue(fEffect, fullarea, shadingIn);
				}

				real32 fResult = 0;

				switch (fData.lOperator)
				{
				case opAdd:
					fResult = heightField[lIndex] + fValue * fData.fHeight;
					break;
				case opMax:
					if (heightField[lIndex] < fValue * fData.fHeight)
					{
						fResult = fValue * fData.fHeight;
					}
					else
					{
						fResult = heightField[lIndex];
					}
					break;
				case opMin:
					if (heightField[lIndex] > fValue * fData.fHeight)
					{
						fResult = fValue * fData.fHeight;
					}
					else
					{
						fResult = heightField[lIndex];
					}
					break;
				case opMultiply:
					fResult = heightField[lIndex] * fValue;
					break;
				case opReplace:
					fResult = fValue * fData.fHeight;
					break;
				case opSubtract:
					fResult = heightField[lIndex] - fValue * fData.fHeight;
					break;
				case opFilter:
					fResult = heightField[lIndex] * (1.0f - fValue) + fValue * fValue * fData.fHeight;
					break;
				case opSmooth:
					if (fValue == 0)
					{
						fResult = heightField[lIndex];
					}
					else if ((u > 0)&&(u < size.x - 1)&&(v > 0)&&(v < size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)] 
							+ heightField[VertexIndex(v - 1, u, size.y)] 
							+ heightField[VertexIndex(v, u + 1, size.y)] 
							+ heightField[VertexIndex(v + 1, u, size.y)]) * 0.25f;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == 0)&&(v > 0)&&(v < size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v - 1, u, size.y)] 
							+ heightField[VertexIndex(v, u + 1, size.y)] 
							+ heightField[VertexIndex(v + 1, u, size.y)]) * ONE_THIRD;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == size.x - 1)&&(v > 0)&&(v < size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)] 
							+ heightField[VertexIndex(v - 1, u, size.y)]  
							+ heightField[VertexIndex(v + 1, u, size.y)]) * ONE_THIRD;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u > 0)&&(u < size.x - 1)&&(v == size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)] 
							+ heightField[VertexIndex(v - 1, u, size.y)] 
							+ heightField[VertexIndex(v, u + 1, size.y)]) * ONE_THIRD;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u > 0)&&(u < size.x - 1)&&(v == 0))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)]
							+ heightField[VertexIndex(v, u + 1, size.y)] 
							+ heightField[VertexIndex(v + 1, u, size.y)]) * ONE_THIRD;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == 0)&&(v == 0))
					{
						real32 avg = (heightField[VertexIndex(v, u + 1, size.y)] 
							+ heightField[VertexIndex(v + 1, u, size.y)]) * 0.50f;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == size.x - 1)&&(v == 0))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)] 
							+ heightField[VertexIndex(v + 1, u, size.y)]) * 0.50f;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == size.x - 1)&&(v ==  size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v, u - 1, size.y)] 
							+ heightField[VertexIndex(v - 1, u, size.y)]) * 0.50f;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					else if ((u == 0)&&(v ==  size.y - 1))
					{
						real32 avg = (heightField[VertexIndex(v - 1, u, size.y)] 
							+ heightField[VertexIndex(v, u + 1, size.y)]) * 0.50f;
						fResult = heightField[lIndex] * (1.0f - fValue) + fValue * avg;
					}
					break;
				case opAverage:
					temp = (heightField[lIndex] + fValue * fData.fHeight) * 0.50f;
					fResult = heightField[lIndex] * (1.0f - fEffect) + fEffect * temp;
					break;
				}//end op switch
				if (bUseBuffer)
				{
					buffer[lIndex] = fResult;
				}
				else
				{
					heightField[lIndex] = fResult;
				}
			}//end v
		}//end u
		
		if (bUseBuffer)
		{
			//blast buffer back to heightField		
			MCmemcpy(heightField.Pointer(0), buffer.Pointer(0), u * v * sizeof(real));
		}
	}//end passes

}


MCCOMErr ShaderFilter::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	if ((message == kMsg_CUIP_ComponentAttached)||((sourceID == 'RMSL')&&(message == EMFPartMessage::kMsg_PartValueChanged))) {
		TMCCountedPtr<I3DShObject> object;
		TMCCountedPtr<I3DShTreeElement> tree;
		TMCCountedPtr<I3DShTerrainFilter> filter;
		TMCCountedPtr<I3DShPrimitiveComponent> primitivecomp;
		TMCCountedPtr<I3DShPrimitive> primitive;

		if (QueryInterface(IID_I3DShTerrainFilter, (void**)&filter) == MC_S_OK) 
		{
			primitivecomp = filter->GetTerrain();
			primitivecomp->GetPrimitive(&primitive);
			primitive->QueryInterface(IID_I3DShObject, reinterpret_cast<void**>(&object));
			object->GetScene(&scene);

		}	
		TMCCountedPtr<IMFPart> popuppart;
		TMCCountedPtr<IMFTextPopupPart> popup;
		TMCCountedPtr<IMFPart> effectpopuppart;
		TMCCountedPtr<IMFTextPopupPart> effectpopup;
		TMCString255 name;
		if (sourceID == 0) {
			sourcePart->FindChildPartByID(&popuppart, 'SSHP');
			sourcePart->FindChildPartByID(&effectpopuppart, 'EFPI');
			}
		else {
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
			parentPart->FindChildPartByID(&popuppart, 'SSHP');
			parentPart->FindChildPartByID(&effectpopuppart, 'EFPI');
			}
		popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
		effectpopuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&effectpopup);
		popup->RemoveAll();
		effectpopup->RemoveAll();


		if (scene != NULL) {
			TMCCountedPtr<I3DShMasterShader> mastershader;
			uint32 numshaders = scene->GetMasterShadersCount();
			uint32 nummenu = 0;
			for (uint32 shaderindex = 0; shaderindex < numshaders; shaderindex++) {
				scene->GetMasterShaderByIndex(&mastershader, shaderindex);
				mastershader->GetName(name);
				mastershader = NULL;
				popup->AppendMenuItem(name);
				popup->SetItemActionNumber(nummenu, shaderindex);
				effectpopup->AppendMenuItem(name);
				effectpopup->SetItemActionNumber(nummenu, shaderindex);
				nummenu++;
				}
			}
		}
	
	return MC_S_OK;
}
MCCOMErr  ShaderFilter::ExtensionDataChanged()
{
	if (fData.lShaderPicker != -1)  
	{
		if (scene != NULL) 
		{
			TMCString255 name;
			TMCCountedPtr<I3DShMasterShader> mastershader;
			scene->GetMasterShaderByIndex(&mastershader, fData.lShaderPicker);
			mastershader->GetName(name);
			mastershader = NULL;
			fData.sShader = name;
						
		}
		fData.lShaderPicker = -1;
	}
	if (fData.lEffectShaderPicker != -1)  
	{
		if (scene != NULL) 
		{
			TMCString255 name;
			TMCCountedPtr<I3DShMasterShader> mastershader;
			scene->GetMasterShaderByIndex(&mastershader, fData.lEffectShaderPicker);
			mastershader->GetName(name);
			mastershader = NULL;
			fData.sEffectShader = name;
						
		}
		fData.lShaderPicker = -1;
	}

	return MC_S_OK;
}
