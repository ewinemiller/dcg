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
#include "CameraMapping.h"
#include "ShaderOps2DLL.h"
#include "MCCountedPtrHelper.h"
#include "I3DShInstance.h"
#include "MCClassArray.h"
#include "ISceneDocument.h"
#include "I3DRenderingModule.h"
#include "I3dShCamera.h"
#include "I3dShTreeElement.h"
#include "RenderTypes.h"
#include "I3dShScene.h"
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"


const MCGUID CLSID_CameraMappingInChannel(R_CLSID_CameraMappingInChannel);
const MCGUID CLSID_CameraMapping(R_CLSID_CameraMapping);


struct ShadingInBackup
{

public:

	ShadingInBackup(const ShadingIn& shadingIn):
	  fUV(shadingIn.fUV), fUVx (shadingIn.fUVx), fUVy(shadingIn.fUVy)
	{
	};


	TVector2	fUV;					///< u,v texture values

	TVector2	fUVx,fUVy;				///< texture derivatives

};

enum ProjectionRatio {pr1by1 = 1, pr4by3 = 2, pr3by2 = 3, pr16by9 = 4};

void getRenderingCamera(I3DShInstance *fInstance, const TMCString255& findCameraName, I3DShCamera** outCamera, I3DRenderingModule** outRenderingModule)
{

	TMCCountedPtr<I3DShScene> scene;
	TMCCountedGetHelper<I3DShCamera> resultCamera(outCamera);
	TMCCountedGetHelper<I3DRenderingModule> resultRenderingModule(outRenderingModule);
	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShTreeElement> cameraTree;
	TMCCountedPtr<I3DRenderingModule> renderingModule;
	TMCCountedPtr<I3DShCamera> camera;

	if (!fInstance)
	{
		return;
	}

	fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);

	if (!tree)
	{
		return;
	}

    tree->GetScene(&scene);

	if (!scene)
	{
		return;
	}

	scene->GetSceneRenderingModule(&renderingModule);
	//first look to see if we can get the camera by name
	{
		uint32 lCameraCount = scene->GetCamerasCount();
		for (uint32 lCameraIndex = 0; lCameraIndex < lCameraCount; lCameraIndex++)
		{
			scene->GetCameraByIndex(&camera, lCameraIndex);
			camera->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&cameraTree));
			if (MCVerify(cameraTree))
			{
				TMCString255 tempcameraName;
				cameraTree->GetName(tempcameraName);
				if (tempcameraName == findCameraName)
				{
					lCameraIndex = lCameraCount;
				}
			}
		}
	}


	if (!camera)
	{
		//try to get rendering camera from scene
		scene->GetRenderingCamera(&camera);
	}

	//can't find it there ask the rendering module
	if (!camera && renderingModule)
	{
		renderingModule->GetRenderingCamera(&camera);
	}

	//can't find it there, ask the rendering module for its
	//name and look for a match
	if (!camera && renderingModule)
	{
		//if the rendering module hasn't set the
		//camera, grab it's name and look for it
		//in the cameras list
		TMCString255 cameraName;
		renderingModule->GetRenderingCameraByName(cameraName);
		uint32 lCameraCount = scene->GetCamerasCount();
		for (uint32 lCameraIndex = 0; lCameraIndex < lCameraCount; lCameraIndex++)
		{
			scene->GetCameraByIndex(&camera, lCameraIndex);
			camera->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&cameraTree));
			if (MCVerify(cameraTree))
			{
				TMCString255 tempcameraName;
				cameraTree->GetName(tempcameraName);
				if (tempcameraName == cameraName)
				{
					lCameraIndex = lCameraCount;
				}
			}
		}
	}

	//can't find it there, just take camera 1
	if (!camera)
	{
		scene->GetCameraByIndex(&camera, 0);
	}

	resultCamera = camera;
	resultRenderingModule = renderingModule;
}

/*Internal use */
void MixTransform(TVector3 &res,const TMatrix33 &AA,const TVector3 &VV)
{
	TVector3 QQ;

	QQ[0]=AA[0][0]*VV[0] + AA[0][1]*VV[1] + AA[0][2]*VV[2] ;
	QQ[1]=AA[1][0]*VV[0] + AA[1][1]*VV[1] + AA[1][2]*VV[2] ;
	QQ[2]=AA[2][0]*VV[0] + AA[2][1]*VV[1] + AA[2][2]*VV[2] ;

	res=QQ;
}

void MixTransformPoint(TVector3 &res,const TTransform3D &AA,const TVector3 &VV)
{
	MixTransform(res,AA.fRotationAndScale,VV);
	res[0] += AA.fTranslation[0] ;
	res[1] += AA.fTranslation[1] ;
	res[2] += AA.fTranslation[2] ;
}

void MixTransformVector(TVector3 &res,const TTransform3D &AA,const TVector3 &VV)
{
	MixTransform(res,AA.fRotationAndScale,VV);

	//Be careful the result vector :res is not an unit vector
	//We have to normalize it
	real norm = sqrt(res[0]*res[0] +  res[1]*res[1] + res[2]*res[2]);
	res[0] /= norm;
	res[1] /= norm;
	res[2] /= norm;
}


