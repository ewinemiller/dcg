/*  Anything Grows - plug-in for Carrara
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
#ifndef __IAnythingGrowsStrandGenerator__
#define __IAnythingGrowsStrandGenerator__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"

#define R_IID_IAnythingGrowsStrandGenerator 0x3b78982d,0x5630,0x4cd7,0xa4,0xa7,0x7a,0x3d,0x8b,0xc3,0xdf,0x36
const MCGUID IID_IAnythingGrowsStrandGenerator(R_IID_IAnythingGrowsStrandGenerator);

class AnythingGrowsStrands : public TMCCountedObject {
public:
	//one element/strand.
	//ElemCount is # of strands
	TMCArray<uint8> vertexCounts;
	TMCArray<TVector2> strandUV;
	TMCArray<uint32> uvSpaceID;


	//vertex data
	//location starts at the sum of all previous strands vertexCounts
	TMCArray<TVector3> vertices;
	TMCArray<real32> vertexWidths;
};


struct IAnythingGrowsStrandGenerator : public IMCUnknown {
	virtual MCCOMErr MCCOMAPI GetStrands(AnythingGrowsStrands** strands) = 0;
};

#endif