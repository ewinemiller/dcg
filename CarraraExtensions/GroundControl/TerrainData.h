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
#ifndef __TerrainData__
#define __TerrainData__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCString.h"
#include "copyright.h"

enum TerrainRenderSize {trs32, trs64, trs128, trs256, trs512, trs1024, trs2048, trs4096};
enum TerrainImageSize {tis256, tis512, tis1024};
#if (VERSIONNUMBER == 0x010000)
struct TerrainData 
{
	int32 version;
	int32 res1, res2, res3, res4;
	int32 width, height;
	int32 dept;
	int32 sizeP, sizeR;
	int32 notC;
	int32 seed;
	TMCDynamicString H001;
	real32 sizeX, sizeY, sizeZ;
	boolean smooth;

};
#else
struct TerrainData 
{
	int32 version;
	int32 res1, res2, res3, res4;
	int32 width, height;
	int32 dept;
	int32 sizeP, sizeR, sizeI;
	int32 notC;
	int32 seed;
	TMCDynamicString H001;
	real32 sizeX, sizeY, sizeZ;
	boolean smooth;

};
#endif
#endif