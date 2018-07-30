/*  Anything Grooves - plug-in for Carrara
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
#ifndef __DEFPOINT__
#define __DEFPOINT__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Apitypes.h"
#include "MCBasicdefines.h"
#include "COMUtilities.h"

class DefPoint {
	public:
		DefPoint() {bHasData = false; bFilled = false;};
		TVertex3D vertex;
		TVertex3D original;
		boolean bHasData;
		boolean bFilled;
		real32 displacement;
	};

class DefPointArray {
	public:
		DefPointArray();
		~DefPointArray();
		void		Allocate(const int32 lowU, const int32 highU, const int32 lowV, const int32 highV);
		void	Deallocate();
		DefPoint*	Point(int32 U, int32 V);
		DefPoint*	GetTop(int32 U);
		DefPoint*	GetBottom(int32 U);
		DefPoint*	GetLeft(int32 V);
		DefPoint*	GetRight(int32 V);
		DefPoint*	GetLeftMost(int32 U, int32 V);
		DefPoint*	GetRightMost(int32 U, int32 V);
	private:
		void		Clear();
		DefPoint* data;
		DefPoint* leftpointsU;
		DefPoint* rightpointsU;
		DefPoint* toppointsV;
		DefPoint* bottompointsV;
		int32 mlowU, mhighU, mlowV, mhighV, mU, mV, mUmV;
		int32 moldU, moldV;
	};

#endif