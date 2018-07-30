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
#include "AnimatedMixer.h"
#include "ShaderOps2DLL.h"
#include "InterfaceIDs.h"

#include "IMFPart.h"
#include "IMFResponder.h"
#include "MFPartMessages.h"

#include "IMCFile.h"
#include "IShUtilities.h"
#include "COMSafeUtilities.h"
#include "IShTextureMap.h"
#include "I3dShInstance.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"

#include "IChangeManagement.h"
#include "ISceneDocument.h"

const MCGUID CLSID_AnimatedMixer(R_CLSID_AnimatedMixer);


real AnimatedMixer::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	result = 0;
	real alpha = 1.0f;
	real animate = getControllingValue(shadingIn);

	int32 topFrame = childShaders.GetElemCount() - 1;
	if (topFrame >= 0) 
	{
		alpha = 0;

		uint32 currentFrame = floor(animate * static_cast<real32>(topFrame));
		real32 farAlong = animate * topFrame - currentFrame;

		real tempResult = 0;
		alpha += getChildValue(tempResult, shadingIn, currentFrame)  * (1.0f - farAlong);
		result += tempResult  * (1.0f - farAlong);

		if (farAlong > 0)
		{
			tempResult = 0;
			alpha += getChildValue(tempResult, shadingIn, currentFrame + 1) * farAlong;
			result += tempResult * farAlong;
		}
	}
	return alpha;
}

real AnimatedMixer::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	result.Set(0, 0, 0, 1.0f);
	real animate = getControllingValue(shadingIn);


	int32 topFrame = childShaders.GetElemCount() - 1;
	if (topFrame >= 0) 
	{
		alpha = 0;
		result.alpha = 0;
		uint32 currentFrame = floor(animate * static_cast<real32>(topFrame));
		real32 farAlong = animate * topFrame - currentFrame;

		TMCColorRGBA tempResult;
		alpha += getChildColor(tempResult, shadingIn, currentFrame) * (1.0f - farAlong);
		result += tempResult  * (1.0f - farAlong);
		if (farAlong > 0)
		{
			tempResult = TMCColorRGBA::kBlackNoAlpha;
			alpha += getChildColor(tempResult, shadingIn, currentFrame + 1) * farAlong;
			result += tempResult * farAlong;
		}	
	}
	return alpha;
}

real AnimatedMixer::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	real alpha = 1.0f;
	result.SetValues(0, 0, 0);
	real animate = getControllingValue(shadingIn);


	int32 topFrame = childShaders.GetElemCount() - 1;
	if (topFrame >= 0) 
	{
		alpha = 0;

		uint32 currentFrame = floor(animate * static_cast<real32>(topFrame));
		real32 farAlong = animate * topFrame - currentFrame;

		TVector3 tempResult;

		alpha += getChildVector(tempResult, shadingIn, currentFrame) * (1.0f - farAlong);
		result += tempResult * (1.0f - farAlong);
		if (farAlong > 0)
		{
			tempResult = TVector3::kZero;

			alpha += getChildVector(tempResult, shadingIn, currentFrame) * farAlong;
			result += tempResult * farAlong;
		}
	}
	return alpha;

}

