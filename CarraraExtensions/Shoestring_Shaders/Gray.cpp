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
#include "Gray.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MCCOMErr Gray::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;

	if (GrayData.fInputShader)
	result = GrayData.fInputShader->QueryInterface(IID_I3DShShader, (void**)&fInputShader);
	
	Imp_Out = 0;

	if (fInputShader != NULL)
		Imp_Out = fInputShader->GetImplementedOutput();  

	Colorset = GrayData.Do_Color;
	Glowset = GrayData.Do_Glow;
	Specularset = GrayData.Do_Spec;
	Transset = GrayData.Do_Trans;

	desat_red = GrayData.Desat_Red/100.0;
	desat_green = GrayData.Desat_Grn/100.0;
	desat_blue = GrayData.Desat_Blu/100.0;

	return result;
}
Gray::Gray()		// Initialize the public data
{
	GrayData.Gray_level = 0;
	GrayData.Desat_Red = 0;
	GrayData.Desat_Grn = 0;
	GrayData.Desat_Blu = 0;
	
	GrayData.fInputShader = NULL;
	GrayData.Mode = kGray_mode;
}
	
void* Gray::GetExtensionDataBuffer()
{
	return ((void*) &(GrayData));
}

boolean Gray::IsEqualTo(I3DExShader* aShader)		// Compare two Gray shaders
{
	int32 equality = 1;	
	if (GrayData.Gray_level != ((Gray*)aShader)->GrayData.Gray_level ||
		GrayData.Desat_Red != ((Gray*)aShader)->GrayData.Desat_Red ||
		GrayData.Desat_Grn != ((Gray*)aShader)->GrayData.Desat_Grn ||
		GrayData.Desat_Blu != ((Gray*)aShader)->GrayData.Desat_Blu ||
		GrayData.Mode != ((Gray*)aShader)->GrayData.Mode ||
		GrayData.fInputShader != ((Gray*)aShader)->GrayData.fInputShader 
		)
			equality = 0;

	return equality;
		  
}

MCCOMErr Gray::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsIsoUV = true;
	theFlags.fNeedsNormal = true;
	theFlags.fNeedsNormalDerivative = true;
	theFlags.fNeedsNormalLoc = true;
	theFlags.fNeedsNormalLocDerivative = true;
	theFlags.fNeedsPixelRatio = true;
	theFlags.fNeedsPoint = true;
	theFlags.fNeedsPointLoc = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Gray::GetImplementedOutput()
{
	return  (EShaderOutput) (kUsesGetColor + kUsesDoShade);	
}
  
#if (VERSIONNUMBER >= 0x040000)
boolean Gray::WantsTransform()
{
	return 0;
}
#endif

MCCOMErr	Gray::DoShade(ShadingOut&  result, ShadingIn &  shadingIn)

