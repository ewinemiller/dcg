/*  Ground Control - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

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
#include "GCWater.h"
#include "DEMDLL.h"
#include <algorithm>

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GCWater(R_CLSID_GCWater);
#else
const MCGUID CLSID_GCWater={R_CLSID_GCWater};
#endif

GCWater::GCWater() 
{
	SetDefaults();
}

void GCWater::SetDefaults () 
{
	fData.lSamples = 4;
	fData.fThreshold = .20f;
	fData.fDistance = .05f;
	fData.fHeight = .05f;
}

void* GCWater::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr GCWater::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildFlags.fNeedsUV = true;
		ChildOutput = shader->GetImplementedOutput();
		}

	samples.SetElemCount(fData.lSamples);

	for (uint32 lSide = 0; lSide < fData.lSamples;lSide++)
	{
		//draw the points around the origin in XY plane
		real32 fAngle = static_cast<real32>(lSide) / static_cast<real32>(fData.lSamples) * TWO_PI;
		real32 fCos = cos(fAngle), fSin = sin(fAngle);
		samples[lSide].x = fCos * fData.fDistance;
		samples[lSide].y = fSin * fData.fDistance;
	}

	return MC_S_OK;
	}

boolean	GCWater::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.param==((GCWater*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr GCWater::GetShadingFlags(ShadingFlags& theFlags){
	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput GCWater::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 GCWater::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	TMCColorRGB getcolorresult;
	int32 retvalue;

	retvalue = GetColor(getcolorresult, fullArea, shadingIn);

	if (retvalue == MC_S_OK)
	{
		result = 0.299 * getcolorresult.R + 0.587 * getcolorresult.G + 0.114 * getcolorresult.B;
	}

	return retvalue;
}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 GCWater::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn) 
{
	if (shader != NULL) 
	{
		TVector2 orignalUV = shadingIn.fUV;
		
#if (VERSIONNUMBER >= 0x040000)
		TMCColorRGBA tempresult, avgresult(0,0,0,1);
#else
		TMCColorRGB tempresult, avgresult(0,0,0);
#endif
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(tempresult, fullArea, shadingIn);
			result = tempresult;
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			} 

		if (result.Intensity() <= fData.fHeight)
		{
			result.Set(0, 0, 0);
			return MC_S_OK;
		}
		
		uint32 lSidesSampled = 0;

		for (uint32 lSide = 0; lSide < fData.lSamples;lSide++)
		{
			shadingIn.fUV = orignalUV + samples[lSide];
			if ((shadingIn.fUV.x >= 0)&&(shadingIn.fUV.x <= 1)&&(shadingIn.fUV.y >= 0)&&(shadingIn.fUV.y <= 1))
			{
				if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
					shader->GetColor(tempresult, fullArea, shadingIn);
					}
				else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
					real childresult;
					shader->GetValue(childresult, fullArea, shadingIn);
					tempresult.R = childresult;
					tempresult.G = childresult;
					tempresult.B = childresult;
					} 
				avgresult+=tempresult;
				lSidesSampled++;
			}
		}

		avgresult /= static_cast<real32>(lSidesSampled);

		if (avgresult.Intensity() > result.Intensity())
		{
			real32 value = 1 + (result.Intensity() - avgresult.Intensity()) / (fData.fThreshold);

			if (value < 0)
			{
				result.Set(0,0,0);
			}
			if (value > 1)
			{
				result.Set(1,1,1);
			}
			else
			{
				result.Set(value,value,value);
			}

		}
		else
		{
			result.Set(1,1,1);
		}

		shadingIn.fUV = orignalUV;
	}

	return MC_S_OK;

}

int16 GCWater::GetResID(){
	return 304;
	}
