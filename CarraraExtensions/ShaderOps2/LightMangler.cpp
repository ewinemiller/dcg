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
#include "LightMangler.h"
#include "rendertypes.h"
#include "ShaderOps2DLL.h"
#include "I3dExRenderFeature.h"
#include "I3DExVertexPrimitive.h"
#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3dExPrimitive.h"
#include "comutilities.h"
#include "ishutilities.h"
#include "I3DShMasterGroup.h"
#include "I3dShTreeElement.h"
#include "dcgmeshutil.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "COMSafeUtilities.h"
#include "MangleResult.h"


const MCGUID CLSID_LightMangler(R_CLSID_LightMangler);


LightMangler::LightMangler()
{
}

LightMangler::~LightMangler() 
{
}

void* LightMangler::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

void LightMangler::extensionDataChangedDoMangler(I3DExShader** subShader, IShParameterComponent* parameter, EShaderOutput& childOutput)
{
	TMCCountedGetHelper<I3DExShader> result(subShader);
	TMCCountedPtr<I3DExShader> localSubShader;

	if (parameter)
	{
		ShadingFlags tempChildFlags;
		parameter->QueryInterface(IID_I3DExShader, reinterpret_cast<void**>(&localSubShader));
		localSubShader->GetShadingFlags(tempChildFlags);
		childFlags.CombineFlagsWith(tempChildFlags);
		childOutput = localSubShader->GetImplementedOutput();
	}

	result = localSubShader;
}

MCCOMErr LightMangler::ExtensionDataChanged()
{
	ShadingFlags tempChildFlags;
	childFlags = tempChildFlags;

	if (!fData.fullShader) 
	{
		fullShader = NULL;
	}
	else
	{
		fData.fullShader->QueryInterface(IID_I3DExShader, (void**)&fullShader);
		fullShader->GetShadingFlags(tempChildFlags);
		childFlags.CombineFlagsWith(tempChildFlags);
		fullChildOutput = fullShader->GetImplementedOutput();
	}

	extensionDataChangedDoMangler(&mangleSpecular, fData.mangleSpecular, childOutputSpecular);
	extensionDataChangedDoMangler(&mangleDiffuse, fData.mangleDiffuse, childOutputDiffuse);
	extensionDataChangedDoMangler(&mangleGlow, fData.mangleGlow, childOutputGlow);
	extensionDataChangedDoMangler(&mangleAmbient, fData.mangleAmbient, childOutputAmbient);
	extensionDataChangedDoMangler(&mangleReflection, fData.mangleReflection, childOutputReflection);
	extensionDataChangedDoMangler(&mangleBackground, fData.mangleBackground, childOutputBackground);
	extensionDataChangedDoMangler(&mangleLightShadow, fData.mangleLightShadow, childOutputLightShadow);
	extensionDataChangedDoMangler(&mangleRefraction, fData.mangleRefraction, childOutputRefraction);
	extensionDataChangedDoMangler(&mangleGlobalIllumination, fData.mangleGlobalIllumination, childOutputGlobalIllumination);
	extensionDataChangedDoMangler(&mangleCaustics, fData.mangleCaustics, childOutputCaustics);
	extensionDataChangedDoMangler(&mangleSubsurfaceScattering, fData.mangleSubsurfaceScattering, childOutputSubsurfaceScattering);	
	extensionDataChangedDoMangler(&mangleAlpha, fData.mangleAlpha, childOutputAlpha);


	return MC_S_OK;
}

boolean	LightMangler::IsEqualTo(I3DExShader* aShader)
{
	return ((static_cast<LightMangler*>(aShader))->fData == fData);
}  

MCCOMErr LightMangler::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = childFlags;
	return MC_S_OK;
}

EShaderOutput LightMangler::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
		+ kUseCalculateDirectLighting
		+ kUseCalculateIndirectLighting
		+ kUseCalculateReflection
		+ kUseCalculateTransparency
		+ kUseCalculateCaustics
		+ kUseCalculateSubsurfaceScattering
		+ kUseCalculateAlpha
		
		);
}

boolean LightMangler::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}

void LightMangler::mangle(I3DExShader* subShader, const EShaderOutput childOutput, TMCColorRGB& result, ShadingIn& shadingIn)
{
	if (subShader)
	{
		boolean fullArea = false;
		if ((childOutput & kUsesGetColor) == kUsesGetColor)
		{
			TMCColorRGBA temp;
			subShader->GetColor(temp, fullArea, shadingIn);
			result.Set(temp.red, temp.green, temp.blue);
		}
		else if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			real temp;
			subShader->GetValue(temp, fullArea, shadingIn);
			result.Set(temp, temp, temp);

		}

	}

}

