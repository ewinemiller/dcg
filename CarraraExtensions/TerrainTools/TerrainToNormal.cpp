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
#include "TerrainToNormal.h" 
#include "TerrainToolsDLL.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_TerrainToNormal(R_CLSID_TerrainToNormal);
#else
const MCGUID CLSID_TerrainToNormal={R_CLSID_TerrainToNormal};
#endif

TerrainToNormal::TerrainToNormal() 
{
}
 
void* TerrainToNormal::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr TerrainToNormal::ExtensionDataChanged()
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

boolean	TerrainToNormal::IsEqualTo(I3DExShader* aShader)
{

  return (
	  (fData.param==((TerrainToNormal*)aShader)->fData.param)
	  ); 
}  

MCCOMErr TerrainToNormal::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags = ChildFlags;
	return MC_S_OK;
}

EShaderOutput TerrainToNormal::GetImplementedOutput()
{
	return ChildOutput;
}

#if (VERSIONNUMBER >= 0x030000)
real
#else
MCCOMErr
#endif
 TerrainToNormal::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (shader != NULL) 
	{
		if ((ChildOutput & kUsesGetValue) == kUsesGetValue)
		{
			return shader->GetValue(result, fullArea, shadingIn);
		}
		else if ((ChildOutput & kUsesGetColor) == kUsesGetColor) 
		{
			#if (VERSIONNUMBER >= 0x040000)
				TMCColorRGBA childresult;
				#if (VERSIONNUMBER >= 0x030000)
				real
				#else
				MCCOMErr
				#endif
				retval;
			#else
				TMCColorRGB childresult;
				MCCOMErr retval;
			#endif
			retval = shader->GetColor(childresult, fullArea, shadingIn);
			result = childresult.Intensity();
			return retval;
		}
	}
	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x040000)
real TerrainToNormal::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real TerrainToNormal::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#else
MCCOMErr TerrainToNormal::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (shader != NULL) 
	{
		if ((ChildOutput & kUsesGetColor) == kUsesGetColor) 
		{
			return shader->GetColor(result, fullArea, shadingIn);
		}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) 
		{
			#if (VERSIONNUMBER >= 0x030000)
				real
			#else
				MCCOMErr
			#endif
			retval;
			real childresult;
			retval = shader->GetValue(childresult, fullArea, shadingIn);
			result.Set(childresult, childresult, childresult
			#if (VERSIONNUMBER >= 0x040000)
				, 1.0f
			#endif
				);
			return retval;
		}
		else if ((ChildOutput & kUsesDoShade) == kUsesDoShade) 
		{
			ShadingOut childresult;
			shader->DoShade(childresult, shadingIn);
			result = childresult.fColor;
		}

	}

	return MC_S_OK;

}


MCCOMErr TerrainToNormal::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (shader != NULL)
	{
		if ((ChildOutput & kUsesDoShade) == kUsesDoShade) 
		{		
			return shader->DoShade(result, shadingIn);
		}
		else if ((ChildOutput & kUsesGetColor) == kUsesGetColor)
		{
			#if (VERSIONNUMBER >= 0x040000)
				TMCColorRGBA childresult;
				#if (VERSIONNUMBER >= 0x030000)
				real
				#else
				MCCOMErr
				#endif
				retval;
			#else
				TMCColorRGB childresult;
				MCCOMErr retval;
			#endif
			boolean fullArea;
			retval = shader->GetColor(childresult, fullArea, shadingIn);
			result.fColor = childresult;
			result.fFullAreaDone = fullArea;
		}
		else if ((ChildOutput & kUsesGetValue) == kUsesGetValue) 
		{
			real childresult;
			#if (VERSIONNUMBER >= 0x030000)
				real
			#else
				MCCOMErr
			#endif
			retval;
			boolean fullArea;
			retval = shader->GetValue(childresult, fullArea, shadingIn);
			result.fColor.Set(childresult, childresult, childresult
			#if (VERSIONNUMBER >= 0x040000)
				, retval
			#endif
			);
			result.fFullAreaDone = fullArea;

		}
	}
	return MC_S_OK;
}
