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
#include "copyright.h"
#include "IShSMP.h"

#include "COMUtilities.h"
#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif
#include "IShUtilities.h"

#include "MultiAdd.h"
#include "MultiMin.h"
#include "MultiMax.h"
#include "MultiSubtract.h"
#include "MultiMultiply.h"
#include "MultiAccumulate.h"

#include "SpaceMangler.h"
#include "LightMangler.h"
#include "Mangle.h"
#include "LayersList.h"
#include "Layer.h"

#include "CameraMapping.h"

#include "AnimatedMixer.h"
#include "Blur.h"
#include "CropUV.h"

IShLocalStorage* gMangleResult = NULL;

TMCCriticalSection* myCS;


void Extension3DInit(IMCUnknown* utilities)
{
	gShellSMPUtilities->CreateLocalStorage(&gMangleResult);

	myCS= NewCS();

}

void Extension3DCleanup()
{
		DeleteCS(myCS);
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{
	TBasicUnknown* res = NULL;

	if (classId == CLSID_MultiAdd) res = new MultiAdd();
	if (classId == CLSID_MultiMin) res = new MultiMin();
	if (classId == CLSID_MultiMax) res = new MultiMax();
	if (classId == CLSID_MultiSubtract) res = new MultiSubtract();
	if (classId == CLSID_MultiMultiply) res = new MultiMultiply();
	if (classId == CLSID_MultiAccumulate) res = new MultiAccumulate();
	if (classId == CLSID_LayersList) res = new LayersList();
	if (classId == CLSID_Layer) 
		res = new Layer();

	if (classId == CLSID_AnimatedMixer) res = new AnimatedMixer();


	if (classId == CLSID_SpaceMangler) res = new SpaceMangler();
	if (classId == CLSID_LightMangler) res = new LightMangler();
	if (classId == CLSID_CameraMappingInChannel) res = new CameraMapping();
	if (classId == CLSID_CameraMapping) res = new CameraMapping();
	if (classId == CLSID_MangleSpecular) res = new Mangle(mmSpecular);
	if (classId == CLSID_MangleDiffuse) res = new Mangle(mmDiffuse);
	if (classId == CLSID_MangleGlow) res = new Mangle(mmGlow);
	if (classId == CLSID_MangleAmbient) res = new Mangle(mmAmbient);
	if (classId == CLSID_MangleReflection) res = new Mangle(mmReflection);
	if (classId == CLSID_MangleIllumination) res = new Mangle(mmIllumination);
	if (classId == CLSID_MangleColor) res = new Mangle(mmColor);
	if (classId == CLSID_MangleBackground) res = new Mangle(mmBackground);
	if (classId == CLSID_MangleLightShadow) res = new Mangle(mmLightShadow);
	if (classId == CLSID_MangleRefraction) res = new Mangle(mmRefraction);
	if (classId == CLSID_MangleGlobalIllumination) res = new Mangle(mmGlobalIllumination);
	if (classId == CLSID_MangleCaustics) res = new Mangle(mmCaustics);
	if (classId == CLSID_MangleSubsurfaceScattering) res = new Mangle(mmSubsurfaceScattering);
	if (classId == CLSID_MangleAmbientOcclusionFactor) res = new Mangle(mmAmbientOcclusionFactor);
	if (classId == CLSID_MangleAlpha) res = new Mangle(mmAlpha);

	if (classId == CLSID_Blur) res = new Blur();
	if (classId == CLSID_CropUV) res = new CropUV();
	return res;
}
