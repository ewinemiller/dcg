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
#ifndef __AGrPrimData__
#define __AGrPrimData__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#define SPLIT_DUMB 0
#define SPLIT_SHORT 1
#define SPLIT_SMART 2

#define PREVIEW_16 0
#define PREVIEW_32 1
#define PREVIEW_64 2
#define PREVIEW_128 3
#define PREVIEW_FULL 4

struct AGrPrimData {
	int32 lPrimitive;
	real32 fStart, fStop;
	real32 fSmoothAngle;
	real32 lAnimate;
	int32  lU, lV;
	boolean bPreSmooth;
	TMCString255 MasterShader; 
	TMCString255 Warnings; 
	boolean bRefresh;
	boolean bAdaptiveMesh;
	boolean bEnabled;
	int32 lSplitMethod;
	real32 sizeX;
	real32 sizeY;
	real32 sizeZ;
	boolean bEmptyZero;
	int32 lPreviewSize;
	ActionNumber			lObjectPicker;
	};

#endif