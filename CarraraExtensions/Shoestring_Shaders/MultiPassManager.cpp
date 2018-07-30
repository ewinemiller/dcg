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
#include "MultiPassManager.h"
#include "copyright.h"
#include "Shoestringshaders.h"

MultiPassManager::MultiPassManager()	// Initialize
{
	for (int32 i = 0 ; i<=10 ; i++) Data.fShader[i] = NULL;
}

MCCOMErr MultiPassManager::GetShadingFlags(ShadingFlags& theFlags)
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

void* MultiPassManager::GetExtensionDataBuffer()
{
	return ((void*) &Data);
}

boolean MultiPassManager::IsEqualTo(I3DExShader* aShader)
{
	boolean flag;

	flag = true;

	for (int32 i = 0 ; i<=10 ; i++) {
		if (Data.fShader[i] != ((MultiPassManager*)aShader)->Data.fShader[i])
				flag = false;
	}
	return flag; 
}

MCCOMErr	MultiPassManager::DoShade(ShadingOut&  result, ShadingIn &  shadingIn)
	{
	int32 i;
	ShadingOut accum_shadingout;
	ShadingOut fetch;
	boolean fullArea;

	fullArea = false;
	luma = 0;
	mix = 0;
	
	if (!fShaders[0])  return MC_S_OK;  // if the mix shader is null, just return black.			
	else {
		if (Imp_Out[0] & kUsesGetColor)	{	// get color implemented
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

		i = (luma + 0.06) * 10;		// map function from MPC shader

//		accum_shadingout.Clear();
		if (Imp_Out[i] & kUsesDoShade)	{	// do shade
			fShaders[i]->DoShade(result, shadingIn);
			accum_shadingout = result;
			shadingIn.fCurrentCompletionMask = kAllChannels;
		}
		else if (Imp_Out[i] & kUsesGetColor){
			fShaders[i]->GetColor(Color, fullArea, shadingIn);
			accum_shadingout.fColor = Color;
			shadingIn.fCurrentCompletionMask = shadingIn.fCurrentCompletionMask | kColorChannel;
		}
		else if (Imp_Out[i] & kUsesGetValue){  //  or get value implemented
			fShaders[i]->GetValue(luma, fullArea, shadingIn);
			Color.R = luma;
			Color.G = luma;
			Color.B = luma;
			accum_shadingout.fColor = Color;
			shadingIn.fCurrentCompletionMask = shadingIn.fCurrentCompletionMask | kColorChannel;
		}
		result = accum_shadingout;
	}


#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 1.0f;
#endif
}

MCCOMErr MultiPassManager::ExtensionDataChanged()
{
	int32 i;

	MCCOMErr result1 = MC_S_OK;
	MCCOMErr result2 = MC_S_OK;

	for (i = 0 ; i<=10 ; i++)	// Copy the shader data to the local data structures
		if (Data.fShader[i])
		{
			result1 = Data.fShader[i]->QueryInterface(IID_I3DShShader, (void**)&fShaders[i]);
			if (result1 != MC_S_OK)
				result2 = MC_S_FALSE;
		}

	for (i = 0 ; i<=10 ; i++)			// check imp outs here as it only changes when the shader does
		if (fShaders[i] != NULL)
			Imp_Out[i] = fShaders[i]->GetImplementedOutput(); 
		else
			Imp_Out[i] = 0;

	return result2;
}

EShaderOutput MultiPassManager::GetImplementedOutput()  {
	uint32 implemented = kUsesDoShade;
	return (EShaderOutput)implemented;
}

#if (VERSIONNUMBER >= 0x040000)
boolean MultiPassManager::WantsTransform()  {
	return 0;
}
#endif
// end of MultiPassManager