EShaderOutput CameraMapping::GetImplementedOutput()
{
	if (extensionDataChangeDirty)
		getChildShaders();
	EShaderOutput retVal = kNotImplemented;
	if (childOutput.GetElemCount() > 0)
	{
		retVal = childOutput[childOutput.GetElemCount() - 1];
	}
	return (EShaderOutput)(retVal | kUsesGetValue | kUsesGetColor | kUsesGetVector | kUsesGetShaderApproxColor | kUsesDoShade);
};

const ShadingIn CameraMapping::getShadingInToUse(const ShadingIn& shadingIn)
{

	ShadingIn shadingInToUse = shadingIn;

	if (combinedChildFlags.fNeedsUV)
	{
		TMCCountedPtr<I3DShCamera> camera;
			TMCCountedPtr<I3DRenderingModule> renderingModule;
			getRenderingCamera(shadingInToUse.fInstance, fData.cameraName, &camera, &renderingModule);

		if (camera)
		{
			TTransform3D G2C ;		//Transformation from Global to Camera
			real distanceToScreen;
			TVector3 screenCoordPoint;
			camera->GetGlobalToCameraTransform(&G2C) ; //We get transformation from Global to Camera,

			MixTransformPoint(screenCoordPoint, G2C, shadingIn.fPoint);

			camera->Project3DTo2D(&screenCoordPoint, &shadingInToUse.fUV, &distanceToScreen);

			shadingInToUse.fUV.x += 1;
			shadingInToUse.fUV.y += 1;
			shadingInToUse.fUV *= 0.5f;

			if (renderingModule)
			{
				uint32 width, height;
				renderingModule->GetPixelSize(width, height);

				shadingInToUse.fUVx.SetValues(1 / width, 0);
				shadingInToUse.fUVy.SetValues(0, 1 / height);
			}
			else
			{
				shadingInToUse.fUVx.SetValues(.00001f, 0);
				shadingInToUse.fUVy.SetValues(0, .00001f);

			}

			if (fData.xRatio > fData.yRatio)
			{
				real32 ratio = static_cast<real32>(fData.xRatio) / static_cast<real32>(fData.yRatio);
				real32 offset = static_cast<real32>(fData.xRatio - fData.yRatio) / static_cast<real32>(fData.xRatio) * 0.5f;

				shadingInToUse.fUV.y = (shadingInToUse.fUV.y - offset) * ratio;
			}
			else if (fData.yRatio > fData.xRatio)
			{
				real32 ratio = static_cast<real32>(fData.yRatio) / static_cast<real32>(fData.xRatio);
				real32 offset = static_cast<real32>(fData.yRatio - fData.xRatio) / static_cast<real32>(fData.yRatio) * 0.5f;

				shadingInToUse.fUV.x = (shadingInToUse.fUV.x - offset) * ratio;
			}
		}



	}

	return shadingInToUse;
}

MCCOMErr CameraMapping::GetShadingFlags(ShadingFlags& theFlags)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	theFlags.CombineFlagsWith(combinedChildFlags);
	theFlags.fNeedsPoint = true;
	return MC_S_OK;
}

real CameraMapping::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return MC_S_OK;
	}
	ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) 
	{
		return getChildValue(result, shadingInToUse, childCount - 1);
	}
	return MC_S_OK;
}

real CameraMapping::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return MC_S_OK;
	}
	ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) 
	{
		return getChildColor(result, shadingInToUse, childCount - 1);
	}
	return MC_S_OK;
}

real CameraMapping::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return MC_S_OK;
	}
	ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) 
	{
		return getChildVector(result, shadingInToUse, childCount - 1);
	}
	return MC_S_OK;
}

MCCOMErr CameraMapping::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return MC_S_OK;
	}
	ShadingIn shadingInToUse;
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) 
	{
		if ((childOutput[childCount - 1] & kUsesDoShade) == kUsesDoShade)
		{
			shadingInToUse = getShadingInToUse(shadingIn);
			childShaders[childCount - 1]->DoShade(result, shadingInToUse);
		}
		shadingIn.fCurrentCompletionMask = shadingInToUse.fCurrentCompletionMask;
	}
	return MC_S_OK;
}

void CameraMapping::GetShaderApproxColor(TMCColorRGBA& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return;
	}

	ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0) 
	{
		getChildShaderApproxColor(result, shadingInToUse, childCount - 1);
	}

}

boolean	CameraMapping::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0)
	{
		return childShaders[childCount - 1]->HasExplicitLightingModel(lightingContext);
	}
	else
		return false;

}

void copyMappedValues(const ShadingIn &from, ShadingIn &to)
{
	to.fUV = from.fUV;
	to.fUVx = from.fUVx;
	to.fUVy = from.fUVy;
}

