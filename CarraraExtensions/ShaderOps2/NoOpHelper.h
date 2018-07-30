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
#ifndef __NoOpHelper__
#define __NoOpHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "MCCountedPtr.h"
#include "BasicShader.h" 
#include "DCGUtil.h"
#include "shadertypes.h"
#include <string.h>


template<class T> class NoOpHelper : public TBasicShader{


public:
	NoOpHelper<T>()  {
		extensionDataChangeDirty = true;
	};

	~NoOpHelper<T>(void){
	};

	STANDARD_RELEASE;

	virtual int32 MCCOMAPI GetParamsBufferSize() const { 
		return sizeof(T);
	};

	virtual void* MCCOMAPI GetExtensionDataBuffer() {
		return ((void*) &(fData));
	};

	virtual boolean	MCCOMAPI IsEqualTo(I3DExShader* aShader){
		return (fData == static_cast<NoOpHelper<T>*>(aShader)->fData);
	};

	virtual MCCOMErr MCCOMAPI ExtensionDataChanged() {
		extensionDataChangeDirty = true;
		return MC_S_OK;
	};

	virtual boolean	MCCOMAPI WantsTransform() {
		return false;
	};

	virtual EShaderOutput MCCOMAPI GetImplementedOutput() {
		return (EShaderOutput)(kUsesGetValue + kUsesGetColor + kUsesGetVector + kUsesGetShaderApproxColor); 
	};
	
protected:

	void get2DVector(TVector3& result, ShadingIn& shadingIn)
	{
		ShadingIn shadingInToUse = shadingIn;
		boolean fullArea = false;

		TVector2 area[4] =
		{
				  shadingIn.fUV - shadingIn.fUVx + shadingIn.fUVy ,
				  shadingIn.fUV + shadingIn.fUVx + shadingIn.fUVy ,
				  shadingIn.fUV + shadingIn.fUVx - shadingIn.fUVy ,
				  shadingIn.fUV - shadingIn.fUVx - shadingIn.fUVy
		};
		real32 hyp[4] = 
		{
			sqrt(sqr(shadingIn.fUV, area[0])) ,
			sqrt(sqr(shadingIn.fUV, area[1])) ,
			sqrt(sqr(shadingIn.fUV, area[2])) ,
			sqrt(sqr(shadingIn.fUV, area[3])) ,

		};

		real32 libresult[5];
		real32 step = (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;

		TVector3 AVec, BVec;
		
		shadingInToUse.fUV.SetValues(shadingIn.fUV.x - step, shadingIn.fUV.y);
		GetValue(libresult[0], fullArea, shadingInToUse);

		shadingInToUse.fUV.SetValues(shadingIn.fUV.x, shadingIn.fUV.y - step);
		GetValue(libresult[1], fullArea, shadingInToUse);

		shadingInToUse.fUV.SetValues(shadingIn.fUV.x + step, shadingIn.fUV.y);
		GetValue(libresult[2], fullArea, shadingInToUse);

		shadingInToUse.fUV.SetValues(shadingIn.fUV.x, shadingIn.fUV.y + step);
		GetValue(libresult[3], fullArea, shadingInToUse);
		
		TVector2 uv(libresult[2] - libresult[0], libresult[3] - libresult[1]);

		AVec = -shadingIn.fIsoU;	
		BVec = -shadingIn.fIsoV;	

		AVec = AVec * uv.x;
		BVec = BVec * uv.y;
		result = AVec + BVec;
	}

	void get3DVector(TVector3& result, ShadingIn& shadingIn)
	{

		ShadingIn shadingInToUse = shadingIn;

		real32 localstep = 0, globalstep = 0;

		if (childFlags.fNeedsPointLoc)
		{
			TVector3 area[4] =
			{
					  shadingIn.fPointLoc - shadingIn.fPointLocx + shadingIn.fPointLocy ,
					  shadingIn.fPointLoc + shadingIn.fPointLocx + shadingIn.fPointLocy ,
					  shadingIn.fPointLoc + shadingIn.fPointLocx - shadingIn.fPointLocy ,
					  shadingIn.fPointLoc - shadingIn.fPointLocx - shadingIn.fPointLocy
			};
			real32 hyp[4] = 
			{
				sqrt(sqr(shadingIn.fPointLoc, area[0])) ,
				sqrt(sqr(shadingIn.fPointLoc, area[1])) ,
				sqrt(sqr(shadingIn.fPointLoc, area[2])) ,
				sqrt(sqr(shadingIn.fPointLoc, area[3])) ,
			};
			localstep = (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;
		}

		if (childFlags.fNeedsPoint)
		{
			TVector3 area[4] =
			{
					  shadingIn.fPoint - shadingIn.fPointx + shadingIn.fPointy ,
					  shadingIn.fPoint + shadingIn.fPointx + shadingIn.fPointy ,
					  shadingIn.fPoint + shadingIn.fPointx - shadingIn.fPointy ,
					  shadingIn.fPoint - shadingIn.fPointx - shadingIn.fPointy
			};
			real32 hyp[4] = 
			{
				sqrt(sqr(shadingIn.fPoint, area[0])) ,
				sqrt(sqr(shadingIn.fPoint, area[1])) ,
				sqrt(sqr(shadingIn.fPoint, area[2])) ,
				sqrt(sqr(shadingIn.fPoint, area[3])) ,
			};
			globalstep = (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;
		}

		real32 libresult[6];

		TVector3 AVec, BVec, CVec;

		shadingIn.fPointLocx.Normalize(BVec);
		shadingIn.fPointLocy.Normalize(AVec);

		BVec = -BVec;
		CVec = AVec ^ BVec;

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x - globalstep, shadingIn.fPoint.y, shadingIn.fPoint.z);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x - localstep, shadingIn.fPointLoc.y, shadingIn.fPointLoc.z);
		GetValue(libresult[0], fullArea, shadingInToUse);

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x, shadingIn.fPoint.y - globalstep, shadingIn.fPoint.z);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y - localstep, shadingIn.fPointLoc.z);
		GetValue(libresult[1], fullArea, shadingInToUse);

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x, shadingIn.fPoint.y, shadingIn.fPoint.z - globalstep);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y, shadingIn.fPointLoc.z - localstep);
		GetValue(libresult[2], fullArea, shadingInToUse);

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x + globalstep, shadingIn.fPoint.y, shadingIn.fPoint.z);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x + localstep, shadingIn.fPointLoc.y, shadingIn.fPointLoc.z);
		GetValue(libresult[3], fullArea, shadingInToUse);

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x, shadingIn.fPoint.y + globalstep, shadingIn.fPoint.z);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y + localstep, shadingIn.fPointLoc.z);
		GetValue(libresult[4], fullArea, shadingInToUse);

		shadingInToUse.fPoint.SetValues(shadingIn.fPoint.x, shadingIn.fPoint.y, shadingIn.fPoint.z + globalstep);
		shadingInToUse.fPointLoc.SetValues(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y, shadingIn.fPointLoc.z + localstep);
		GetValue(libresult[5], fullArea, shadingInToUse);

		
		AVec = AVec * (libresult[3] - libresult[0]);
		BVec = BVec * (libresult[4] - libresult[1]);
		CVec = CVec * (libresult[5] - libresult[2]);

		result = AVec + BVec + CVec;
	}

	ShadingFlags combineFlags, EmptyChildFlags;
	T fData;
	boolean extensionDataChangeDirty;

};



#endif