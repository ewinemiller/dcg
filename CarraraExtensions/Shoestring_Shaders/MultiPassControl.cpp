/*  Shoestring Shaders - plug-in for Carrara
    Copyright (C) 2003  Mark DesMarais

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
#include "math.h"
#include "MultiPassControl.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr MultiPassControl::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;

	if (MultiPassControlData.fInputShader)
	result = MultiPassControlData.fInputShader->QueryInterface(IID_I3DShShader, (void**)&fInputShader);
	
	Imp_Out = 0;

	if (fInputShader != NULL)
		Imp_Out = fInputShader->GetImplementedOutput();  

	return result;
}
MultiPassControl::MultiPassControl()		// Initialize the public data
{	
	MultiPassControlData.fInputShader = NULL;
	MultiPassControlData.Level = kRef;
}
	
void* MultiPassControl::GetExtensionDataBuffer()
{
	return ((void*) &(MultiPassControlData));
}

boolean MultiPassControl::IsEqualTo(I3DExShader* aShader)		// Compare two MultiPassControl shaders
{
	int32 equality = 1;	
	if (
		MultiPassControlData.Level != ((MultiPassControl*)aShader)->MultiPassControlData.Level ||
		MultiPassControlData.fInputShader != ((MultiPassControl*)aShader)->MultiPassControlData.fInputShader 
		)
			equality = 0;
	return equality;
}

MCCOMErr MultiPassControl::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;

	return MC_S_OK;
}

EShaderOutput MultiPassControl::GetImplementedOutput()
{
	return  kUsesGetValue;	
}
  
#if (VERSIONNUMBER >= 0x040000)
boolean MultiPassControl::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr MultiPassControl::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr MultiPassControl::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real MCCOMAPI MultiPassControl::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;
	ShadingOut fetch;

	if (MultiPassControlData.Level == kRef)	{
		if (!fInputShader)  // if the shader is null, just return black.
				return MC_S_OK;
		else {
			if (Imp_Out & kUsesGetColor) {										// get color implemented
				fInputShader->GetColor(Color, fullArea, shadingIn);
				result = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
			}
			else if (Imp_Out & kUsesGetValue)									//  or get value implemented
				fInputShader->GetValue(result, fullArea, shadingIn);
			else if (Imp_Out & kUsesDoShade)	{								// do shade
				fInputShader->DoShade(fetch, shadingIn);
				result = (fetch.fColor.R*0.3 + fetch.fColor.G*0.59+ fetch.fColor.B*0.11);
			}
			else 
				result = 0;		// no useful data back from shader, return black
		}
	}
	else {
		switch (MultiPassControlData.Level){
			case kOne:
				result = 0.05f;
				break;
			case kTwo:
				result = 0.15f;
				break;
			case kThree:
				result = 0.25f;
				break;
			case kFour:
				result = 0.35f;
				break;
			case kFive:
				result = 0.45f;
				break;
			case kSix:
				result = 0.55f;
				break;
			case kSeven:
				result = 0.65f;
				break;
			case kEight:
				result = 0.75f;
				break;
			case kNine:
				result = 0.85f;
				break;
			case kTen:
				result = 0.95f;
				break;
			default:
				result = 0;
		}
	}



#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}