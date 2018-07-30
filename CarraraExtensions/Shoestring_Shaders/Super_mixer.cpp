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
#include "Super_mixer.h"
#include "copyright.h"
#include "Shoestringshaders.h"

Super_mixer::Super_mixer()	// Initialize
{
	for (int32 i = 0 ; i<=9 ; i++)
	{
		Data.vec2Influence[i][0] = i * 10;
		Data.vec2Influence[i][1] = (i * 10) + 10 ;
		Data.fShader[i] = NULL;
	}
	Data.fShader[10] = NULL;
	Data.fShader[11] = NULL;

	Data.MixMode = kMix_mode;
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
	
void* Super_mixer::GetExtensionDataBuffer()
{
	return ((void*) &Data);
}

boolean Super_mixer::IsEqualTo(I3DExShader* aShader)
{
	boolean flag;

	flag = true;

	for (int32 i = 0 ; i<=9 ; i++)
	{
		if ((Data.vec2Influence[i][0] != ((Super_mixer*)aShader)->Data.vec2Influence[i][0]) |
			(Data.vec2Influence[i][1] != ((Super_mixer*)aShader)->Data.vec2Influence[i][1]) |
			(Data.fShader[i] != ((Super_mixer*)aShader)->Data.fShader[i]))
				flag = false;
	}
	if ((Data.fShader[10] != (((Super_mixer*)aShader)->Data.fShader[10])) |
		(Data.fShader[11] != (((Super_mixer*)aShader)->Data.fShader[11])) |
		(Data.MixMode != (((Super_mixer*)aShader)->Data.MixMode)) |
		(Data.BlendMode != (((Super_mixer*)aShader)->Data.BlendMode)) |
		(Data.Smooth != (((Super_mixer*)aShader)->Data.Smooth)) |
		(Data.PercentBlend != (((Super_mixer*)aShader)->Data.PercentBlend)) |
		(Data.Intensity != (((Super_mixer*)aShader)->Data.Intensity)) |
		(Data.Initialize != (((Super_mixer*)aShader)->Data.Initialize)) |
		(Data.Fill != (((Super_mixer*)aShader)->Data.Fill)) |
		(Data.Color != (((Super_mixer*)aShader)->Data.Color)) |		
		(Data.Show_Mix != (((Super_mixer*)aShader)->Data.Show_Mix)) |		
		(Data.Luma_value != (((Super_mixer*)aShader)->Data.Luma_value))		
		)
			flag = false;

	return flag; 
}


MCCOMErr Super_mixer::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

MCCOMErr Super_mixer::ExtensionDataChanged()
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

	for (i = 0 ; i<=11 ; i++)			// check imp outs here as it only changes when the shader does
		if (fShaders[i] != NULL)
			Imp_Out[i] = fShaders[i]->GetImplementedOutput();  
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

EShaderOutput Super_mixer::GetImplementedOutput()
{
	return kUsesGetColor ;	// Tell the shell that the Super_mixer Shader is a Color Shader
}

#if (VERSIONNUMBER >= 0x040000)
boolean Super_mixer::WantsTransform()
{
	return 0;
}
#endif

#if (VERSIONNUMBER == 0x010000)
MCCOMErr Super_mixer::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Super_mixer::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Super_mixer::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Super_mixer::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif

{
	int32 i;

	result.R = 0;
	result.G = 0;
	result.B = 0;
	luma = 0;
	mix = 0;

	hold = result; 

	fullArea = false;

	if (Data.Show_Mix) {
		fShaders[0]->GetColor(Color, fullArea, shadingIn);
		result = Color;
	}
	else
	{
		switch (Data.MixMode)
		{
			case kMix_mode:        // mix with grayscale
			{
				if (!fShaders[0])  return MC_S_OK;  // if the mix shader is null, just return black.			
				else
				{
					if (Imp_Out[0] & kUsesGetColor)		// get color implemented
					{
						fShaders[0]->GetColor(Color, fullArea, shadingIn);
						luma = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
					}
					else if (Imp_Out[0] & kUsesGetValue)  //  or get value implemented
						fShaders[0]->GetValue(luma, fullArea, shadingIn);
					else 
						return MC_S_OK;		// no useful data back from mix shader, return black


					luma = luma > 1 ? last_luma : luma * 100;	

					// Add in blend shader, if requested
					if (Data.BlendMode == kShader_mode)
						{
							if (!fShaders[1])  // if the mix shader is null, just return black.
							{
								mix = 0.5;
							}
							else
							{
								if (Imp_Out[1] & kUsesGetColor)		// get color implemented
								{
									fShaders[1]->GetColor(Color, fullArea, shadingIn);
									mix = (Color.R*0.3 + Color.G*0.59+ Color.B*0.11);
								}
								else if (Imp_Out[1] & kUsesGetValue)  //  or get value implemented
									fShaders[1]->GetValue(mix, fullArea, shadingIn);
								else 
									mix = 0.5;		// no useful data back from mix shader,no influence
							}

							luma = (luma + Data.Intensity*(mix - 0.5)) > 0 ? (luma + Data.Intensity*(mix - 0.5)) : 0; // pin at 0
							luma = luma > 100 ? 100: luma; // and at 100

						}

					last_luma = luma;	// in case it returned over 1.0, a bad value, preserve the last good value
					quant = 0;
					for (i = 2 ; i<=11 ; i++) {					
						real32 top = Data.vec2Influence[i-2][1];			// top value for influence range
						real32 bottom = Data.vec2Influence[i-2][0];

						if ((fShaders[i] != NULL) & ((bottom <= luma) & (top >= luma) |
							((quant == 0) & Data.Fill & (i == 11)))) {// this shader has influence						
							weight = 1;
							if (Data.Smooth){
								real32 center = (bottom + top)/2;
								real32 fall = (top - bottom) * Data.PercentBlend/100;
								
								if ((luma > center) & (top != 100)){
									per_along_curve = ((top - luma) / fall);
									per_along_curve = per_along_curve > 1 ? 0 :  1 - per_along_curve;
									weight = Data.Linear ? 1 - per_along_curve:
										0.5 * sinf(per_along_curve * PI + M_PI_2) + 0.5;	// smooth blend with a sin curve
								}
								else if ((luma < center) & (bottom != 0)){
									per_along_curve = ( luma - bottom) / fall;
									per_along_curve = per_along_curve > 1 ? 1 : per_along_curve;
									weight = Data.Linear ? per_along_curve:
										0.5 *  sinf(per_along_curve * PI - M_PI_2) + 0.5;
								}
							}
					 
							++quant;					// count up how many shaders contribute
							if (Imp_Out[i] & kUsesGetColor)		// get color implemented
							{
								fShaders[i]->GetColor(Color, fullArea, shadingIn);
								result.R += Color.R * weight;
								result.G += Color.G * weight;
								result.B += Color.B * weight;
								#if (VERSIONNUMBER >= 0x040000)
								result.alpha += Color.alpha * weight;
								#endif
							}
							else if (Imp_Out[i] & kUsesGetValue)  //  or get value implemented
							{
								fShaders[i]->GetValue(luma, fullArea, shadingIn);
								luma *= weight;
								result.R += luma;
								result.G += luma;
								result.B += luma;
							}
						}
					}
					if ((quant > 1) & !Data.Smooth)	// take the average to keep from blowing out on non-smooth case
					{
						result.R = result.R / quant;	
						result.G = result.G / quant;
						result.B = result.B / quant;
					}
				}
			}
			break;
			case kAdd_mode:
			{
				for (i = 2 ; i<=11 ; i++)
					if (Imp_Out[i] != 0)
						if (Imp_Out[i] & kUsesGetColor)		// get color implemented
							{
								fShaders[i]->GetColor(Color, fullArea, shadingIn);
								result.R += Color.R;
								result.G += Color.G;
								result.B += Color.B;
								#if (VERSIONNUMBER >= 0x040000)
								result.alpha += Color.alpha * weight;
								#endif
							}
							else if (Imp_Out[i] & kUsesGetValue)  //  or get value implemented
							{
								fShaders[i]->GetValue(luma, fullArea, shadingIn);
								result.R += luma;
								result.G += luma;
								result.B += luma;
							}
			}
			break;
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
// end of Super_mixer