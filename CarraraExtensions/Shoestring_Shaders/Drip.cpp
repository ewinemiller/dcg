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
#include "Drip.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Drip::ExtensionDataChanged()
{

	MCCOMErr result = MC_S_OK;

	return result;
}

Drip::Drip()		// Initialize the public data
{
	DripData.Ulocation = 0.5f;
	DripData.Vlocation = 0.5f;
	DripData.Height = 0.5f;
	DripData.Width = 0.1f;
	DripData.Offset = 0;
	DripData.Completion = 0.2f;
	DripData.AutoWidth = 0;
	DripData.Inter_ripple_freq = 1;
	DripData.Use_shaping = 1;
	DripData.Rebound = 0;
	DripData.Rebound_Lag = 0.5f;
	DripData.Rebound_Size = 0.7f;
	DripData.Show_Center = 0;
}
	
void* Drip::GetExtensionDataBuffer()
{
	return ((void*) &(DripData));
}

#if (VERSIONNUMBER >= 0x040000)
boolean Drip::WantsTransform()
{
	return 0;
}
#endif

boolean Drip::IsEqualTo(I3DExShader* aShader)		// Compare two Drip shaders
{
	int32 equality = 1;	
	
	if (DripData.AutoWidth != ((Drip*)aShader)->DripData.AutoWidth ||
		DripData.Completion != ((Drip*)aShader)->DripData.Completion ||
		DripData.Height != ((Drip*)aShader)->DripData.Height ||
		DripData.Inter_ripple_freq != ((Drip*)aShader)->DripData.Inter_ripple_freq ||
		DripData.Offset != ((Drip*)aShader)->DripData.Offset ||
		DripData.Rebound != ((Drip*)aShader)->DripData.Rebound || 
		DripData.Ulocation != ((Drip*)aShader)->DripData.Ulocation || 
		DripData.Use_shaping != ((Drip*)aShader)->DripData.Use_shaping || 
		DripData.Vlocation != ((Drip*)aShader)->DripData.Vlocation || 
		DripData.Rebound_Lag != ((Drip*)aShader)->DripData.Rebound_Lag || 
		DripData.Rebound_Size != ((Drip*)aShader)->DripData.Rebound_Size || 
		DripData.Show_Center != ((Drip*)aShader)->DripData.Show_Center || 
		DripData.Width != ((Drip*)aShader)->DripData.Width 
		)
			equality = 0;

	return equality;
		  
}

MCCOMErr Drip::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsIsoUV = true;		// IsoUV are normals aligned with the UV axes
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Drip::GetImplementedOutput()
{
	return (EShaderOutput) (kUsesGetValue);	
}


#if (VERSIONNUMBER == 0x010000)
MCCOMErr Drip::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Drip::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x030000)
real MCCOMAPI Drip::GetValue (real& result,boolean& fullArea,ShadingIn& shadingIn)
#endif

{
	fullArea = false;
	Drip::Ring_return Calc_result;
	real hold;

	if (DripData.Show_Center){						// draw a cross hairs
		real uval = fabs(shadingIn.fUV[0] - DripData.Ulocation);
		real vval = fabs(shadingIn.fUV[1] - DripData.Vlocation);
		if (((uval < 0.2) && (vval < 0.02)) || ((vval < 0.2) && (uval < 0.02)))
			result = 1;
		else
			result = 0;
	}
	else {
		result = 1;
		// radius of the point we are shading
		r = sqrt(pow(shadingIn.fUV[0] - DripData.Ulocation, 2) + pow(shadingIn.fUV[1] - DripData.Vlocation, 2));	
		
		// check the main ring
		Calc_result = Calculate_ring(DripData.Completion, DripData.Width, DripData.Height);
		result = Calc_result.value;
		if (result == -100)
			result = DripData.Offset;
		else
			float temp = result;

		// now calculate the rebound
		if (DripData.Rebound) {
			hold = result;
			Calc_result = Calculate_ring(DripData.Completion * DripData.Rebound_Lag, 
				DripData.Width * fabs(DripData.Rebound_Size), DripData.Height * DripData.Rebound_Size);
			result = Calc_result.value;
			if (result == -100)
				result = hold;
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

Drip::Ring_return Drip::Calculate_ring(real dd_completion, real dd_width, real dd_height) {

	real location;
	real leading_edge;
	real width;
	real temp;
	Ring_return result;

	// dd_completion is the radius of the inner edge of the ring 

	if (dd_width) {
		temp =1.0 + dd_width;
	}
	else  temp = real(1.1);							// no width specified, set to .1

	// find the outer edge and width
	if (DripData.AutoWidth) {
		leading_edge = dd_completion*temp;	
		width = leading_edge - dd_completion;
	}
	else {
		leading_edge = dd_completion + dd_width;	
		width = dd_width;
	}

	if ((r >= dd_completion ) && (r <=leading_edge ))		// between the edges
	{
		// figure out exactly where within the band we are (placed in a range from intercept to intercept)
		location = TWO_PI * ((r - dd_completion)/width) - PI;
		
		if (DripData.Use_shaping){
			result.value = DripData.Offset + dd_height * cos(fabs(location)*DripData.Inter_ripple_freq) * (NEG_ONE_PI * fabs(location) + 1);

			result.slope = -sin(fabs(location) * DripData.Inter_ripple_freq) * (NEG_ONE_PI * fabs(location) + 1);
		}
		else {
			result.value = dd_height + DripData.Offset;
			result.slope = 0;
		}
	}
	else result.value = -100;					// return -100 to indicate that the ring is not hit.

	return (result);
}

TVector3 Drip::VectorRotate(TVector3 shaft, TVector3 vector, real32 asin_angle)
{
	real32 cos_rot = cosf(asinf(asin_angle));	
	real32 sin_rot = asin_angle;
	real32 t = 1.0 - cos_rot;
	TVector3 return_vec;


	return_vec[0] = 0;
	return_vec[1] = 0;
	return_vec[2] = 0;
			
	return_vec[0] += (cos_rot + t * shaft[0] * shaft[0]) * vector[0];
	return_vec[0] += (t * shaft[0] * shaft[1] - shaft[2] * sin_rot) * vector[1];
	return_vec[0] += (t * shaft[0] * shaft[2] + shaft[1] * sin_rot) * vector[2];

	return_vec[1] += (t * shaft[0] * shaft[1] + shaft[2] * sin_rot) * vector[0];
	return_vec[1] += (cos_rot + t * shaft[1] * shaft[1]) * vector[1];
	return_vec[1] += (t * shaft[1] * shaft[2] - shaft[0] * sin_rot) * vector[2];

	return_vec[2] += (t * shaft[0] * shaft[2] - shaft[1] * sin_rot) * vector[0];
	return_vec[2] += (t * shaft[1] * shaft[2] + shaft[0] * sin_rot) * vector[1];
	return_vec[2] += (cos_rot + t * shaft[2] * shaft[2]) * vector[2];

	return return_vec;
}
