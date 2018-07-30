/*  Shader Ops - plug-in for Carrara
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
#include "UVTile.h" 
#include "ShaderOpsDLL.h"


inline real32 intcompare(const real32 &in) {
	return floorf(in * 100000 + .5);
	}

inline real32 filter(const real32 &in) {
	return intcompare(in) / (real32)100000;
	}
  
#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_UVTile(R_CLSID_UVTile);
#else
const MCGUID CLSID_UVTile={R_CLSID_UVTile};
#endif
UVTile::UVTile() {
	SetDefaults();
	}

void UVTile::SetDefaults() {
	fData.fUTile = 2;
	fData.fVTile = 2;
	}

void* UVTile::GetExtensionDataBuffer(){
	return ((void*) &(fData));
	}

MCCOMErr UVTile::ExtensionDataChanged(){
	if (!fData.param) {
		shader = NULL;
		}
	else {
		fData.param->QueryInterface(IID_I3DExShader, (void**)&shader);
		shader->GetShadingFlags(ChildFlags);
		ChildOutput = shader->GetImplementedOutput();
		}

	return MC_S_OK;
	}

boolean	UVTile::IsEqualTo(I3DExShader* aShader){

  return (
	  (fData.fUTile ==((UVTile*)aShader)->fData.fUTile)
	  &&
	  (fData.fVTile ==((UVTile*)aShader)->fData.fVTile)
	  &&
	  (fData.param==((UVTile*)aShader)->fData.param)
	  ); 
	}  

MCCOMErr UVTile::GetShadingFlags(ShadingFlags& theFlags){

	theFlags = ChildFlags;
	return MC_S_OK;
	}

EShaderOutput UVTile::GetImplementedOutput(){
	return static_cast<EShaderOutput>(kUsesGetValue + kUsesGetColor);
	}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
UVTile::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA getcolorresult;
#else
	TMCColorRGB getcolorresult;
#endif
	int32 retvalue;

	retvalue = GetColor(getcolorresult, fullArea, shadingIn);

	if (retvalue == MC_S_OK)
	{
		result = 0.299 * getcolorresult.R + 0.587 * getcolorresult.G + 0.114 * getcolorresult.B;
	}

	return retvalue;
}

#if (VERSIONNUMBER >= 0x040000)
real UVTile::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real UVTile::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr UVTile::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) {
		TVector2 fUV = shadingIn.fUV;
		TVector2 fUVx = shadingIn.fUVx;
		TVector2 fUVy = shadingIn.fUVy;
		shadingIn.fUV.x = fData.fUTile * shadingIn.fUV.x;
		shadingIn.fUV.y = fData.fVTile * shadingIn.fUV.y;
		if (shadingIn.fUV.x != (int32)shadingIn.fUV.x) {
			shadingIn.fUV.x -= (int32)shadingIn.fUV.x;
			}
		if (shadingIn.fUV.y != (int32)shadingIn.fUV.y) {
			shadingIn.fUV.y -= (int32)shadingIn.fUV.y;
			}
		shadingIn.fUVx.x *= fData.fUTile; 
		shadingIn.fUVx.y *= fData.fVTile;
		shadingIn.fUVy.x *= fData.fUTile;
		shadingIn.fUVy.y *= fData.fVTile;
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) {
			shader->GetColor(result, fullArea, shadingIn);
			}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) {
			real childresult;
			shader->GetValue(childresult, fullArea, shadingIn);
			result.R = childresult;
			result.G = childresult;
			result.B = childresult;
			}   
		shadingIn.fUV = fUV;
		shadingIn.fUVx = fUVx;
		shadingIn.fUVy = fUVy;
		}  
	return MC_S_OK;

	}

int16 UVTile::GetResID(){
	return 311;
	}
