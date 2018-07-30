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
#ifndef __DEMPrimData__
#define __DEMPrimData__  


#if CP_PRAGMA_ONCE
#pragma once
#endif


struct DEMPrimData 
{
	real32 fResolution;
	real32 sizeX;
	real32 sizeY;
	real32 topElevation;
	real32 baseElevation;
	int32 lPreviewSize;
	TMCString255 sDescription;
	TMCString255 sResolution;
	real32 adaptiveError;
	TMCString255 facets;
	real32 animate;
	boolean interpolateFrames;
	ActionNumber meshType;
};

#endif