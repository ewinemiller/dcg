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
#define  _USE_MATH_DEFINES
//#include "math.h"
#include "Super_mixer_top.h"
#include "copyright.h"
#include "Shoestringshaders.h"

Super_mixer_top::Super_mixer_top()	// Initialize
{
	for (int32 i = 0 ; i<=9 ; i++)
	{
		Data.vec2Influence[i][0] = i * 10;
		Data.vec2Influence[i][1] = (i * 10) + 10 ;
		Data.fShader[i] = NULL;
	}
	Data.fShader[10] = NULL;
	Data.fShader[11] = NULL;

	Data.BlendMode = kSmooth_mode;

	Data.Smooth = 0;
	Data.Fill = 0;

	Data.PercentBlend = 10;

	Data.Intensity = 10;

	Data.Initialize = kManual;
	last_luma = 0;
	last_mix = 0;
	Data.Luma_value = int32(100*(Data.Color.R*0.3 + Data.Color.G*0.59+ Data.Color.B*0.11));
	Data.Show_Mix = 0;

}

MCCOMErr Super_mixer_top::GetShadingFlags(ShadingFlags& theFlags)
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

void* Super_mixer_top::GetExtensionDataBuffer()
{
	return ((void*) &Data);
}

boolean Super_mixer_top::IsEqualTo(I3DExShader* aShader)
{
	boolean flag;

	flag = true;

	for (int32 i = 0 ; i<=9 ; i++)
	{
		if ((Data.vec2Influence[i][0] != ((Super_mixer_top*)aShader)->Data.vec2Influence[i][0]) |
			(Data.vec2Influence[i][1] != ((Super_mixer_top*)aShader)->Data.vec2Influence[i][1]) |
			(Data.fShader[i] != ((Super_mixer_top*)aShader)->Data.fShader[i]))
				flag = false;
	}
	if ((Data.fShader[10] != (((Super_mixer_top*)aShader)->Data.fShader[10])) |
		(Data.fShader[11] != (((Super_mixer_top*)aShader)->Data.fShader[11])) |
		(Data.BlendMode != (((Super_mixer_top*)aShader)->Data.BlendMode)) |
		(Data.Smooth != (((Super_mixer_top*)aShader)->Data.Smooth)) |
		(Data.PercentBlend != (((Super_mixer_top*)aShader)->Data.PercentBlend)) |
		(Data.Intensity != (((Super_mixer_top*)aShader)->Data.Intensity)) |
		(Data.Initialize != (((Super_mixer_top*)aShader)->Data.Initialize)) |
		(Data.Fill != (((Super_mixer_top*)aShader)->Data.Fill)) |
		(Data.Color != (((Super_mixer_top*)aShader)->Data.Color)) |		
		(Data.Show_Mix != (((Super_mixer_top*)aShader)->Data.Show_Mix)) |		
		(Data.Luma_value != (((Super_mixer_top*)aShader)->Data.Luma_value))		
		)
			flag = false;

	return flag; 
}


