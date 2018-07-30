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
#ifndef __DCGUTIL__
#define __DCGUTIL__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"

#define OPT_AUTO 0
#define OPT_OCTAL_TREE 1
#define OPT_XY_QUADRANT 2
#define OPT_XZ_QUADRANT 3
#define OPT_YZ_QUADRANT 4
#define OPT_LONG_SIDE 5
#define OPT_NONE 6

#define SPACE_LOCAL 0
#define SPACE_GLOBAL 1


struct FacetEdges
{
	uint8 lCurrentIndex;
	uint32 lEdgeIndex[3];
};

template <class c>
inline real32 sqr(c pfIn)
{
	return pfIn * pfIn;
}

inline real32 sqr(const TVector3 &first, const TVector3 &second) 
{
	return sqr(first.x - second.x) + sqr(first.y - second.y) + sqr(first.z - second.z);
}

inline real32 sqr(const TVector2 &first, const TVector2 &second) 
{
	return sqr(first.x - second.x) + sqr(first.y - second.y);
}

template<class c>
inline real32 distance(const c &first,const c &second) 
{
	return sqrt(sqr(first, second));
}

template<class c>
inline c min(c first, c second)
{
	if (first < second) return first;
	else return second;
}

inline double min(double first, double second)
{
	if (first < second) return first;
	else return second;
}
inline double max(double first, double second)
{
	if (first > second) return first;
	else return second;
}
#define FPOSINF 1e20f

inline real32 GetArea(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3)
{
	real32 fEdgeLength[3];

	real32 cos;
	real32 theta;

	fEdgeLength[0] = sqrt(sqr(pt1, pt2));
	fEdgeLength[1] = sqrt(sqr(pt2, pt3));
	fEdgeLength[2] = sqrt(sqr(pt3, pt1));
	
	if ((fEdgeLength[1] == 0)||(fEdgeLength[2] == 0)||(fEdgeLength[2] == 0))
	{
		return 0;
	}
	else
	{
		cos = (sqr(fEdgeLength[1]) + sqr(fEdgeLength[2]) - sqr(fEdgeLength[0]))
			/ (2 * fEdgeLength[1] * fEdgeLength[2]);

		if (cos < -1.0) { cos = -1.0;}
		else if (cos > 1.0) {cos = 1.0;}
		theta = acos(cos);

		return 0.5 * fEdgeLength[1] * fEdgeLength[2] * sin(theta);
	}


}

#endif
