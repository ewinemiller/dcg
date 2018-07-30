/*  Wirerender utilities for Carrara
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
#include "copyright.h"
#include "ToonEnabled.h"
#include "MCCountedPtrhelper.h"
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "commessages.h"
#include "IMFResponder.h"
#include "COM3DUtilities.h"
#include "ISceneDocument.h"
#include "dcguiutil.h"
#include "I3DShTreeElement.h"
#include "I3DShObject.h"
#include "DCGStreamHelper.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_ToonEnabled(R_CLSID_ToonEnabled);
const MCGUID IID_ToonEnabled(R_IID_ToonEnabled);
const MCGUID CLSID_WireframeEnabled(R_CLSID_WireframeEnabled);
const MCGUID IID_WireframeEnabled(R_IID_WireframeEnabled);
#else
const MCGUID CLSID_ToonEnabled={R_CLSID_ToonEnabled};
const MCGUID IID_ToonEnabled={R_IID_ToonEnabled};
const MCGUID CLSID_WireframeEnabled={R_CLSID_WireframeEnabled};
const MCGUID IID_WireframeEnabled={R_IID_WireframeEnabled};
#endif

#define UI_ELEM_OVERRIDE_SELECTED_DOMAIN 'OVSD'
#define UI_ELEM_USE_OBJECT_COLOR 'OBJC'
#define UI_ELEM_LINE_WIDTH 'line'
#define UI_ELEM_LINE_EFFECT 'LIVL'
#define UI_ELEM_LINE_COLOR 'LICO'
#define UI_ELEM_DO_DEPTH 'DEPT'
#define UI_ELEM_DEPTH 'DIST'
#define UI_ELEM_EDGE 'EDGT'
#define UI_ELEM_OVERDRAW 'OVDR'
#define UI_ELEM_DOMAIN_BOUNDARY_EDGES 'DOLI'
#define UI_ELEM_REMOVE_HIDDEN_LINES 'RHLN'

#define UI_ELEM_SHADER_LIST 'ShaL' 

#define UI_ELEM_LEVEL_INDEX 'lind'
#define UI_ELEM_NAME 'lnam'
#define UI_ELEM_GO_UP 'goup'
#define UI_ELEM_GO_DOWN 'godn'
#define SAVE_KEY_SORT_ORDER 'sord'

MCCOMErr ToonEnabled::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}
	else
	if (MCIsEqualIID(riid, IID_I3DExDataComponent))
	{
		TMCCountedGetHelper<I3DExDataComponent> result(ppvObj);
		result = (I3DExDataComponent*)this;
		return MC_S_OK;
	}
	else
	if ((MCIsEqualIID(riid, IID_ToonEnabled))&&(lRenderStyle == rsToon))
	{
		TMCCountedGetHelper<ToonEnabled> result(ppvObj);
		result = (ToonEnabled*)this;
		return MC_S_OK;
	}
	else
	if ((MCIsEqualIID(riid, IID_WireframeEnabled))&&(lRenderStyle == rsWireFrame))
	{
		TMCCountedGetHelper<ToonEnabled> result(ppvObj);
		result = (ToonEnabled*)this;
		return MC_S_OK;
	}
	return TBasicDataExchanger::QueryInterface(riid, ppvObj);
}

ToonEnabled::ToonEnabled(RenderStyle renderStyle) {
	lRenderStyle = renderStyle;
	ignoreSelectionChange = false;
	}

ToonEnabled::~ToonEnabled() {
	
	}

void* ToonEnabled::GetExtensionDataBuffer()
{
	return &fData;
}

boolean ToonEnabled::IsEqual(IExDataExchanger* otherDataExchanger)
{
	TMCCountedPtr<ToonEnabled> otherToonEnabled;

	if (lRenderStyle == rsWireFrame) {
		otherDataExchanger->QueryInterface(IID_WireframeEnabled, (void**)&otherToonEnabled);
	}
	else if (lRenderStyle == rsToon) {
		otherDataExchanger->QueryInterface(IID_ToonEnabled, (void**)&otherToonEnabled);
	}
	ThrowIfNil(otherToonEnabled);
	return (fData == otherToonEnabled->fData && tree == otherToonEnabled->tree);
}


uint32 ToonEnabled::AddRef()
{
	return TBasicDataComponent::AddRef();
}

void ToonEnabled::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	ToonEnabled* clone = new ToonEnabled(static_cast<RenderStyle>(lRenderStyle));
	result = (IExDataExchanger*)clone;

	clone->CopyDataFrom(this);

	clone->SetControllingUnknown(pUnkOuter);
}

void ToonEnabled::CopyDataFrom(const ToonEnabled* source)
{
	toonSettings = source->toonSettings;
	tree = source->tree;
	fData = source->fData;
}

boolean  ToonEnabled::IsActive(I3DShTreeElement* tree)
{
	this->tree = tree;
	return true;
}

void ToonEnabled::enableDomainControls(TMCCountedPtr<IMFPart> parentPart, boolean enabled)
{
	SetPartEnabled(parentPart, UI_ELEM_OVERRIDE_SELECTED_DOMAIN, enabled);
	SetPartEnabled(parentPart, UI_ELEM_USE_OBJECT_COLOR, enabled);
	SetPartEnabled(parentPart, UI_ELEM_LINE_WIDTH, enabled);
	SetPartEnabled(parentPart, UI_ELEM_LINE_EFFECT, enabled);
	SetPartEnabled(parentPart, UI_ELEM_LINE_COLOR, enabled);
	SetPartEnabled(parentPart, UI_ELEM_DO_DEPTH, enabled);
	SetPartEnabled(parentPart, UI_ELEM_DEPTH, enabled);
	SetPartEnabled(parentPart, UI_ELEM_EDGE, enabled);
	SetPartEnabled(parentPart, UI_ELEM_OVERDRAW, enabled);
	SetPartEnabled(parentPart, UI_ELEM_DOMAIN_BOUNDARY_EDGES, enabled);
	SetPartEnabled(parentPart, UI_ELEM_REMOVE_HIDDEN_LINES, enabled);
}

MCCOMErr  ToonEnabled::HandleEvent(MessageID message, IMFResponder* source, void* data)

{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;


	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	//fill the list
	if (message == kMsg_CUIP_ComponentAttached && tree != NULL) {
		TMCCountedPtr<IMFPart> listPart;
		TMCCountedPtr<IMFListPart> list;
		sourcePart->FindChildPartByID(&listPart, UI_ELEM_SHADER_LIST);

		ThrowIfNil(listPart);
		listPart->QueryInterface(IID_IMFListPart, (void**)&list);

		mergeLevelsWithScene();
		fillShadingDomainList(list, 0);
		if (toonSettings.GetElemCount() == 0)
		{
			enableDomainControls(sourcePart, false);

			listPart->Enable(false);
		}

	}
	//rerank
	if (
		(sourceID == UI_ELEM_GO_UP
		||sourceID == UI_ELEM_GO_DOWN)
		
		&& message == EMFPartMessage::kMsg_PartValueChanged)
	{
		TMCCountedPtr<IMFListPart> list;
		TMCCountedPtr<IMFPart> parentPart;
		TMCCountedPtr<IMFPart> listPart;
		sourcePart->GetPartParent(&parentPart);
		int32 selectedCell = 0;

		parentPart->FindChildPartByID(&listPart, UI_ELEM_SHADER_LIST);
		listPart->QueryInterface(IID_IMFListPart, (void**)&list);

		if (list->GetNextSelectedCell(selectedCell))
		{
			int32 step = 0;
			if (sourceID == UI_ELEM_GO_UP)
			{
				step = -1;
			}
			else
			{
				step = 1;
			}
			selectedCell = toonSettings.reRank(selectedCell, step);

			ignoreSelectionChange = true;
			fillShadingDomainList(list, selectedCell);
			ignoreSelectionChange = false;
		}

	}

	//values updated
	if (
		(sourceID == UI_ELEM_LINE_COLOR
		||sourceID == UI_ELEM_LINE_EFFECT
		||sourceID == UI_ELEM_LINE_WIDTH
		||sourceID == UI_ELEM_USE_OBJECT_COLOR
		||sourceID == UI_ELEM_DO_DEPTH
		||sourceID == UI_ELEM_DEPTH
		||sourceID == UI_ELEM_OVERDRAW
		||sourceID == UI_ELEM_OVERRIDE_SELECTED_DOMAIN
		||sourceID == UI_ELEM_EDGE
		||sourceID == UI_ELEM_DOMAIN_BOUNDARY_EDGES
		||sourceID == UI_ELEM_REMOVE_HIDDEN_LINES)
		
		&& message == EMFPartMessage::kMsg_PartValueChanged)
	{
		//update the list from the UI
		TMCCountedPtr<IMFListPart> list;
		TMCCountedPtr<IMFPart> parentPart;
		TMCCountedPtr<IMFPart> listPart;
		sourcePart->GetPartParent(&parentPart);
		sourcePart->GetPartParent(&parentPart);
		if (sourceID == UI_ELEM_DO_DEPTH)
		{
			parentPart->GetPartParent(&parentPart);
		}

		ShadingDomainLineLevels level;


		GetSliderValue(parentPart, UI_ELEM_LINE_WIDTH, level.lineWidth);
		GetSliderValue(parentPart, UI_ELEM_LINE_EFFECT, level.lineEffect);
		GetCheckValue(parentPart, UI_ELEM_OVERRIDE_SELECTED_DOMAIN, level.overrideSettings);
		GetCheckValue(parentPart, UI_ELEM_USE_OBJECT_COLOR, level.useObjectColor);
		GetColorChipValue(parentPart, UI_ELEM_LINE_COLOR, level.color);
		GetSliderValue(parentPart, UI_ELEM_DEPTH, level.depthCue);
		GetSliderValue(parentPart, UI_ELEM_EDGE, level.vectorAngle);
		GetSliderValue(parentPart, UI_ELEM_OVERDRAW, level.overDraw);
		GetCheckValue(parentPart, UI_ELEM_DO_DEPTH, level.doDepthCueing);
		GetCheckValue(parentPart, UI_ELEM_DOMAIN_BOUNDARY_EDGES, level.doDomainBoundaryEdges);
		GetCheckValue(parentPart, UI_ELEM_REMOVE_HIDDEN_LINES, level.removeHiddenLines);

		parentPart->FindChildPartByID(&listPart, UI_ELEM_SHADER_LIST);
		listPart->QueryInterface(IID_IMFListPart, (void**)&list);

		int32 selectedCell = 0;

		if (list->GetNextSelectedCell(selectedCell))
		{

			if (fData.synchAll)
			{
				uint32 levelCount = toonSettings.GetElemCount();
				for (uint32 levelIndex = 0; levelIndex < levelCount; levelIndex++)
				{
					level.name = toonSettings.getDomainByIndex(levelIndex).name;
					toonSettings.SetElem(levelIndex, level);
				}
			}
			else
			{
				uint32 index = toonSettings.getRankedDomainIndex(selectedCell);
				level.name = toonSettings.getDomainByIndex(index).name;
				toonSettings.SetElem(index, level);
			}
			ignoreSelectionChange = true;
			fillShadingDomainList(list, selectedCell);
			ignoreSelectionChange = false;
		}
	}

	//selection changed
	if (sourceID == UI_ELEM_SHADER_LIST && message == EMFPartMessage::kMsg_PartValueChanged 
		&& !ignoreSelectionChange)
	{
		//push the current values out to the UI
		TMCCountedPtr<IMFListPart> list;
		TMCCountedPtr<IMFPart> parentPart;

		sourcePart->GetPartParent(&parentPart);
		parentPart->GetPartParent(&parentPart);
		parentPart->GetPartParent(&parentPart);

		sourcePart->QueryInterface(IID_IMFListPart, (void**)&list);

		int32 selectedCell = 0;

		if (list->GetNextSelectedCell(selectedCell))
		{
			ShadingDomainLineLevels& currentLevel = toonSettings.getDomainByRank(selectedCell);

			SetSliderValue(parentPart, UI_ELEM_LINE_WIDTH, currentLevel.lineWidth);
			SetSliderValue(parentPart, UI_ELEM_LINE_EFFECT, currentLevel.lineEffect);
			SetCheckValue(parentPart, UI_ELEM_OVERRIDE_SELECTED_DOMAIN, currentLevel.overrideSettings);
			SetCheckValue(parentPart, UI_ELEM_USE_OBJECT_COLOR, currentLevel.useObjectColor);
			SetColorChipValue(parentPart, UI_ELEM_LINE_COLOR, currentLevel.color);
			SetCheckValue(parentPart, UI_ELEM_DO_DEPTH, currentLevel.doDepthCueing);
			SetCheckValue(parentPart, UI_ELEM_DOMAIN_BOUNDARY_EDGES, currentLevel.doDomainBoundaryEdges);
			SetCheckValue(parentPart, UI_ELEM_REMOVE_HIDDEN_LINES, currentLevel.removeHiddenLines);
			SetSliderValue(parentPart, UI_ELEM_OVERDRAW, currentLevel.overDraw);
			SetSliderValue(parentPart, UI_ELEM_EDGE, currentLevel.vectorAngle);
			SetSliderValue(parentPart, UI_ELEM_DEPTH, currentLevel.depthCue);
			enableDomainControls(parentPart, true);
		}
		else
		{
			enableDomainControls(parentPart, false);
		}

	}

	return MC_S_OK;
}

void ToonEnabled::fillShadingDomainList(TMCCountedPtr<IMFListPart> list, uint32 selection)
{
	TMCClassArray<TMCDynamicString> inNames;
	uint32 domainCount = toonSettings.GetElemCount();
	for (uint32 levelsIndex = 0; levelsIndex < domainCount; levelsIndex++)
	{
		TMCString255 currentState;
		ShadingDomainLineLevels& level = toonSettings.getDomainByRank(levelsIndex);
		if (level.overrideSettings == true)
		{
			 currentState = " - overridden";
		}
		else
		{
			 currentState = "";
		}
		inNames.AddElem(level.name + currentState);
	}

	list->CreateNamedCells(inNames);
	if (inNames.GetElemCount() > 0)
	{
		list->SelectCell(selection, true, false);
	}
	list = NULL;
}

void ToonEnabled::mergeLevelsWithScene()
{
	//spin through the list of existing shading domains
	ToonSettings newToonSettings;

	TMCCountedPtr<I3DShInstance> instance;
	TMCCountedPtr<I3DShObject> object;

	uint32 numlevels = toonSettings.GetElemCount();

	tree->QueryInterface(IID_I3DShInstance, (void**)&instance);
	
	if(instance)
	{
		instance->Get3DObject(&object);

		ThrowIfNil(object);
		uint32 uvSpaceCount = object->GetUVSpaceCount();
		if (uvSpaceCount == 0)
		{
			uvSpaceCount = 1;
		}

		newToonSettings.SetElemCount(uvSpaceCount);
		TMCArray<boolean> copied(numlevels, true);
		copied.FillWithValue(false);
		TMCArray<uint32> oldRank(uvSpaceCount, UNSPECIFIED_SORT_ORDER);
		oldRank.FillWithValue(UNSPECIFIED_SORT_ORDER);
		TMCArray<boolean> ranked(uvSpaceCount, true);
		ranked.FillWithValue(false);

		for (uint32 uvSpaceIndex = 0; uvSpaceIndex < uvSpaceCount; uvSpaceIndex++) 
		{
			TMCString255 name;
			UVSpaceInfo uvSpaceInfo;
			object->GetUVSpace(uvSpaceIndex, &uvSpaceInfo);
			
			name = uvSpaceInfo.fName;

			if (name.Length() == 0)
			{
				name = "Unnamed Shading Domain";
			}

			//spin through the list of existing levels looking for a 
			//matching name that has not already been copied
			ShadingDomainLineLevels newLevel;
			newLevel.name = name;

			for (uint32 levelIndex = 0; levelIndex < numlevels; levelIndex++)
			{
				ShadingDomainLineLevels& currentLevel = toonSettings.getDomainByIndex(levelIndex);

				if (!copied[levelIndex] && name == currentLevel.name)
				{
					newLevel = currentLevel;
					copied[levelIndex] = true;
					oldRank[uvSpaceIndex] = toonSettings.getRankByIndex(levelIndex);
					levelIndex = numlevels;
				}

			}

			newToonSettings.SetElem(uvSpaceIndex, newLevel);
		}

		//now spin through and rerank
		uint32 currentOldRank = 0;
		for (uint32 uvSpaceIndex = 0; uvSpaceIndex < uvSpaceCount; uvSpaceIndex++) 
		{
			uint32 currentRank = UNSPECIFIED_SORT_ORDER;
			uint32 lowestRankIndex = UNSPECIFIED_SORT_ORDER;

			//look through the old ranks and find the lowest rank item available
			for (uint32 oldRankIndex = 0; oldRankIndex < uvSpaceCount; oldRankIndex++)
			{
				//if this item ranked lower than our current one
				if (!ranked[oldRankIndex] && oldRank[oldRankIndex] < currentRank)
				{
					currentRank = oldRank[oldRankIndex];
					lowestRankIndex = oldRankIndex;
				}
			}
			if (lowestRankIndex != UNSPECIFIED_SORT_ORDER)
			{
				newToonSettings.setRankedDomainIndex(uvSpaceIndex, lowestRankIndex);
				ranked[lowestRankIndex] = true;
			}
			//ran out of ranked items, put the remainder on the list and then drop out of the loop
			else
			{
				uint32 offset = 0;
				for (uint32 oldRankIndex = 0; oldRankIndex < uvSpaceCount; oldRankIndex++)
				{
					if (!ranked[oldRankIndex] && oldRank[oldRankIndex] == UNSPECIFIED_SORT_ORDER)
					{
						newToonSettings.setRankedDomainIndex(uvSpaceIndex + offset, oldRankIndex);
						offset++;
					}
				}


				uvSpaceIndex = uvSpaceCount;
			}
		}
	}

	toonSettings = newToonSettings;
}



MCCOMErr ToonEnabled::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
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
				numlevels = DCGStreamHelper::GetInt32(stream);
				toonSettings.SetElemCount(numlevels);
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
					case UI_ELEM_LEVEL_INDEX:
						{
							levelIndex = DCGStreamHelper::GetInt32(stream);
						} break;
					case UI_ELEM_NAME:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.name = DCGStreamHelper::GetString(stream);
						} break;
					case UI_ELEM_OVERRIDE_SELECTED_DOMAIN:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.overrideSettings = DCGStreamHelper::GetBoolean(stream);
						} break;
					case UI_ELEM_LINE_EFFECT:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.lineEffect = DCGStreamHelper::GetReal32(stream);
						} break;
					case UI_ELEM_USE_OBJECT_COLOR:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.useObjectColor = DCGStreamHelper::GetBoolean(stream);
						} break;
					case UI_ELEM_LINE_COLOR:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							DCGStreamHelper::GetRGBColor(stream, currentLevel.color);
						} break;
					case UI_ELEM_LINE_WIDTH:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.lineWidth = DCGStreamHelper::GetReal32(stream);
						} break;
					case UI_ELEM_DO_DEPTH:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.doDepthCueing = DCGStreamHelper::GetBoolean(stream);
						} break;
					case UI_ELEM_DOMAIN_BOUNDARY_EDGES:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.doDomainBoundaryEdges = DCGStreamHelper::GetBoolean(stream);
						} break;
					case UI_ELEM_REMOVE_HIDDEN_LINES:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.removeHiddenLines = DCGStreamHelper::GetBoolean(stream);
						} break;
					case UI_ELEM_DEPTH:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.depthCue = DCGStreamHelper::GetReal32(stream);
						} break;
					case UI_ELEM_EDGE:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.vectorAngle = DCGStreamHelper::GetReal32(stream);
						} break;
					case UI_ELEM_OVERDRAW:
						{
							ShadingDomainLineLevels& currentLevel 
								= toonSettings.getDomainByIndex(levelIndex);
							currentLevel.overDraw = DCGStreamHelper::GetReal32(stream);
						} break;
					case SAVE_KEY_SORT_ORDER:
						{
							toonSettings.setRankedDomainIndex(levelIndex, DCGStreamHelper::GetInt32(stream));
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

MCCOMErr ToonEnabled::Write(IShTokenStream* stream)
{
	MCCOMErr result;
	uint32 numlevels = toonSettings.GetElemCount();

	DCGStreamHelper::PutInt32(stream,'lnum', numlevels);

	for (uint32 levelIndex = 0; levelIndex < numlevels; levelIndex++)
	{
		ShadingDomainLineLevels& currentLevel = toonSettings.getDomainByIndex(levelIndex);
		result=stream->PutKeywordAndBegin('llev');
		if (MCCOMERR_SEVERITY(result))
			return result;

		DCGStreamHelper::PutInt32(stream,UI_ELEM_LEVEL_INDEX, levelIndex);
		DCGStreamHelper::PutString(stream, UI_ELEM_NAME, currentLevel.name);
		DCGStreamHelper::PutBoolean(stream,UI_ELEM_OVERRIDE_SELECTED_DOMAIN, currentLevel.overrideSettings);
		DCGStreamHelper::PutReal32(stream,UI_ELEM_LINE_EFFECT, currentLevel.lineEffect);
		DCGStreamHelper::PutBoolean(stream,UI_ELEM_USE_OBJECT_COLOR, currentLevel.useObjectColor);
		DCGStreamHelper::PutRGBColor(stream,UI_ELEM_LINE_COLOR, currentLevel.color);
		DCGStreamHelper::PutReal32(stream,UI_ELEM_LINE_WIDTH, currentLevel.lineWidth);
		DCGStreamHelper::PutReal32(stream,UI_ELEM_OVERDRAW, currentLevel.overDraw);
		DCGStreamHelper::PutReal32(stream,UI_ELEM_DEPTH, currentLevel.depthCue);
		DCGStreamHelper::PutReal32(stream,UI_ELEM_EDGE, currentLevel.vectorAngle);
		DCGStreamHelper::PutBoolean(stream,UI_ELEM_DO_DEPTH, currentLevel.doDepthCueing);
		DCGStreamHelper::PutBoolean(stream,UI_ELEM_DOMAIN_BOUNDARY_EDGES, currentLevel.doDomainBoundaryEdges);
		DCGStreamHelper::PutBoolean(stream,UI_ELEM_REMOVE_HIDDEN_LINES, currentLevel.removeHiddenLines);
		DCGStreamHelper::PutInt32(stream,SAVE_KEY_SORT_ORDER, toonSettings.getRankedDomainIndex(levelIndex));

		stream->PutEnd();
	}

	return MC_S_OK;
}
