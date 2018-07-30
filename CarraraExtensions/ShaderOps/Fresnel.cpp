/*  Shader Ops - plug-in for Carrara
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
#include "Fresnel.h" 
#include "ShaderOpsDLL.h"
#include "I3DShInstance.h"
#include "ISceneDocument.h"
#include "I3DRenderingModule.h"
#include "I3dShCamera.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Fresnel(R_CLSID_Fresnel);
#else
const MCGUID CLSID_Fresnel={R_CLSID_Fresnel};
#endif

Fresnel::Fresnel() {
	SetDefaults();
	}

void Fresnel::SetDefaults() {
	fData.fRolloff = .3333f;
	}

void* Fresnel::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr Fresnel::ExtensionDataChanged(){
	return MC_S_OK;
	}

boolean	Fresnel::IsEqualTo(I3DExShader* aShader)
{
  return (fData.fRolloff==((Fresnel*)aShader)->fData.fRolloff); 
}  

MCCOMErr Fresnel::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsPoint  = true;
	theFlags.fNeedsNormal = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
	}

EShaderOutput Fresnel::GetImplementedOutput(){
	return (EShaderOutput)(kUsesGetValue | kUsesGetShaderApproxColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 Fresnel::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	fullArea = false;
	result = 0;

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCCountedPtr<I3DShScene> scene;
	TMCCountedPtr<I3DRenderingModule> renderingModule;
	TMCCountedPtr<I3DShTreeElement> cameratree;
	TMCCountedPtr<I3DShCamera> camera;

	real32 fRolloff = 1.0f + fData.fRolloff * 9.0f;

	if (!shadingIn.fInstance)
	{
		return result;
	}

	shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);

	if (!tree)
	{
		return result;
	}

    tree->GetScene(&scene);

	if (!scene)
	{
		return result;
	}

	//try to get rendering camera from scene
	scene->GetRenderingCamera(&camera);

	//can't find it there ask the rendering module
	if (!camera)
	{
		scene->GetSceneRenderingModule(&renderingModule);
		if (renderingModule)
		{
			renderingModule->GetRenderingCamera(&camera);
		}
	}

	//can't find it there, ask the rendering module for its 
	//name and look for a match
	if (!camera)
	{
		scene->GetSceneRenderingModule(&renderingModule);
		if (renderingModule)
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
				camera->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&cameratree));
				if (MCVerify(cameratree))
				{
					TMCString255 tempcameraName;
					cameratree->GetName(tempcameraName);
					if (tempcameraName == cameraName)
					{
						lCameraIndex = lCameraCount;
					}
				}
			}
		}
	}
	
	//can't find it there, just take camera 1
	if (!camera)
	{
		scene->GetCameraByIndex(&camera, 0);
	}

	if (camera)
	{
		camera->QueryInterface(IID_I3DShTreeElement, (void**)&cameratree);
	}
	if (cameratree)
	{
		TTransform3D cameratransform;
		TVector3 away;
#if (VERSIONNUMBER >= 0x040000 )	
		cameratransform = cameratree->GetGlobalTransform3D();
#else
		cameratree->GetGlobalTransform3D(cameratransform);
#endif
		away = cameratransform.fTranslation - shadingIn.fPoint;
		away.Normalize(away);

		result = away * shadingIn.fGNormal;
		result = 1.0f - pow(1.0f - result, fRolloff);
	}

	tree = NULL;
	scene = NULL;
	renderingModule = NULL;
	cameratree = NULL;
	camera = NULL;
	if (result < 0)
	{
		result = 0;
	}
	return result;
}

int16 Fresnel::GetResID(){
	return 318;
	}
