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
#include "Spoke.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Spoke::ExtensionDataChanged() {

	segment_size = 360.0/SpokeData.Number;	
	seg_root = segment_size/2;						// find the root (middle) of the segment
	falloff_point = 1 - SpokeData.Circ_Falloff;

	MCCOMErr result = MC_S_OK;

	return result;
}

Spoke::Spoke()	{	// Initialize the public data
	SpokeData.Ulocation = 0.5;
	SpokeData.Vlocation = 0.5;
	SpokeData.Height = 1.0;
	SpokeData.Width = 30;
	SpokeData.Number = 4;
	SpokeData.Circ_Falloff = 0.2f;
	SpokeData.Radial_Start = 0.02f;
	SpokeData.Radial_End = 1.0;
	SpokeData.Start_Radial_Falloff = 0.1f;
	SpokeData.End_Radial_Falloff = 0.5f;
	SpokeData.ConstantWidth = 0;
	SpokeData.FillCenter = 0;
	SpokeData.Offset = 0;
	SpokeData.Taper = 0;
}
	
void* Spoke::GetExtensionDataBuffer(){
	return ((void*) &(SpokeData));
}

#if (VERSIONNUMBER >= 0x040000)
boolean Spoke::WantsTransform() {
	return 0;
}
#endif

boolean Spoke::IsEqualTo(I3DExShader* aShader)	{	// Compare two Spoke shaders
	int32 equality = 1;	
	
	if (SpokeData.Number != ((Spoke*)aShader)->SpokeData.Number ||
		SpokeData.Ulocation != ((Spoke*)aShader)->SpokeData.Ulocation || 
		SpokeData.Vlocation != ((Spoke*)aShader)->SpokeData.Vlocation || 
		SpokeData.Height != ((Spoke*)aShader)->SpokeData.Height ||
		SpokeData.Circ_Falloff != ((Spoke*)aShader)->SpokeData.Circ_Falloff ||
		SpokeData.Radial_Start != ((Spoke*)aShader)->SpokeData.Radial_Start ||
		SpokeData.Radial_End != ((Spoke*)aShader)->SpokeData.Radial_End || 
		SpokeData.Offset != ((Spoke*)aShader)->SpokeData.Offset ||
		SpokeData.Start_Radial_Falloff != ((Spoke*)aShader)->SpokeData.Start_Radial_Falloff || 
		SpokeData.End_Radial_Falloff != ((Spoke*)aShader)->SpokeData.End_Radial_Falloff|| 
		SpokeData.ConstantWidth != ((Spoke*)aShader)->SpokeData.ConstantWidth || 
		SpokeData.FillCenter != ((Spoke*)aShader)->SpokeData.FillCenter || 
		SpokeData.Width != ((Spoke*)aShader)->SpokeData.Width 
		)
			equality = 0;

	return equality;
		  
}

MCCOMErr Spoke::GetShadingFlags(ShadingFlags& theFlags){
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Spoke::GetImplementedOutput(){
	return  kUsesGetValue;	
}

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Spoke::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Spoke::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real MCCOMAPI Spoke::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#endif

{
	fullArea = false;
	result = 0;

	shift_x = shadingIn.fUV[0] - SpokeData.Ulocation;
	shift_y = shadingIn.fUV[1] - SpokeData.Vlocation;

	if ((shift_x == 0) && (shift_y == 0))
		result = 0;
	else {
		// radius of the point we are shading
		r =  sqrt(pow(shift_x, 2) + pow(shift_y, 2));

		// In constant width we keep the spoke wide. . .
		if (SpokeData.ConstantWidth) {
			range_check = (SpokeData.Width/20)*DEG_RAD/r;
			if (range_check > 1) {
				if (SpokeData.FillCenter)
					result = 1;
				else
					// since we are done, return
				#if (VERSIONNUMBER == 0x010000)
				return MC_S_OK;
				#elif (VERSIONNUMBER == 0x020000)
				return MC_S_OK;
				#elif (VERSIONNUMBER >= 0x030000)
				return 1.0f;
				#endif
			}
			width = RAD_DEG*asin(range_check)/2.0;
		}
		else 
			width = ( SpokeData.Width  -  (r * SpokeData.Taper))/2.0;	// divide in half to center

			bot = seg_root - width;
			top = seg_root + width;							// make the bracket

		if ((r>= SpokeData.Radial_Start) && ( r<= SpokeData.Radial_End * SQRT_2)) {		// in the radius zone. . .
			// now the angle of the point we are shading 
			angle = RAD_DEG * (atan2(shift_x, shift_y)+ PI)+ SpokeData.Offset;

			angle = fmod(angle,segment_size);	// shift the angle down to the 0 segment	
			in_the_angle = 0;
			
			if ((angle > bot) && (angle <= seg_root)) {
				percent_value = (seg_root - angle)/width; 
				in_the_angle = 1;
			}
			if ( (angle > seg_root) && (angle <= top)){
				percent_value = (angle - seg_root)/width; 
				in_the_angle = 1;
			}

			if (in_the_angle) {
				if (percent_value < falloff_point){
					result = SpokeData.Height;
				}
				else {
					result =(1- (percent_value-falloff_point)/(1-falloff_point)) * SpokeData.Height;
				}

				if (r<SpokeData.Start_Radial_Falloff){
					result = result * ((r - SpokeData.Radial_Start)/
						(SpokeData.Start_Radial_Falloff - SpokeData.Radial_Start));
				} else if (r>SpokeData.End_Radial_Falloff) {
					result = result * ((SpokeData.Radial_End - r)/
						(SpokeData.Radial_End - SpokeData.End_Radial_Falloff));
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