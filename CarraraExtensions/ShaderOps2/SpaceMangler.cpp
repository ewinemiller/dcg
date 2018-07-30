/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#include "SpaceMangler.h"
#include "ShaderOps2DLL.h"
#include "I3DShInstance.h"
#include "Transforms.h"
#include "I3DShTreeElement.h"

const MCGUID CLSID_SpaceMangler(R_CLSID_SpaceMangler);


void wrapValue(real32& val)
{
	if (val > 1.0f)
	{
		val = val - floor(val);
	}
	else
	if (val < 0)
	{
		val = 1.0f + val + ceil(val);
	}

}

void wrapUV(TVector2 &uv)
{
	wrapValue(uv.x);
	wrapValue(uv.y);
}


SpaceMangler::SpaceMangler() 
{
	fData.globalX = mangleGlobalX;
	fData.globalY = mangleGlobalY;
	fData.globalZ = mangleGlobalZ;
	fData.localX = mangleLocalX;
	fData.localY = mangleLocalY;
	fData.localZ = mangleLocalZ;
	fData.u = mangleU;
	fData.v = mangleV;

	fData.globalScaleX = fData.globalScaleY = fData.globalScaleZ = fData.localScaleX 
		= fData.localScaleY = fData.localScaleZ = fData.scaleU = fData.scaleV = 1.0f;
}

const ShadingIn SpaceMangler::getMappedShadingIn(const ShadingIn& shadingIn)
{
	ShadingIn retVal = shadingIn;

	retVal.fPoint.x = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.globalX)) * fData.globalScaleX;
	retVal.fPoint.y = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.globalY)) * fData.globalScaleY;
	retVal.fPoint.z = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.globalZ)) * fData.globalScaleZ;

	retVal.fPointLoc.x = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.localX)) * fData.localScaleX;
	retVal.fPointLoc.y = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.localY)) * fData.localScaleY;
	retVal.fPointLoc.z = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.localZ)) * fData.localScaleZ;

	retVal.fUV.x = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.u)) * fData.scaleU;
	retVal.fUV.y = getMappedValue(shadingIn, static_cast<MangleSelections>(fData.v)) * fData.scaleV;

	return retVal;
}


const real32 SpaceMangler::getMappedValue(const ShadingIn& shadingIn, const MangleSelections mangleSelection)
{
	switch (mangleSelection)
	{
	case mangleGlobalX:
		return shadingIn.fPoint.x;
		break;
	case mangleGlobalY:
		return shadingIn.fPoint.y;
		break;
	case mangleGlobalZ:
		return shadingIn.fPoint.z;
		break;
	case mangleLocalX:
		return shadingIn.fPointLoc.x;
		break;
	case mangleLocalY:
		return shadingIn.fPointLoc.y;
		break;
	case mangleLocalZ:
		return shadingIn.fPointLoc.z;
		break;
	case mangleU:
		return shadingIn.fUV.x;
		break;
	case mangleV:
		return shadingIn.fUV.y;
		break;
	case mangleAngleUX:
		return cos (shadingIn.fUV.x * 2 * PI);
		break;
	case mangleAngleUY:
		return sin (shadingIn.fUV.x * 2 * PI);
		break;
	case mangleAngleVX:
		return cos (shadingIn.fUV.y * 2 * PI);
		break;
	case mangleAngleVY:
		return sin (shadingIn.fUV.y * 2 * PI);
		break;
	case mangleConstant:
		return 1.0f;
		break;
	case mangleGlobalXToOrigin:
		{
			real32 retVal = shadingIn.fPoint.x;
			if (shadingIn.fInstance)
			{
				retVal -= shadingIn.fInstance->GetTreeElement()->GetGlobalTransform3D().fTranslation.x;
			}
			return retVal;
		}
		break;
	case mangleGlobalYToOrigin:
		{
			real32 retVal = shadingIn.fPoint.y;
			if (shadingIn.fInstance)
			{
				retVal -= shadingIn.fInstance->GetTreeElement()->GetGlobalTransform3D().fTranslation.y;
			}
			return retVal;
		}
		break;
	case mangleGlobalZToOrigin:
		{
			real32 retVal = shadingIn.fPoint.z;
			if (shadingIn.fInstance)
			{
				retVal -= shadingIn.fInstance->GetTreeElement()->GetGlobalTransform3D().fTranslation.z;
			}
			return retVal;
		}
		break;
	default:
		ThrowException(MC_E_UNEXPECTED, &TMCString255("getMappedValue found an unexpected selection."));
		break;
	}
}

real SpaceMangler::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn mangledShadingIn = getMappedShadingIn(shadingIn);
	return getChildValue(result, fullArea, mangledShadingIn);
}

real SpaceMangler::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn mangledShadingIn = getMappedShadingIn(shadingIn);
	return getChildColor(result, fullArea, mangledShadingIn);
}

MCCOMErr SpaceMangler::GetShadingFlags(ShadingFlags& theFlags) 
{
	if (extensionDataChangeDirty)
		getChildShaders();
	theFlags.CombineFlagsWith(combinedChildFlags);
	theFlags = getMappedShadingFlags(theFlags);
	return MC_S_OK;
};

real SpaceMangler::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn mangledShadingIn = getMappedShadingIn(shadingIn);
	ShadingFlags mangledFlags = getMappedShadingFlags(combinedChildFlags);
	return ParentOpHelper<SpaceManglerPublicData>::getChildVector(childShader, childOutput, mangledFlags, result, mangledShadingIn);
}

MCCOMErr SpaceMangler::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn mangledShadingIn = getMappedShadingIn(shadingIn);
	MCCOMErr retVal = doChildShade(result, mangledShadingIn);
	shadingIn.fCurrentCompletionMask = mangledShadingIn.fCurrentCompletionMask;
	return retVal;
};

void SpaceMangler::GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	ShadingIn mangledShadingIn = getMappedShadingIn(shadingIn);
	return ParentOpHelper<SpaceManglerPublicData>::getChildShaderApproxColor(childShader, childOutput, result, mangledShadingIn);
};

const boolean SpaceMangler::isSelected(const MangleSelections mangleSelection)
{
	if (fData.globalX == mangleSelection) return true;
	if (fData.globalY == mangleSelection) return true;
	if (fData.globalZ == mangleSelection) return true;
	if (fData.localX == mangleSelection) return true;
	if (fData.localY == mangleSelection) return true;
	if (fData.localZ == mangleSelection) return true;
	if (fData.u == mangleSelection) return true;
	if (fData.v == mangleSelection) return true;

	return false;
}


const ShadingFlags SpaceMangler::getMappedShadingFlags(const ShadingFlags& theFlags)
{
	ShadingFlags retval = theFlags;
	retval.fNeedsUV = (isSelected(mangleU) || isSelected(mangleV) || isSelected(mangleAngleUX) || isSelected(mangleAngleUY) || isSelected(mangleAngleVX) || isSelected(mangleAngleVY));
	retval.fNeedsPoint = (isSelected(mangleGlobalX) || isSelected(mangleGlobalY) || isSelected(mangleGlobalZ));
	retval.fNeedsPointLoc = (isSelected(mangleLocalX) || isSelected(mangleLocalY) || isSelected(mangleLocalZ));
	return retval;
}
