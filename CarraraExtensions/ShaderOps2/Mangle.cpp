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
#include "Mangle.h"
#include "ShaderOps2DLL.h"
#include "MangleResult.h"

const MCGUID CLSID_MangleSpecular(R_CLSID_MangleSpecular);
const MCGUID CLSID_MangleDiffuse(R_CLSID_MangleDiffuse);
const MCGUID CLSID_MangleGlow(R_CLSID_MangleGlow);
const MCGUID CLSID_MangleAmbient(R_CLSID_MangleAmbient);
const MCGUID CLSID_MangleReflection(R_CLSID_MangleReflection);
const MCGUID CLSID_MangleIllumination(R_CLSID_MangleIllumination);
const MCGUID CLSID_MangleColor(R_CLSID_MangleColor);
const MCGUID CLSID_MangleBackground(R_CLSID_MangleBackground);
const MCGUID CLSID_MangleLightShadow(R_CLSID_MangleLightShadow);
const MCGUID CLSID_MangleRefraction(R_CLSID_MangleRefraction);
const MCGUID CLSID_MangleGlobalIllumination(R_CLSID_MangleGlobalIllumination);
const MCGUID CLSID_MangleCaustics(R_CLSID_MangleCaustics);
const MCGUID CLSID_MangleSubsurfaceScattering(R_CLSID_MangleSubsurfaceScattering);
const MCGUID CLSID_MangleAmbientOcclusionFactor(R_CLSID_MangleAmbientOcclusionFactor);
const MCGUID CLSID_MangleAlpha(R_CLSID_MangleAlpha);


TMCColorRGB colorFromGrayScale(real grayScale)
{
	TMCColorRGB value(grayScale, grayScale, grayScale);
	return value;
}

Mangle::Mangle(MangleMode mangleMode) 
{
	this->mangleMode = mangleMode;
}

Mangle::~Mangle() 
{

}


void* Mangle::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

boolean	Mangle::IsEqualTo(I3DExShader* aShader)
{
	return (true);
}  

MCCOMErr Mangle::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Mangle::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
}


TMCColorRGB getMangleColor(MangleMode mangleMode)
{
	if (gMangleResult->GetValue() == NULL)
	{
		TMCColorRGB red(1, 0, 0);
		return red;//TMCColorRGB::kBlack;
	}

	switch (mangleMode)
	{
	//TMCColorRGB based
	case mmSpecular:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fSpecularLight;
		break;
	case mmDiffuse:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fDiffuseLight;
		break;
	case mmGlow:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fGlow;
		break;
	case mmAmbient:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fAmbient;
		break;
	case mmReflection:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fReflection;
		break;
	case mmIllumination:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fIllumination;
		break;
	case mmColor:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fRenderedColor;
		break;
	case mmBackground:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fBackground;
		break;
	case mmLightShadow:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fLightShadow;
		break;
	case mmRefraction:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fRefraction;
		break;
	case mmGlobalIllumination:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fGlobalIllumination;
		break;
	case mmCaustics:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fCaustics;
		break;
	case mmSubsurfaceScattering:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fSubsurfaceScattering;
		break;
	
	//real based
	case mmAmbientOcclusionFactor:
		return colorFromGrayScale(static_cast<MangleResult*>(gMangleResult->GetValue())->ambientOcclusionFactor);
		break;
	case mmAlpha:
		return colorFromGrayScale(static_cast<MangleResult*>(gMangleResult->GetValue())->alpha);
		break;
	default:
		return TMCColorRGB::kBlack;
		break;
	};

}

real getMangleValue(MangleMode mangleMode)
{
	if (gMangleResult->GetValue() == NULL)
	{
		return 0;
	}

	switch (mangleMode)
	{
	//TMCColorRGB based
	case mmSpecular:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fSpecularLight.Intensity();
		break;
	case mmDiffuse:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fDiffuseLight.Intensity();
		break;
	case mmGlow:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fGlow.Intensity();
		break;
	case mmAmbient:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fAmbient.Intensity();
		break;
	case mmReflection:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fReflection.Intensity();
		break;
	case mmIllumination:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fIllumination.Intensity();
		break;
	case mmColor:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fRenderedColor.Intensity();
		break;
	case mmBackground:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fBackground.Intensity();
		break;
	case mmLightShadow:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fLightShadow.Intensity();
		break;
	case mmRefraction:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fRefraction.Intensity();
		break;
	case mmGlobalIllumination:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fGlobalIllumination.Intensity();
		break;
	case mmCaustics:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fCaustics.Intensity();
		break;
	case mmSubsurfaceScattering:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->fSubsurfaceScattering.Intensity();
		break;
	
	//real based
	case mmAmbientOcclusionFactor:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->ambientOcclusionFactor;
		break;
	case mmAlpha:
		return static_cast<MangleResult*>(gMangleResult->GetValue())->alpha;
		break;
	default:
		return 0;
		break;
	};

}

real Mangle::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	result = getMangleValue(mangleMode);
	return MC_S_OK;
}

real Mangle::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
{
	result = getMangleColor(mangleMode);
	return MC_S_OK;

}
