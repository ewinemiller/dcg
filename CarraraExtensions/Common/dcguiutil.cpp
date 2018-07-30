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
#include "IMFPart.h"
#include "MCColorRGBA.h"
#include "MCCountedPtr.h"

void SetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, real32 value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->SetValue(&value, kReal32ValueType, true, false);
}

void SetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, const TVector2& value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->SetValue(&value, kVector2ValueType, true, false);
}


void SetCheckValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->SetValue(&value, kBooleanValueType, true, false);
}

void SetColorChipValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, const TMCColorRGBA& value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->SetValue(&value, kColorRGBValueType, true, false);
}

void GetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, real32& value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->GetValue(&value, kReal32ValueType);
}

void GetCheckValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean& value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->GetValue(&value, kBooleanValueType);
}

void GetColorChipValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, TMCColorRGBA& value)
{
	TMCCountedPtr<IMFPart> tempPart;
	parentPart->FindChildPartByID(&tempPart, inPartID);
	if (tempPart)
		tempPart->GetValue(&value, kColorRGBValueType);
}

void GetParentPart(TMCCountedPtr<IMFPart> thisPart, TMCCountedPtr<IMFPart> parentPart)
{
	thisPart->GetPartParent(&parentPart);
}

void GetTopPart(TMCCountedPtr<IMFPart> thisPart, TMCCountedPtr<IMFPart>& topPart)
{
	TMCCountedPtr<IMFPart> parentPart = thisPart;
	topPart = thisPart;
	parentPart->GetPartParent(&parentPart);
	while(parentPart)
	{
		topPart = parentPart;
		parentPart->GetPartParent(&parentPart);
	}
}

void SetPartEnabled(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean& enabled)
{
	TMCCountedPtr<IMFPart> enabPart;
	parentPart->FindChildPartByID(&enabPart, inPartID);
	if (enabPart)
		enabPart->Enable(enabled);
}