MCCOMErr	Super_mixer_top::DoShade(ShadingOut&  result, ShadingIn &  shadingIn)
	{
	int32 i;
	ShadingOut accum_shadingout;
	ShadingOut fetch;
	boolean fullArea;

	fullArea = false;
	luma = 0;
	mix = 0;

	if (Data.Show_Mix) {
		fShaders[0]->GetColor(Color, fullArea, shadingIn);
		result.fColor = Color;
		shadingIn.fCurrentCompletionMask = kColorChannel;
	}
	else {
		
		if (!fShaders[0])  return MC_S_OK;  // if the mix shader is null, just return black.			
		else {
			if (Imp_Out[0] & (kUsesGetColor | kUsesDoShade))	{	// get color implemented
				fShaders[0]->GetColor(Color, fullArea, shadingIn);
				luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
			}
			else if (Imp_Out[0] & kUsesGetValue)  //  or get value implemented
				fShaders[0]->GetValue(luma, fullArea, shadingIn);
			else if (Imp_Out[0] & kUsesDoShade)	{	// do shade
				fShaders[0]->DoShade(fetch, shadingIn);
				luma = (fetch.fColor.R*0.3 + fetch.fColor.G*0.59+ fetch.fColor.B*0.11);
			}
			else
				return MC_S_OK;		// no useful data back from mix shader, return black

			luma = luma > 1 ? last_luma : luma * 100;	

			// Add in blend shader, if requested
			if (Data.BlendMode == kShader_mode)	{
					if (!fShaders[1])  {
						mix = 0.5;
					}
					else {
						if (Imp_Out[1] & (kUsesGetColor | kUsesDoShade ))	{	// get color implemented
							fShaders[1]->GetColor(Color, fullArea, shadingIn);
							mix = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
						}
						else if (Imp_Out[1] & kUsesGetValue)  //  or get value implemented
							fShaders[1]->GetValue(mix, fullArea, shadingIn);
						else 
							mix = 0.5;		// no useful data back from mix shader,no influence
					}

					luma = (luma + Data.Intensity*(mix - 0.5)) > 0 ?  // pin at 0
						(luma + Data.Intensity*(mix - 0.5)) : 0;
					luma = luma > 100 ? 100: luma; // and at 100
				}

			last_luma = luma;	// in case it returned over 1.0, a bad value, preserve the last good value
			quant = 0;
			if (HaveAShader) {				// if there is at least one shader in place .. . 
				accum_shadingout.Clear();
				for (i = 2 ; i<=11 ; i++) {					
					real32 top = Data.vec2Influence[i-2][1];			// top value for influence range
					real32 bottom = Data.vec2Influence[i-2][0];
					if ((fShaders[i] != NULL) & 
						((bottom <= luma) & (top >= luma) |
						((quant == 0) & Data.Fill & (i == 11)))) {				// this shader has influence	
						weight = 1;
						if (Data.Smooth) {
							real32 center = (bottom + top)/2;
							real32 fall = (top - bottom) * Data.PercentBlend/100;
							
							if ((luma > center) & (top != 100)){
								per_along_curve = ((top - luma) / fall);
								per_along_curve = per_along_curve > 1 ? 0 :  1 - per_along_curve;
								weight = Data.Linear ? 1 - per_along_curve:
									0.5 * sin(per_along_curve * PI + M_PI_2) + 0.5;	// smooth blend with a sin curve
							}
							else if ((luma < center) & (bottom != 0)){
								per_along_curve = ( luma - bottom) / fall;
								per_along_curve = per_along_curve > 1 ? 1 : per_along_curve;
								weight = Data.Linear ? per_along_curve:
									0.5 * sin(per_along_curve * PI - M_PI_2) + 0.5;
							}
						}
						++quant;					// count up how many shaders contribute
						if (Imp_Out[i] & kUsesDoShade)	{	// do shade
							fetch.SetDefaultValues();
							fShaders[i]->DoShade(fetch, shadingIn);
							//float temp = fetch.fSubsurfaceScattering.fRefractionIndex;
							fetch *= weight;
							fetch.fGlow *= weight;
							accum_shadingout += fetch;	
							//accum_shadingout.fSubsurfaceScattering.fRefractionIndex = temp;
							shadingIn.fCurrentCompletionMask = kAllChannels;
						}
						else if (Imp_Out[i] & (kUsesGetColor | kUsesDoShade)){
							fShaders[i]->GetColor(Color, fullArea, shadingIn);
							Color *= weight;
							accum_shadingout.fColor += Color;
							shadingIn.fCurrentCompletionMask = shadingIn.fCurrentCompletionMask | kColorChannel;
						}
						else if (Imp_Out[i] & kUsesGetValue){  //  or get value implemented
							fShaders[i]->GetValue(luma, fullArea, shadingIn);
							Color.R = luma;
							Color.G = luma;
							Color.B = luma;
							Color *= weight;
							accum_shadingout.fColor += Color;
							shadingIn.fCurrentCompletionMask = shadingIn.fCurrentCompletionMask | kColorChannel;
						}
					}
				}
				result = accum_shadingout;
				if ((quant > 1) & !Data.Smooth)	{ // take the average to keep from blowing out on non-smooth case
					accum_shadingout *= (1.0/quant);
					//accum_shadingout.fGlow *= (1.0/quant);
					result = accum_shadingout;
				}
			}
			else
				return MC_S_OK;
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

MCCOMErr Super_mixer_top::ExtensionDataChanged()
{
	int32 i,j;

	MCCOMErr result1 = MC_S_OK;
	MCCOMErr result2 = MC_S_OK;

	for (i = 0 ; i<=11 ; i++)	// Copy the shader data to the local data structures
		if (Data.fShader[i])
		{
			result1 = Data.fShader[i]->QueryInterface(IID_I3DShShader, (void**)&fShaders[i]);
			if (result1 != MC_S_OK)
				result2 = MC_S_FALSE;
		}

	HaveAShader = false;
	for (i = 0 ; i<=11 ; i++)			// check imp outs here as it only changes when the shader does
		if (fShaders[i] != NULL){
			Imp_Out[i] = fShaders[i]->GetImplementedOutput(); 
			if (i>1) HaveAShader = true;		// if we have at least one mixed shader. . 
		}
		else
			Imp_Out[i] = 0;

	last_luma = 0;
	last_mix = 0;

	if (Data.Initialize == kInit_Auto)  // set up auto ranges
	{
		quant = 0;
		for (i = 2 ; i<=11 ; i++)		// count how many live shaders there are
			if (fShaders[i] != NULL)
				quant++;
		if (quant != 0)				
			{
				j = 0;
				for (i = 2 ; i<=11 ; i++)
					if (fShaders[i] != NULL)		// each live shader gets an even percentage
					{
						Data.vec2Influence[i-2][0] = j * 100/quant;
						j++;
						Data.vec2Influence[i-2][1] = j * 100/quant ;
					}
			}
	}
	Data.Luma_value = int32(100*(Data.Color.R*0.3 + Data.Color.G*0.59+ Data.Color.B*0.11));
	return result2;
}

EShaderOutput Super_mixer_top::GetImplementedOutput()
{
	uint32 implemented = kUsesDoShade;
	return (EShaderOutput)implemented;
}

#if (VERSIONNUMBER >= 0x040000)
boolean Super_mixer_top::WantsTransform()
{
	return 0;
}
#endif
// end of Super_mixer_top