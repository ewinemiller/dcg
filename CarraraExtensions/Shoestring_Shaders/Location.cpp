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

//#include "math.h"
#include "Location.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Location::ExtensionDataChanged()
{


	for (int32 i = 0; i<3; i++){

		if (LocationData.Axes[i].Direction == kGreater){
			Axes[i].Axis = LocationData.Axes[i].Axis;
			Axes[i].Direction = 1;
			Axes[i].Bound1 = (LocationData.Axes[i].Bound1 >= LocationData.Axes[i].Bound2) 
				? LocationData.Axes[i].Bound2 : LocationData.Axes[i].Bound1;
			Axes[i].Bound2 = (LocationData.Axes[i].Bound1 >= LocationData.Axes[i].Bound2) 
				? LocationData.Axes[i].Bound1 : LocationData.Axes[i].Bound2;
		}
		else { // less than
			Axes[i].Axis = LocationData.Axes[i].Axis;
			Axes[i].Direction = 0;
			Axes[i].Bound1 = (LocationData.Axes[i].Bound1 >= LocationData.Axes[i].Bound2)
				? LocationData.Axes[i].Bound1 : LocationData.Axes[i].Bound2;
			Axes[i].Bound2 = (LocationData.Axes[i].Bound1 >= LocationData.Axes[i].Bound2)
				? LocationData.Axes[i].Bound2 : LocationData.Axes[i].Bound1;
		}
	}

	MCCOMErr result = MC_S_OK;

	if (LocationData.fMixShade)
	result = LocationData.fMixShade->QueryInterface(IID_I3DShShader, (void**)&fMixShader);
	
	Imp_Out = 0;

	if (fMixShader != NULL)
		Imp_Out = fMixShader->GetImplementedOutput(); 

	return result;
}

Location::Location()		// Initialize the public data
{
	for (int32 i = 0; i<3; i++){
    	LocationData.Axes[i].Bound1 = 0;
    	LocationData.Axes[i].Bound2 = 0;
		LocationData.Axes[i].Direction = kGreater;
		LocationData.Axes[i].Axis = 0;
	}
	LocationData.Mode = kOr_mode;
	LocationData.Coord = kGlobal;
}
	
void* Location::GetExtensionDataBuffer()
{
	return ((void*) &(LocationData));
}

boolean Location::IsEqualTo(I3DExShader* aShader)		// Compare two Location shaders
{
	int32 equality = 1;	
	
	if (LocationData.Axes != ((Location*)aShader)->LocationData.Axes ||
		LocationData.Coord != ((Location*)aShader)->LocationData.Coord ||
		LocationData.Shader_factor != ((Location*)aShader)->LocationData.Shader_factor ||
		LocationData.Mode != ((Location*)aShader)->LocationData.Mode ||
		LocationData.Use_shader != ((Location*)aShader)->LocationData.Use_shader ||
		LocationData.fMixShade != ((Location*)aShader)->LocationData.fMixShade 
		)
			equality = 0;

	return equality;
		  
}

MCCOMErr Location::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Location::GetImplementedOutput()
{
	return  kUsesGetValue;	
}

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Location::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Location::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real MCCOMAPI Location::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#endif

{
	int32 i = 0;
	real result_temp[3];
	result = 1;
	fullArea = false;
	luma = 0;

		// Add in mix shader, if requested
	if (LocationData.Use_shader) {
		if (!fMixShader){  // if the mix shader is null, just return black.
			luma = 0;
		}
		else {
			TMCColorRGBA Color;
			if (Imp_Out & kUsesGetColor){		// get color implemented
				fMixShader->GetColor(Color, fullArea, shadingIn);
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
			}
			else if (Imp_Out & kUsesGetValue)  //  or get value implemented
				fMixShader->GetValue(luma, fullArea, shadingIn);
			else 
				luma = 0;		// no useful data back from mix shader, return black
		}
		luma = luma > 1 ? last_luma : luma;
		luma = luma * LocationData.Shader_factor;
		last_luma = luma;	// in case it returns over 1.0, a bad value, preserve the last good value
	}

	for (i = 0; i<3; i++){
		result_temp[i] = 0;
		if (Axes[i].Axis)
			result_temp[i] = DoAxis(Axes[i].Bound1, Axes[i].Bound2, 
			(LocationData.Coord == kGlobal)? shadingIn.fPoint[i]: shadingIn.fPointLoc[i],
			Axes[i].Direction, luma);
	}
	if (LocationData.Mode == kOr_mode){
		for (i = 0; i<3; i++){
			if (Axes[i].Axis && (result_temp[i] < result)){
				result = result_temp[i];
			}
		}
	}
	else {
		for (i = 0; i<3; i++){
			if (Axes[i].Axis){
				if (result_temp[i] < 1.0){
					result = result_temp[i];
				}
				else {
					result = 1.0;
					i = 3;
				}
			}
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

// 

real32 Location::DoAxis(real32 Start, real32 End, real32 coord, int32 Greater, real32 rand)
{
	real32 result = 1;
	real32 shift = 0;
	
	coord = coord + (fabs(Start - End) * rand)/100.0;

	if (Greater){
		if (coord >= End)
			result = 0;
		if (coord <= Start)
			result = 1;
		if ((coord < End) & (coord > Start))
			result = 1 - ((coord - Start)/(End - Start));
	}
	else { // Less
		if (coord <= End)
			result = 0;
		if (coord >= Start)
			result = 1;
		if ((coord > End) & (coord < Start))
			result = (coord - End)/(Start - End);
	}
	return result;
}