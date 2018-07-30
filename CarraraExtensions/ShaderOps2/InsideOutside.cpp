/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#include "InsideOutside.h"
#include "ShaderOps2DLL.h"


const MCGUID CLSID_InsideOutside(R_CLSID_InsideOutside);

real InsideOutside::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	real alpha = 1;

	if (normalFlipped)
		result = 1.0f;
	else
		result = 0;

	return alpha;
}

EShaderOutput InsideOutside::GetImplementedOutput() 
{
	return (EShaderOutput)(kUsesGetValue + kUsesDoShade); 
};

boolean InsideOutside::HasExplicitLightingModel(const LightingContext& lightingContext)
{
	normalFlipped = lightingContext.fNormalFlipped;
	return true;
}

#if VERSIONNUMBER >= 0x070000
MCCOMErr InsideOutside::ShadeAndLight2(LightingDetail& result,const LightingContext& lightingContext,I3DShLightingModel* inDefaultLightingModel,TAbsorptionFunction* absorptionFunction)
#else
MCCOMErr InsideOutside::ShadeAndLight(LightingDetail& result,const LightingContext& lightingContext, I3DShLightingModel* inDefaultLightingModel)
#endif
{
	normalFlipped = lightingContext.fNormalFlipped;
	return MC_E_NOTIMPL;
}