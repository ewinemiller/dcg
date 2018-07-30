/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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
#include "NormalToTerrain.h" 
#include "TerrainToolsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_NormalToTerrain(R_CLSID_NormalToTerrain);
#else
const MCGUID CLSID_NormalToTerrain={R_CLSID_NormalToTerrain};
#endif

NormalToTerrain::NormalToTerrain() 
{
}
 
void* NormalToTerrain::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr NormalToTerrain::ExtensionDataChanged()
{
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

boolean	NormalToTerrain::IsEqualTo(I3DExShader* aShader)
{

  return (
	  (fData.param==((NormalToTerrain*)aShader)->fData.param)
	  ); 
}  

MCCOMErr NormalToTerrain::GetShadingFlags(ShadingFlags& theFlags)
{

	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput NormalToTerrain::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesGetValue);
}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 NormalToTerrain::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	MCCOMErr retval = MC_S_OK;
	if (shader != NULL) 
	{
		if ((ChildOutput & kUsesGetValue) == kUsesGetValue) 
		{
			retval = shader->GetValue(result, fullArea, shadingIn);
		}
		else if ((ChildOutput & kUsesGetColor) == kUsesGetColor) 
		{
			#if (VERSIONNUMBER >= 0x040000)
				TMCColorRGBA getcolorresult;
			#else
				TMCColorRGB getcolorresult;
			#endif
			retval = GetColor(getcolorresult, fullArea, shadingIn);

			result = 0.299 * getcolorresult.R + 0.587 * getcolorresult.G + 0.114 * getcolorresult.B;		
		}
	}
	return retval;
}
