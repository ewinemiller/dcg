/*  Carrara plug-in utilities
    Copyright (C) 2000 Eric Winemiller

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
#include "DCGShaderHelper.h"
#include "dcgutil.h"

real32 DCGShaderHelper::GetVectorSampleStep2D(const ShadingIn& shadingIn)
{
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

	return (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;
}

real32 DCGShaderHelper::GetVectorSampleStep3D(const ShadingIn& shadingIn, const int32& LocalOrGlobal)
{
	if (LocalOrGlobal == SPACE_LOCAL)
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
		return (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;
	}
	else
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
		return (hyp[0] + hyp[1] + hyp[2] + hyp[3]) * 0.25f;
	}
}
