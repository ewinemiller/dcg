/*  Shader Plus - plug-in for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#include "SpecularDiffuseControl.h"
#include "rendertypes.h"
#include "ShadersPlusDLL.h"
#include "I3dExRenderFeature.h"
#include "I3dShTreeElement.h"
#include "I3DShInstance.h"
#include "ISceneDocument.h"
#include "I3DRenderingModule.h"
#include "I3dShCamera.h"
#include "I3DShLightsource.h"
#include "MCCountedPtrhelper.h"
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "COM3DUtilities.h"
#include "dcguiutil.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_SpecularDiffuseControl(R_CLSID_SpecularDiffuseControl);
const MCGUID IID_SpecularDiffuseControl(R_IID_SpecularDiffuseControl);
#else
const MCGUID CLSID_SpecularDiffuseControl={R_CLSID_SpecularDiffuseControl};
const MCGUID IID_SpecularDiffuseControl={R_IID_SpecularDiffuseControl};
#endif


SpecularDiffuseControl::SpecularDiffuseControl() 
{
	fData.synchAllLights = true;
	fData.ambientLight = 1.0f;
	fData.giLight = 1.0f;
}

void* SpecularDiffuseControl::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr SpecularDiffuseControl::ExtensionDataChanged()
{
	if (!fData.param) 
	{
		shader = NULL;

	}
	else 
	{
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();

	}
	return MC_S_OK;
}

boolean	SpecularDiffuseControl::IsEqualTo(I3DExShader* aShader)
{
	return (false);
}  

MCCOMErr SpecularDiffuseControl::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput SpecularDiffuseControl::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesDoShade 
#if VERSIONNUMBER >= 0x050000
		+ kUseCalculateDirectLighting
		+ kUseCalculateIndirectLighting
#endif
		);
}

#if VERSIONNUMBER >= 0x050000
#if VERSIONNUMBER >= 0x070000
MCCOMErr SpecularDiffuseControl::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr SpecularDiffuseControl::ShadeAndLight	(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	defaultlightingmodel = inDefaultLightingModel;
	return MC_E_NOTIMPL;
}
#endif

boolean SpecularDiffuseControl::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	return true;
}
void SpecularDiffuseControl::CalculateDirectLighting(LightingDetail&   result, const LightingContext& lightingContext)
{
#if VERSIONNUMBER >= 0x070000
	const IllumSettings& illumSettings = *lightingContext.fIllumSettings;
#else
	const IllumSettings& illumSettings = lightingContext.fIllumSettings;
#endif	
	const LightingFlags lightingFlags = lightingContext.fLightingFlags;

	if (lightingFlags.fComputeDiffuseDirectLighting)
	{
		DirectLighting directLighting;
		const TMCColorRGB& lighting  = directLighting.fLightColor;
		const TVector3& lightDirection = directLighting.fLightDirection;
#if VERSIONNUMBER >= 0x070000
		const TMCColorRGB&	noShadowlighting = directLighting.fNoShadowLightColor;	// the no shadow ligtht color will be used when we need to compute separated shadow
		TMCColorRGB			diffShadow = TMCColorRGB::kBlack;
		TMCColorRGB			specShadow = TMCColorRGB::kBlack;
#endif

		const ShadingOut& shading = *result.fShadingOut;
		uint32 numlevels = levels.GetElemCount();
		TMCArray<boolean> copied(numlevels, true);

		//first get the ambient light

		I3DExRaytracer* raytracer = lightingContext.fRaytracer;

		TMCColorRGBA ambientLight = lightingContext.GetAmbientLight();

		// this attenuates the ambient light to look more like a distant, in case no other lights
		real norme= (shading.fChangedNormal[2]+shading.fChangedNormal[0]) * 0.5f;

		if (norme<0.0f) norme=-norme;
		norme= 0.5f*(norme-0.5f)+shading.fAmbient;

		if (norme<0.0f) norme=0.0f;
		if (norme>1.0f) norme=1.0f;

		// compute the base color
#if VERSIONNUMBER >= 0x050000
		result.fAmbient.R = shading.fColor.R * ambientLight.R * norme * fData.ambientLight + shading.fGlow.R;
		result.fAmbient.G = shading.fColor.G * ambientLight.G * norme * fData.ambientLight + shading.fGlow.G;
		result.fAmbient.B = shading.fColor.B * ambientLight.B * norme * fData.ambientLight + shading.fGlow.B;
#else
		result.fAmbientLight.R = shading.fColor.R * ambientLight.R * norme * fData.ambientLight + shading.fGlow.R;
		result.fAmbientLight.G = shading.fColor.G * ambientLight.G * norme * fData.ambientLight + shading.fGlow.G;
		result.fAmbientLight.B = shading.fColor.B * ambientLight.B * norme * fData.ambientLight + shading.fGlow.B;
#endif
		//  compute the lighting of each light

		const int32 lightCount = raytracer->GetLightCount();

		result.fDiffuseLight = TMCColorRGB::kBlack;
		result.fSpecularLight = TMCColorRGB::kBlack;

		for (int32 lightIndex = 0;lightIndex<lightCount; lightIndex++)
		{
			SpecularDiffuseLevel level;
			if (MCVerify(scene))
			{
				TMCCountedPtr<I3DShLightsource> light;
				scene->GetLightsourceByIndex(&light, lightIndex);
				if (MCVerify(light))
				{
					TMCCountedPtr<I3DShTreeElement> lightTree;
					lightTree = light->GetTreeElement();
					if (MCVerify(lightTree))
					{
						TMCString255 name;
						lightTree->GetName(name);
						for (uint32 levelIndex = 0; levelIndex < numlevels; levelIndex++)
						{
							SpecularDiffuseLevel currentLevel;
							levels.GetElem(levelIndex, currentLevel);

							if (!copied[levelIndex] && name == currentLevel.name)
							{
								level.fDiffuse = currentLevel.fDiffuse;
								level.fSpecular = currentLevel.fSpecular;
								level.shadowIntensity = currentLevel.shadowIntensity;
								copied[levelIndex] = true;
								levelIndex = numlevels;
							}

						}
					}
				}
			}
#if VERSIONNUMBER >= 0x050000
			if (raytracer->GetLightIntensity(directLighting
				,lightIndex,lightingContext,illumSettings.fShadowsOn, false))
#else
			if (raytracer->GetLightIntensity(directLighting
				,lightIndex,lightingContext,lightingContext.fIllumSettings.fShadowsOn))
#endif
			{
				DirectLighting dummyLighting;
#if VERSIONNUMBER >= 0x050000
				raytracer->GetLightIntensity(dummyLighting
					,lightIndex,lightingContext,false, false);
#else
				raytracer->GetLightIntensity(dummyLighting
					,lightIndex,lightingContext,false);
#endif
				if (level.shadowIntensity < 1.0f)
				{
					directLighting.fLightColor = directLighting.fLightColor * level.shadowIntensity + dummyLighting.fLightColor * (1.0f - level.shadowIntensity);
				}
				else if (level.shadowIntensity > 1.0f && directLighting.fLightColor != dummyLighting.fLightColor)
				{
					directLighting.fLightColor /= level.shadowIntensity;
				}


				result.fIsInShadow |= directLighting.fIsInShadow;
				const real coef= (shading.fChangedNormal * lightDirection) * shading.fLambert;

				if (coef>0)
				{
#if VERSIONNUMBER >= 0x070000
					if (illumSettings.fComputeSeparatedShadow)
					{
						result.fDiffuseLight.R += coef * noShadowlighting.R * level.fDiffuse;
						result.fDiffuseLight.G += coef * noShadowlighting.G * level.fDiffuse;
						result.fDiffuseLight.B += coef * noShadowlighting.B * level.fDiffuse;

						diffShadow.R += coef * lighting.R * level.fDiffuse;
						diffShadow.G += coef * lighting.G * level.fDiffuse;
						diffShadow.B += coef * lighting.B * level.fDiffuse;
					}
					else
					{
#endif
						result.fDiffuseLight.R += coef * lighting.R * level.fDiffuse;
						result.fDiffuseLight.G += coef * lighting.G * level.fDiffuse;
						result.fDiffuseLight.B += coef * lighting.B * level.fDiffuse;
#if VERSIONNUMBER >= 0x070000
					}
#endif
					const real S_R = lightingContext.fReflectDir * lightDirection;

					if (S_R > 0.0f)
					{
						const real yy=(0.001f+shading.fSpecularSize*shading.fSpecularSize)* kSpecularPowerCoef;

						const real specCoef= 2 * RealPow(S_R,yy);

#if VERSIONNUMBER >= 0x070000
						if (illumSettings.fComputeSeparatedShadow)
						{
							result.fSpecularLight.R += specCoef * noShadowlighting.R * level.fSpecular;
							result.fSpecularLight.G += specCoef * noShadowlighting.G * level.fSpecular;
							result.fSpecularLight.B += specCoef * noShadowlighting.B * level.fSpecular;

							specShadow.R += specCoef * lighting.R * level.fSpecular;
							specShadow.G += specCoef * lighting.G * level.fSpecular;
							specShadow.B += specCoef * lighting.B * level.fSpecular;
						}
						else
						{
#endif
							result.fSpecularLight.R += lighting.R * specCoef * level.fSpecular;
							result.fSpecularLight.G += lighting.G * specCoef * level.fSpecular;
							result.fSpecularLight.B += lighting.B * specCoef * level.fSpecular;
#if VERSIONNUMBER >= 0x070000
						}
#endif
					}
				}
			}
		}//end for lights
#if VERSIONNUMBER >= 0x070000
		if (illumSettings.fComputeSeparatedShadow)
		{
			// we can now compute the fLightShadow using the information from the lighting with and without shadow.

			TMCColorRGB shadowFactorDiffuse;
			for (int32 i=0; i<3; ++i)
			{
				if (result.fDiffuseLight[i] > 0) shadowFactorDiffuse[i] = diffShadow[i] / result.fDiffuseLight[i];
				else shadowFactorDiffuse[i] = (result.fIsInShadow ? 0 : 1);
			}

			// the result diffuse and specular light coorespond to the lighting without shadowing
			result.fDiffuseLight = result.fDiffuseLight % shading.fColor;
			result.fSpecularLight = result.fSpecularLight % shading.fSpecularColor;

			// this resColor is the diffuse and specular lighting with shadow. we use it to compute
			// fLightShadow
			TMCColorRGB resColor = (diffShadow % shading.fColor) + (specShadow % shading.fSpecularColor);

			const TMCColorRGB noShadowDiffuseSpecular = result.fDiffuseLight + result.fSpecularLight;
			for (int32 i=0; i<3; ++i)
			{
				if (noShadowDiffuseSpecular[i] > 1e-3f)
					result.fLightShadow[i] = resColor[i] / noShadowDiffuseSpecular[i];
				else
					result.fLightShadow[i] = shadowFactorDiffuse[i];
			}
		}
		else
		{
#endif
			// only the shading color is missing to have the correct result

			result.fDiffuseLight = result.fDiffuseLight % shading.fColor;
			result.fSpecularLight = result.fSpecularLight % shading.fSpecularColor;
#if VERSIONNUMBER >= 0x070000
			result.fLightShadow.Set(1, 1, 1);
		}
#endif
	}//end do direct
}

MCCOMErr SpecularDiffuseControl::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	// Find the scene
	if (MCVerify(shadingIn.fInstance)) 
	{
		TMCCountedPtr<I3DShTreeElement> tree;
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK) 
		{
			tree->GetScene(&scene);
		} 
	} 

	if (shader != NULL)
	{
		shader->DoShade(result, shadingIn);
	}

	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x070000)
void SpecularDiffuseControl::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor, real& ambientOcclusionFactor, const LightingContext& lightingContext,const ShadingOut& shading)
#else
void SpecularDiffuseControl::CalculateIndirectLighting(TMCColorRGB& indirectDiffuseColor,const LightingContext&	lightingContext,const ShadingOut& shading)
#endif
{
	if (shader != NULL)
	{
#if (VERSIONNUMBER >= 0x050000)
		if ((ChildOutput & kUseCalculateIndirectLighting) == kUseCalculateIndirectLighting)
		{
#if (VERSIONNUMBER >= 0x070000)
				(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
				(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
		else
		{
#if (VERSIONNUMBER >= 0x070000)
				defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, ambientOcclusionFactor, lightingContext, shading);
#else
				defaultlightingmodel->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		}
#else
		(reinterpret_cast<TBasicShader*>(shader.fObject))->CalculateIndirectLighting(indirectDiffuseColor, lightingContext, shading);
#endif
		indirectDiffuseColor*= fData.giLight;
	}
} 







MCCOMErr  SpecularDiffuseControl::HandleEvent(MessageID message, IMFResponder* source, void* data)

{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;


	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	//fill the list
	if (message == kMsg_CUIP_ComponentAttached)
	{
		TMCCountedPtr<IMFPart> listPart;
		TMCCountedPtr<IMFListPart> list;
		sourcePart->FindChildPartByID(&listPart, 'ShaL');

		ThrowIfNil(listPart);
		listPart->QueryInterface(IID_IMFListPart, (void**)&list);

		if (!scene)
		{
			TMCCountedPtr<ISceneDocument> sDoc;
			sDoc = gShell3DUtilities->GetLastActiveSceneDoc();
			ThrowIfNil(sDoc);
			sDoc->GetScene(&scene);
			ThrowIfNil(scene);

		}

		if (scene != NULL) {
			mergeLevelsWithScene();
			fillLightList(list);
		}

	}

	//sliders updated
	if ((sourceID == 'SPEC' || sourceID == 'DIFF' || sourceID == 'SHIN') && message == EMFPartMessage::kMsg_PartValueChanged)
	{
		//update the list from the UI
		TMCCountedPtr<IMFListPart> list;
		TMCCountedPtr<IMFPart> parentPart;
		TMCCountedPtr<IMFPart> listPart;
		sourcePart->GetPartParent(&parentPart);

		SpecularDiffuseLevel level;

		GetSliderValue(parentPart, 'DIFF', level.fDiffuse);
		GetSliderValue(parentPart, 'SPEC', level.fSpecular);
		GetSliderValue(parentPart, 'SHIN', level.shadowIntensity);

		if (fData.synchAllLights)
		{
			uint32 levelCount = levels.GetElemCount();

			for (uint32 levelIndex = 0; levelIndex < levelCount; levelIndex++)
			{
				levels[levelIndex].fDiffuse = level.fDiffuse;
				levels[levelIndex].fSpecular = level.fSpecular;
				levels[levelIndex].shadowIntensity = level.shadowIntensity;
			}
		}
		else
		{
			parentPart->FindChildPartByID(&listPart, 'ShaL');
			listPart->QueryInterface(IID_IMFListPart, (void**)&list);

			int32 selectedCell = 0;

			if (list->GetNextSelectedCell(selectedCell))
			{
				levels[selectedCell].fDiffuse = level.fDiffuse;
				levels[selectedCell].fSpecular = level.fSpecular;
				levels[selectedCell].shadowIntensity = level.shadowIntensity;
			}
		}
	}

	//selection changed
	if (sourceID == 'ShaL' && message == EMFPartMessage::kMsg_PartValueChanged)
	{
		//push the current values out to the UI
		TMCCountedPtr<IMFListPart> list;
		TMCCountedPtr<IMFPart> parentPart;
		TMCCountedPtr<IMFPart> diffPart;
		TMCCountedPtr<IMFPart> specPart;
		TMCCountedPtr<IMFPart> shadowPart;

		sourcePart->GetPartParent(&parentPart);
		parentPart->GetPartParent(&parentPart);
		parentPart->GetPartParent(&parentPart);

		parentPart->FindChildPartByID(&diffPart, 'DIFF');
		parentPart->FindChildPartByID(&specPart, 'SPEC');
		parentPart->FindChildPartByID(&shadowPart, 'SHIN');


		sourcePart->QueryInterface(IID_IMFListPart, (void**)&list);

		int32 selectedCell = 0;

		if (list->GetNextSelectedCell(selectedCell))
		{
			SetSliderValue(parentPart, 'DIFF', levels[selectedCell].fDiffuse);
			SetSliderValue(parentPart, 'SPEC', levels[selectedCell].fSpecular);
			SetSliderValue(parentPart, 'SHIN', levels[selectedCell].shadowIntensity);

			diffPart->Enable(true);
			specPart->Enable(true);
			shadowPart->Enable(true);
		}
		else
		{
			diffPart->Enable(false);
			specPart->Enable(false);
			shadowPart->Enable(false);
		}

	}

	return MC_S_OK;
}

void SpecularDiffuseControl::fillLightList(TMCCountedPtr<IMFListPart> list)
{
	TMCClassArray<TMCDynamicString> inNames;
	uint32 numlights = levels.GetElemCount();
	for (uint32 levelsIndex = 0; levelsIndex < levels.GetElemCount(); levelsIndex++)
	{
		SpecularDiffuseLevel level;
		levels.GetElem(levelsIndex, level);
		inNames.AddElem(level.name);
	}

	list->CreateNamedCells(inNames);
	if (inNames.GetElemCount() > 0)
	{
		list->SelectCell(0, true, false);
	}
	list = NULL;
}

void SpecularDiffuseControl::mergeLevelsWithScene()
{
	//spin through the list of existing lights
	TMCClassArray<SpecularDiffuseLevel> newlevels;
	TMCCountedPtr<I3DShLightsource> light;
	uint32 numlevels = levels.GetElemCount();
	TMCArray<boolean> copied(numlevels, true);

	uint32 numlights = scene->GetLightsourcesCount();
	for (uint32 lightIndex = 0; lightIndex < numlights; lightIndex++) 
	{
		TMCCountedPtr<I3DShTreeElement> tree;
		TMCString255 name;

		scene->GetLightsourceByIndex(&light, lightIndex);
		tree = light->GetTreeElement();
		tree->GetName(name);

		//spin through the list of existing levels looking for a 
		//matching name that has not already been copied
		SpecularDiffuseLevel newLevel;
		newLevel.name = name;

		for (uint32 levelIndex = 0; levelIndex < numlevels; levelIndex++)
		{
			SpecularDiffuseLevel currentLevel;
			levels.GetElem(levelIndex, currentLevel);

			if (!copied[levelIndex] && name == currentLevel.name)
			{
				newLevel.fDiffuse = currentLevel.fDiffuse;
				newLevel.fSpecular = currentLevel.fSpecular;
				newLevel.shadowIntensity = currentLevel.shadowIntensity;
				copied[levelIndex] = true;
				levelIndex = numlevels;
			}

		}

		newlevels.AddElem(newLevel);
	}

	levels = newlevels;
}


MCCOMErr SpecularDiffuseControl::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{

	int8 token[256];
	MCCOMErr result;
	uint32 levelIndex;

	result = stream->GetBegin();
	if (result) return result;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token))
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword)
		{
		case 'lnum':
			{
				int32 numlevels=0;
				numlevels = stream->GetInt32Token();
				levels.SetElemCount(numlevels);
			} break;
		case 'llev':
			{
				result = stream->GetBegin();
				if (result) return result;

				result=stream->GetNextToken(token);
				if (result) return result;

				while (!stream->IsEndToken(token))
				{
					int32 keyword;
					stream->CompactAttribute(token,&keyword);

					switch (keyword)
					{
					case 'lind':
						{
							levelIndex = stream->GetInt32Token();
						} break;
					case 'spec':
						{
							SpecularDiffuseLevel currentLevel;
							levels.GetElem(levelIndex, currentLevel);
							currentLevel.fSpecular = stream->GetReal32Token();
							levels.SetElem(levelIndex, currentLevel);
						} break;
					case 'diff':
						{
							SpecularDiffuseLevel currentLevel;
							levels.GetElem(levelIndex, currentLevel);
							currentLevel.fDiffuse = stream->GetReal32Token();
							levels.SetElem(levelIndex, currentLevel);
						} break;
					case 'shin':
						{
							SpecularDiffuseLevel currentLevel;
							levels.GetElem(levelIndex, currentLevel);
							currentLevel.shadowIntensity = stream->GetReal32Token();
							levels.SetElem(levelIndex, currentLevel);
						} break;
					case 'lnam':
						{
							SpecularDiffuseLevel currentLevel;
							levels.GetElem(levelIndex, currentLevel);
							currentLevel.name = stream->GetStringToken();
							levels.SetElem(levelIndex, currentLevel);
						} break;


					default:
						stream->SkipTokenData();
						break;
					};
					result=stream->GetNextToken(token);
				}
			}
			break;

		default:
			stream->SkipTokenData();
			break;
		};
		result=stream->GetNextToken(token);
	}

	return result;
}

MCCOMErr SpecularDiffuseControl::Write(IShTokenStream* stream)
{
	MCCOMErr result;
	uint32 numlevels = levels.GetElemCount();

	stream->PutInt32Attribute('lnum', numlevels);

	for (uint32 levelIndex = 0; levelIndex < numlevels; levelIndex++)
	{
		SpecularDiffuseLevel currentLevel;
		levels.GetElem(levelIndex, currentLevel);
		result=stream->PutKeywordAndBegin('llev');
		if (MCCOMERR_SEVERITY(result))
			return result;


		stream->PutInt32Attribute('lind', levelIndex);
		stream->PutReal32Attribute('spec', currentLevel.fSpecular);
		stream->PutReal32Attribute('diff', currentLevel.fDiffuse);
		stream->PutReal32Attribute('shin', currentLevel.shadowIntensity);

		stream->PutStringAttribute('lnam', currentLevel.name);


		stream->PutEnd();
	}

	return MC_S_OK;
}


MCCOMErr SpecularDiffuseControl::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}

	else if (MCIsEqualIID(riid, IID_SpecularDiffuseControl))
	{
		TMCCountedGetHelper<SpecularDiffuseControl> result(ppvObj);
		result = (SpecularDiffuseControl*)this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);	
}

uint32 SpecularDiffuseControl::AddRef()
{
	return TBasicShader::AddRef();
}

void SpecularDiffuseControl::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	SpecularDiffuseControl* clone = new SpecularDiffuseControl();
	result = (IExDataExchanger*)clone;

	clone->CopyDataFrom(this);

	clone->SetControllingUnknown(pUnkOuter);
}

void SpecularDiffuseControl::CopyDataFrom(const SpecularDiffuseControl* source)
{
	levels = source->levels;
	scene = source->scene;
	fData = source->fData;
}