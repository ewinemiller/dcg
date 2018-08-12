/*  Anything Goos - plug-in for LightWave
Copyright (C) 2009 Eric Winemiller

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
#ifndef __BoundingBox__
#define __BoundingBox__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include <lwtypes.h>

typedef struct BoundingBox_t {
	LWFVector min, max;
} BoundingBox, *BoundingBox_p;

void addFirstPointToBoundingBox(BoundingBox_p boundingBox, const LWFVector *point)
{
	boundingBox->min[0] = boundingBox->max[0] = (*point)[0];
	boundingBox->min[1] = boundingBox->max[1] = (*point)[1];
	boundingBox->min[2] = boundingBox->max[2] = (*point)[2];

}

void addPointToBoundingBox(BoundingBox_p boundingBox, const LWFVector *point)
{
	if ((*point)[0] < boundingBox->min[0])
	{
		boundingBox->min[0] = (*point)[0];
	}
	else if ((*point)[0] > boundingBox->max[0])
	{
		boundingBox->max[0] = (*point)[0];
	}
	if ((*point)[1] < boundingBox->min[1])
	{
		boundingBox->min[1] = (*point)[1];
	}
	else if ((*point)[1] > boundingBox->max[1])
	{
		boundingBox->max[1] = (*point)[1];
	}
	if ((*point)[2] < boundingBox->min[2])
	{
		boundingBox->min[2] = (*point)[2];
	}
	else if ((*point)[2] > boundingBox->max[2])
	{
		boundingBox->max[2] = (*point)[2];
	}
}

void getBoundingBoxCenter(const BoundingBox_p boundingBox, LWFVector *center)
{
	(*center)[0] = (boundingBox->min[0] + boundingBox->max[0]) * 0.5f;
	(*center)[1] = (boundingBox->min[1] + boundingBox->max[1]) * 0.5f;
	(*center)[2] = (boundingBox->min[2] + boundingBox->max[2]) * 0.5f;
}

#endif