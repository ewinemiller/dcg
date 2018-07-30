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
#include "GCBump.h"
#include "IShComponent.h"


#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GCBump(R_CLSID_GCBump);
#else
const MCGUID CLSID_GCBump={R_CLSID_GCBump};
#endif

GCBump::GCBump()	
{
}

GCBump::~GCBump()
{
}
	
void* GCBump::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

boolean GCBump::IsEqualTo(I3DExShader* aShader)
{
	return true;
}

MCCOMErr GCBump::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fNeedsIsoUV = true;
	theFlags.fConstantChannelsMask = 0;
	return MC_S_OK;
}

EShaderOutput GCBump::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetValue + kUsesGetVector);
}

MCCOMErr GCBump::ExtensionDataChanged()
{
	return MC_S_OK;
}

MCCOMErr GCBump::GetDem(ShadingIn &shadingIn, DEMPrim** GCBump)
{
	if (shadingIn.fInstance != NULL)
	{
		TMCCountedPtr<DEMPrim> dem;
		TMCCountedPtr<I3DShObject> baseobject;
		TMCCountedPtr<I3DShExternalPrimitive> extprimitive;
		TMCCountedPtr<I3DShPrimitiveComponent> primcomp;
		TMCCountedPtr<IShComponent> component;

		shadingIn.fInstance->Get3DObject(&baseobject);
		ThrowIfNil(baseobject);

		baseobject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extprimitive);
		ThrowIfNil(extprimitive);

		extprimitive->GetPrimitiveComponent(&component);
		ThrowIfNil(component);

		component->QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primcomp);
		ThrowIfNil(primcomp);

		primcomp->QueryInterface(IID_DEMPrim,(void**)&dem);
		if (dem)
		{
			TMCCountedGetHelper<DEMPrim> result(GCBump);
			result = dem;
			return MC_S_OK;
		}
	}
	return MC_S_FALSE;
}

real GCBump::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	TMCCountedPtr<DEMPrim> dem;
	GetDem(shadingIn, &dem);
	if (dem) 
	{
		result = dem->getShaderValue(shadingIn.fUV, fiBilinear);			
	}

	return MC_S_OK;
}

real GCBump::GetVector(TVector3& result,ShadingIn& shadingIn)
{
	TMCCountedPtr<DEMPrim> dem;
	GetDem(shadingIn, &dem);
	if (dem) 
	{
		real32 step = GetVectorSampleStep2D(shadingIn);

		double libresult[5];

		TVector3 AVec, BVec;
		TVector2 tempUV;
		
		tempUV.x = shadingIn.fUV.x - step;
		tempUV.y = shadingIn.fUV.y;
		libresult[0] = dem->getShaderValue(tempUV, fiBilinear);

		tempUV.x = shadingIn.fUV.x;
		tempUV.y = shadingIn.fUV.y - step;
		libresult[1] = dem->getShaderValue(tempUV, fiBilinear);

		tempUV.x = shadingIn.fUV.x + step;
		tempUV.y = shadingIn.fUV.y;
		libresult[2] = dem->getShaderValue(tempUV, fiBilinear);

		tempUV.x = shadingIn.fUV.x;
		tempUV.y = shadingIn.fUV.y + step;
		libresult[3] = dem->getShaderValue(tempUV, fiBilinear);
		
		tempUV.x = shadingIn.fUV.x;
		tempUV.y = shadingIn.fUV.y;
		libresult[4] = dem->getShaderValue(tempUV, fiBilinear);

		TVector2 uv(0,0);

		uv.x = libresult[2] - libresult[0];
		uv.y = libresult[3] - libresult[1];

		AVec = -shadingIn.fIsoU;	
		BVec = -shadingIn.fIsoV;	

		AVec = AVec * uv.x;
		BVec = BVec * uv.y;
		result = AVec + BVec;
	}
	return MC_S_OK;
}



