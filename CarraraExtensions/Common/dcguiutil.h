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
#include "MCColorRGB.h"

extern void SetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, real32 value);
extern void SetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, const TVector2& value);
extern void SetCheckValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean value);
extern void GetSliderValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, real32& value);
extern void GetCheckValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean& value);
extern void GetParentPart(TMCCountedPtr<IMFPart> thisPart, TMCCountedPtr<IMFPart>& parentPart);
extern void GetTopPart(TMCCountedPtr<IMFPart> thisPart, TMCCountedPtr<IMFPart>& topPart);
extern void SetPartEnabled(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, boolean& enabled);
extern void SetColorChipValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, const TMCColorRGBA& value);
extern void GetColorChipValue(TMCCountedPtr<IMFPart> parentPart, IDType inPartID, TMCColorRGBA& value);