void copyMappedValues(const ShadingInBackup &from, ShadingIn &to)
{
	to.fUV = from.fUV;
	to.fUVx = from.fUVx;
	to.fUVy = from.fUVy;
}

MCCOMErr CameraMapping::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return MC_S_OK;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0)
	{

		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		childShaders[childCount - 1]->ShadeAndLight2(result, lightingContext, absorptionFunction);

		copyMappedValues(backup, *lightingContext.fHit);

		return MC_S_OK;
	}
	else
		return MC_E_NOTIMPL;

}

void CameraMapping::CalculateDirectLighting(LightingDetail& result,const LightingContext&	lightingContext)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateDirectLighting) == kUseCalculateDirectLighting)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateDirectLighting(result, lightingContext);

		copyMappedValues(backup, *lightingContext.fHit);

	}
	
}

void CameraMapping::CalculateReflection(TMCColorRGB& reflectionColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateReflection) == kUseCalculateReflection)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateReflection(reflectionColor, lightingContext, shading);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}

void CameraMapping::CalculateCaustics(TMCColorRGB& causticColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateCaustics) == kUseCalculateCaustics)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateCaustics(causticColor, lightingContext, shading);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}

void CameraMapping::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateIndirectLighting) == kUseCalculateIndirectLighting)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}

void CameraMapping::CalculateTransparency(TMCColorRGB& resColor,real32& outAlpha,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateTransparency) == kUseCalculateTransparency)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateTransparency(resColor, outAlpha, lightingContext, shading);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}

void CameraMapping::CalculateAlpha(LightingDetail& result,const LightingContext& lightingContext,const ShadingOut& shading,TAbsorptionFunction* absorptionFunction)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateAlpha) == kUseCalculateAlpha)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateAlpha(result, lightingContext, shading, absorptionFunction);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}

void CameraMapping::CalculateSubsurfaceScattering(TMCColorRGB& subsurfaceColor,const TMCColorRGB& currentColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != lightingContext.fHit->fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUseCalculateSubsurfaceScattering) == kUseCalculateSubsurfaceScattering)
	{
		ShadingInBackup backup(*lightingContext.fHit);

		ShadingIn shadingInToUse = getShadingInToUse(*lightingContext.fHit);

		copyMappedValues(shadingInToUse, *lightingContext.fHit);

		(reinterpret_cast<TBasicShader*>(childShaders[childCount - 1]))->CalculateSubsurfaceScattering(subsurfaceColor, currentColor, lightingContext, shading);

		copyMappedValues(backup, *lightingContext.fHit);
	}
}


void CameraMapping::GetReflection(TReflectionParameters& reflectionParams,const ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUsesGetReflection) == kUsesGetReflection)
	{
		ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
		childShaders[childCount - 1]->GetReflection(reflectionParams, shadingInToUse);
	}
}

void CameraMapping::GetTransparency(TTransparencyParameters& transparencyParams,const ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUsesGetTransparency) == kUsesGetTransparency)
	{
		ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
		childShaders[childCount - 1]->GetTransparency(transparencyParams, shadingInToUse);
	}
}

void CameraMapping::GetShaderApproximation(ShadingOut& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUsesGetShaderApprox) == kUsesGetShaderApprox)
	{
		ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
		childShaders[childCount - 1]->GetShaderApproximation(result, shadingInToUse);
	}
}

MCCOMErr CameraMapping::GetBump(TVector3& result,ShadingIn& shadingIn)
{
	if (extensionDataChangeDirty)
		getChildShaders();
	if (fData.uvSpaceId != -2 && fData.uvSpaceId != shadingIn.fUVSpaceID)
	{
		return MC_S_OK;
	}

	uint32 childCount = childShaders.GetElemCount();
	if (childCount > 0 && (childOutput[childCount - 1] & kUsesGetBump) == kUsesGetBump)
	{
		ShadingIn shadingInToUse = getShadingInToUse(shadingIn);
		childShaders[childCount - 1]->GetBump(result, shadingInToUse);
	}
	return MC_S_OK;
}


MCCOMErr CameraMapping::HandleEvent(MessageID message, IMFResponder* source, void* data)
{

	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	
	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	if (sourceID == 'PRAT' &&message == EMFPartMessage::kMsg_PartValueChanged) 
	{
		TMCCountedPtr<IMFPart> parentPart;
		ActionNumber newValue;

		sourcePart->GetPartParent(&parentPart);
		ThrowIfNil(parentPart);
		sourcePart->GetValue(&newValue, kActionValueType);

		switch (newValue)
		{
		case pr1by1:
			fData.xRatio = 1;
			fData.yRatio = 1;
			break;
		case pr4by3:
			fData.xRatio = 4;
			fData.yRatio = 3;
			break;
		case pr3by2:
			fData.xRatio = 3;
			fData.yRatio = 2;
			break;
		case pr16by9:
			fData.xRatio = 16;
			fData.yRatio = 9;
			break;

		}
	}


	return MC_S_OK;}
