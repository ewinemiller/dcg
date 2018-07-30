/*  Carrara plug-in utilities
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
#include "DCGSceneUtil.h"
#include "MCCountedPtrHelper.h"
#include "I3DRenderingModule.h"
#include "I3DShTreeElement.h"

MCCOMErr getRenderingCamera(I3DShScene* scene, I3DShCamera** renderingCamera)
{
	TMCCountedGetHelper<I3DShCamera> result(renderingCamera);
	TMCCountedPtr<I3DRenderingModule> renderingModule;

	scene->GetSceneRenderingModule(&renderingModule);

	if (MCVerify(renderingModule))
	{

		renderingModule->GetRenderingCamera(renderingCamera);
		if (!MCVerify(*renderingCamera))
		{
			//if the rendering module hasn't set the 
			//camera, grab it's name and look for it
			//in the cameras list
			TMCString255 cameraName;
			renderingModule->GetRenderingCameraByName(cameraName);
			uint32 lCameraCount = scene->GetCamerasCount();
			for (uint32 lCameraIndex = 0; lCameraIndex < lCameraCount; lCameraIndex++)
			{
				scene->GetCameraByIndex(renderingCamera, lCameraIndex);
				if (MCVerify(*renderingCamera))
				{
					TMCCountedPtr<I3DShTreeElement> cameraTree;
					(*renderingCamera)->QueryInterface(IID_I3DShTreeElement, reinterpret_cast<void**>(&cameraTree));
					if (MCVerify(cameraTree))
					{
						TMCString255 tempCameraName;
						cameraTree->GetName(tempCameraName);
						if (tempCameraName == cameraName)
						{

							break;
						}
					}
				}
			}
		}
	}
	else
	{
		scene->GetRenderingCamera(renderingCamera);
		if (!*renderingCamera)
		{
			scene->GetCameraByIndex(renderingCamera, 0);
		}
	}
	if (MCVerify(*renderingCamera))
		return MC_S_OK;
	else
		return MC_S_FALSE;
}

MCCOMErr getSceneFromInstance(I3DShInstance* instance, I3DShScene** scene)
{
	TMCCountedGetHelper<I3DShScene> result(scene);
	TMCCountedPtr<I3DShTreeElement> tree;
	
	if (MCVerify(instance))
		tree = instance->GetTreeElement();

	if (MCVerify(tree))
		tree->GetScene(scene);

	if (MCVerify(*scene))
		return MC_S_OK;
	else
		return MC_S_FALSE;

}