void LightMangler::mangle(I3DExShader* subShader, const EShaderOutput childOutput, real& result, ShadingIn& shadingIn)
{
	if (subShader)
	{
		boolean fullArea = false;
		if ((childOutput & kUsesGetValue) == kUsesGetValue)
		{
			subShader->GetValue(result, fullArea, shadingIn);

		}
		else if ((childOutput & kUsesGetColor) == kUsesGetColor)
		{
			TMCColorRGBA temp;
			subShader->GetColor(temp, fullArea, shadingIn);
			result = temp.Intensity();
		}


	}

}

#if VERSIONNUMBER >= 0x070000
MCCOMErr LightMangler::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr LightMangler::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	this->absorptionFunction = absorptionFunction;
	return MC_E_NOTIMPL;
}

void LightMangler::CalculateDirectLighting(LightingDetail&  result, const LightingContext& lightingContext)
{
	if (fullShader != NULL)
	{
		resultLocal = &result;

		const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
		const LightingFlags lightingFlags = lightingContext.fLightingFlags;
		ShadingOut& shadingOut = *result.fShadingOut;

		ShadingIn& shadingIn = *lightingContext.fHit;
		LightingDetail illuminationResult = result;
		ShadingOut illuminationShadingOut = *illuminationResult.fShadingOut;
		illuminationShadingOut.fColor = TMCColorRGBA::kWhiteNoAlpha;
		illuminationResult.fShadingOut = &illuminationShadingOut;

		if ((fullChildOutput & kUseCalculateDirectLighting) == kUseCalculateDirectLighting)
		{
			(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateDirectLighting(result, lightingContext);
			if (fData.calculateIllumination)
			{
				(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateDirectLighting(illuminationResult, lightingContext);
			}
		}
		else
		{

			defaultlightingmodel->CalculateDirectLighting(result, lightingContext);
			if (fData.calculateIllumination)
			{
				defaultlightingmodel->CalculateDirectLighting(illuminationResult, lightingContext);
			}
		}

		if (shadingOut.fReflection.fReflection.Intensity() > 0)
		{
			if ((fullChildOutput & kUseCalculateReflection) == kUseCalculateReflection)
			{
				(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateReflection(result.fReflection, lightingContext, shadingOut);
			}
			else
			{
				defaultlightingmodel->CalculateReflection(result.fReflection, lightingContext, shadingOut);
			}
		}

		if ((fullChildOutput & kUseCalculateCaustics) == kUseCalculateCaustics)
		{
			(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateCaustics(result.fCaustics, lightingContext, shadingOut);
		}
		else
		{
			defaultlightingmodel->CalculateCaustics(result.fCaustics, lightingContext, shadingOut);
		}
		

		if ( (illumSettings.fRadiosityOn || illumSettings.fUseSkyLight) && lightingFlags.fComputeDiffuseIndirectLighting 
			&& (shadingOut.fColor.R>0.02f || shadingOut.fColor.G>0.02f || shadingOut.fColor.B>0.02f))
		{
			if ((fullChildOutput & kUseCalculateIndirectLighting) == kUseCalculateIndirectLighting)
			{
				(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateIndirectLighting(result.fGlobalIllumination, result.fAmbientOcclusionFactor, lightingContext, shadingOut);		
			}
			else
			{
				defaultlightingmodel->CalculateIndirectLighting(result.fGlobalIllumination, result.fAmbientOcclusionFactor, lightingContext, shadingOut);
			}		
		}
		
		
		if (shadingOut.fSubsurfaceScattering.fIntensity > 0)
		{
			TMCColorRGB currentColor = result.fAmbient;
			currentColor += result.fDiffuseLight;
			currentColor += result.fGlobalIllumination;

			if ((fullChildOutput & kUseCalculateSubsurfaceScattering) == kUseCalculateSubsurfaceScattering)
			{
				(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateSubsurfaceScattering(result.fSubsurfaceScattering, currentColor, lightingContext, shadingOut);
			}
			else
			{
				defaultlightingmodel->CalculateSubsurfaceScattering(result.fSubsurfaceScattering, currentColor, lightingContext, shadingOut);
			}
		}

		if ((fullChildOutput & kUseCalculateTransparency) == kUseCalculateTransparency && shadingOut.fTransparency.fIntensity > 0)
		{
			(reinterpret_cast<TBasicShader*>(fullShader.fObject))->CalculateTransparency(result.fRefraction, transparencyAlpha, lightingContext, shadingOut);
		}
		else
		{
			defaultlightingmodel->CalculateTransparency(result.fRefraction, transparencyAlpha, lightingContext, shadingOut);
		}
		transparencyColor = result.fRefraction;

		TMCColorRGBA backgroundColor = TMCColorRGBA::kBlackNoAlpha;
		if (fData.calculateBackground)
		{
			boolean	fullAreaDone;
			lightingContext.fRaytracer-> GetBackGroundColor(backgroundColor, fullAreaDone,
													*lightingContext.fIncomingRay,
													lightingContext.fScreenCoordinates,
													true,
													true,
													false);
		}

		MangleResult mangleResult;

		mangleResult.fSpecularLight = result.fSpecularLight;
		mangleResult.fDiffuseLight = result.fDiffuseLight;
		mangleResult.fGlow = result.fGlow;
		mangleResult.fAmbient = result.fAmbient;
		mangleResult.fReflection = result.fReflection;
		mangleResult.fIllumination = illuminationResult.fDiffuseLight;
		mangleResult.fRenderedColor.Set(result.fShadingOut->fColor.red, result.fShadingOut->fColor.green, result.fShadingOut->fColor.blue);
		mangleResult.alpha = result.fShadingOut->fColor.alpha;
		mangleResult.fBackground.Set(backgroundColor.red, backgroundColor.green, backgroundColor.blue);
		mangleResult.fLightShadow = result.fLightShadow;
		mangleResult.fRefraction = result.fRefraction;
		mangleResult.fCaustics = result.fCaustics;
		mangleResult.fSubsurfaceScattering = result.fSubsurfaceScattering;
		mangleResult.fGlobalIllumination = result.fGlobalIllumination;
		mangleResult.ambientOcclusionFactor = result.fAmbientOcclusionFactor;
		
		gMangleResult->SetValue(static_cast<void*>(&mangleResult));

		mangle(mangleSpecular, childOutputSpecular, result.fSpecularLight, shadingIn);
		mangle(mangleDiffuse, childOutputDiffuse, result.fDiffuseLight, shadingIn);
		mangle(mangleGlow, childOutputGlow, result.fGlow, shadingIn);
		mangle(mangleAmbient, childOutputAmbient, result.fAmbient, shadingIn);
		mangle(mangleReflection, childOutputReflection, result.fReflection, shadingIn);
		mangle(mangleBackground, childOutputBackground, result.fBackground, shadingIn);
		if (illumSettings.fComputeSeparatedShadow)
		{
			mangle(mangleLightShadow, childOutputLightShadow, result.fLightShadow, shadingIn);
		}
		else
		{
			result.fLightShadow.Set(1, 1, 1);
		}
		mangle(mangleRefraction, childOutputRefraction, result.fRefraction, shadingIn);
		mangle(mangleGlobalIllumination, childOutputGlobalIllumination, result.fGlobalIllumination, shadingIn);
		mangle(mangleCaustics, childOutputCaustics, result.fCaustics, shadingIn);
		mangle(mangleSubsurfaceScattering, childOutputSubsurfaceScattering, result.fSubsurfaceScattering, shadingIn);
		mangle(mangleAlpha, childOutputAlpha, result.fShadingOut->fColor.alpha, shadingIn);

		gMangleResult->SetValue(NULL);

		//set the values to be reported later
		reflectionColor = result.fReflection;
		result.fReflection = TMCColorRGB::kBlack;

		causticColor = result.fCaustics;
		result.fCaustics = TMCColorRGB::kBlack;

		indirectDiffuseColor = result.fGlobalIllumination;
		ambientOcclusionFactor = result.fAmbientOcclusionFactor;

		subsurfaceColor = result.fSubsurfaceScattering;

		result.fRefraction -= transparencyColor;

		//CalculateAlpha not being called as expected
		//alpha = result.fShadingOut->fColor.alpha;
		//result.fShadingOut->fColor.alpha = 1.0f;

	}
}

void LightMangler::CalculateReflection(TMCColorRGB& reflectionColor,const LightingContext&	lightingContext,const ShadingOut& shading)
{
	reflectionColor = this->reflectionColor;
}

void LightMangler::CalculateCaustics(TMCColorRGB& causticColor, const LightingContext& lightingContext,const ShadingOut& shading)
{
	causticColor = this->causticColor;
}
void LightMangler::CalculateTransparency(TMCColorRGB& resColor, real32& outAlpha, const LightingContext& lightingContext,const ShadingOut& shading)
{
	resColor = this->transparencyColor;
	outAlpha = this->transparencyAlpha;
}


void LightMangler::CalculateSubsurfaceScattering(TMCColorRGB& subsurfaceColor, const TMCColorRGB& currentColor, const LightingContext& lightingContext, const ShadingOut& shading)
{
	subsurfaceColor = this->subsurfaceColor;
}


void LightMangler::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
{
	indirectDiffuseColor = this->indirectDiffuseColor;
	ambientOcclusionFactor = this->ambientOcclusionFactor;
}

void LightMangler::CalculateAlpha(LightingDetail& result, const LightingContext& lightingContext, const ShadingOut& shading, TAbsorptionFunction* absorptionFunction)
{
	//result.fResColor.alpha = alpha;
}



MCCOMErr LightMangler::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{

	if (fullShader != NULL)
	{

		fullShader->DoShade(result, shadingIn);	
	}
	return MC_S_OK;
}