{
	boolean fullArea;
	fullArea = false;

	ShadingOut hold = result;
	TMCColorRGBA RGBA_Color;

	if (!fInputShader) { // if the shader is null, just return.
		return 1.0f;}

	if (GrayData.Mode == kGray_mode) {
		if (Imp_Out & kUsesDoShade)	{	// do shade
			fInputShader->DoShade(result, shadingIn);
			shadingIn.fCurrentCompletionMask = kAllChannels;
			if (Colorset) {
				Color = result.fColor;
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11); 
				Color.red = luma;
				Color.blue = luma;
				Color.green = luma;
				result.fColor = Color; }
			if (Glowset) {
				Color = result.fGlow;
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11); 
				Color.red = luma;
				Color.blue = luma;
				Color.green = luma;
				result.fGlow = Color; }
			if (Specularset) {
				Color = result.fSpecularColor;
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11); 
				Color.red = luma;
				Color.blue = luma;
				Color.green = luma;
				result.fSpecularColor = Color; }
			if (Transset) {
				Color = result.fTranslucency;
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11); 
				Color.red = luma;
				Color.blue = luma;
				Color.green = luma;
				result.fTranslucency = Color;  }
		}
		else 
			return 1.0f;		// no useful data back from shader, return
	}
	else {
		if (Imp_Out & kUsesDoShade)	{	// do shade
			fInputShader->DoShade(result, shadingIn);
			shadingIn.fCurrentCompletionMask = kAllChannels;
			if (Colorset) {
				Color = SatControl(result.fColor);
				result.fColor = Color; }
			if (Glowset) {
				RGBA_Color = result.fGlow;
				Color = SatControl(RGBA_Color);
				result.fGlow = Color; }
			if (Specularset) {
				RGBA_Color = result.fSpecularColor;
				Color = SatControl(RGBA_Color);
				result.fSpecularColor = Color; }
			if (Transset) {
				RGBA_Color = result.fTranslucency;
				Color = SatControl(RGBA_Color);
				result.fTranslucency = Color;  }
		else 
			return 1.0f;		// no useful data back from shader
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

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Gray::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Gray::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Gray::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Gray::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	fullArea = false;

	// initialize colors
	result.R = 0;
	result.G = 0;
	result.B = 0;

	hold = result; 

	if (GrayData.Mode == kGray_mode){
		if (!fInputShader)  // if the shader is null, just return black.
				return MC_S_OK;
		else {
			if (Imp_Out & kUsesGetColor)	{	// get color implemented
				fInputShader->GetColor(Color, fullArea, shadingIn);
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
				#if (VERSIONNUMBER >= 0x040000)
				result.A = Color.A;
				#endif
			}
			else if (Imp_Out & kUsesGetValue)  //  or get value implemented
				fInputShader->GetValue(luma, fullArea, shadingIn);
			else 
				luma = 0;		// no useful data back from shader, return black
		}

		result.R = luma;
		result.G = luma;
		result.B = luma;
	}
	else
	{
		if (!fInputShader)  // if the shader is null, just return black.
				return 1.0f;
		else	{
			if (Imp_Out & kUsesGetColor)	{	// get color implemented			
				fInputShader->GetColor(Color, fullArea, shadingIn);
				result = SatControl(Color);
				#if (VERSIONNUMBER >= 0x040000)
				result.A = Color.A;
				#endif
			}
			else 
				return 1.0f;		// no useful data back from shader, return black
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


// Hue to RGB subroutine

real32 Gray::HueToRGB(real32 n1, real32 n2, real32 hue)
{
    if ( hue < 0 ) hue += 360;
    if ( hue > 360 ) hue -= 360;

 
    if ( hue < 60 )
        return ( n1 + (n2-n1)*hue/60 );
    if ( hue < 180 )
        return n2;
    if ( hue < 240)
        return ( n1 + (n2-n1)*(240 -hue)/60 );
    else
        return n1;
}

// Main action subroutine

TMCColorRGBA Gray::SatControl(TMCColorRGBA Color)
{
	real32 Magic1, Magic2;
	real32 max;
	real32 min;
	real32 lightness;
	real32 saturation;
	real32 hue;
	real32 delta;

	// convert to hls space to allow hue preservation
	if		((Color.R >= Color.G) & (Color.R >= Color.B)) max = Color.R;
	else if	((Color.G >= Color.R) & (Color.G >= Color.B)) max = Color.G;
	else max = Color.B;
	
	if		((Color.R <= Color.G) & (Color.R <= Color.B)) min = Color.R;
	else if	((Color.G <= Color.R) & (Color.G <= Color.B)) min = Color.G;
	else min = Color.B;

	if (Color.green != Color.green)
		return Color;

	lightness = (max+min)/2;
	// calculate saturation
	if (max == min)	{			// colorless case?		   
		saturation = 0;
		hue = 1000; }
	else {
		if (lightness <= 0.5)	saturation = (max-min)/(max+min);
		else					saturation = (max-min)/(2-max-min);
		// calculate hue
		delta = (max-min) ;
		if (Color.R == max)
			hue = (Color.G - Color.B)/delta * 60;
		else if (Color.G == max)
			hue = (2 + (Color.B - Color.R)/delta)* 60;
		else if (Color.B == max)
			hue = (4 + (Color.R - Color.G)/delta) * 60;
		if (hue < 0.0)
			hue = hue + 360; }

	// reduce saturation (the real action)
	saturation += GrayData.Gray_level/100;

	if  ((hue >= 0) & (hue <= 60)) 
			{ 
			saturation += desat_red;                // full on red 
			saturation += (hue/60 * desat_green);  // green contribution 
			} 
	else if ((hue > 60) & (hue <= 120)) 
			{ 
			saturation += desat_green; 
			saturation += (60-(hue-60))/60 * desat_red; 
			} 
	else if ((hue >120) & (hue <= 180)) 
			{ 
			saturation += desat_green; 
			saturation += (hue-120)/60 * desat_blue; 
			} 
	else if ((hue >180) & (hue <= 240)) 
			{ 
			saturation += desat_blue;       
			saturation += (60-(hue-180))/60 * desat_green;  
			} 
	else if ((hue >240) & (hue <= 300)) 
			{ 
			saturation += desat_blue;       
			saturation += (hue-240)/60 * desat_red; 
			} 
	else if ((hue >300) & (hue <= 360)) 
			{ 
			saturation += desat_red;        
			saturation += (60-(hue-300))/60 * desat_blue;  
			} 
			
	if (saturation < 0) saturation = 0;		// pin it to prevent false coloring

// finish up routine

	if ( saturation==0) {
		Color.R = lightness;
		Color.G = lightness;
		Color.B = lightness;}
	else {        
		if (lightness <= 0.5)
			Magic2 = (lightness * (1 + saturation));
		else
			Magic2 = lightness + saturation - (lightness * saturation);

		Magic1 = 2 * lightness - Magic2;

		Color.R = HueToRGB(Magic1, Magic2, hue + 120);
		Color.G = HueToRGB(Magic1, Magic2, hue);
		Color.B = HueToRGB(Magic1, Magic2, hue - 120);	}	
	return Color;
}

