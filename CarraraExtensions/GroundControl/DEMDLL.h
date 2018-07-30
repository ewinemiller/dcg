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
#ifndef __DEMDLL__
#define __DEMDLL__  


#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCStream.h"
#include <stdlib.h>

typedef union {
	real64 	value;
	char	elements[8];
} Endianreal64;

typedef union
{
	int32 value;
	char elements[4];
	
} Endianint32;

inline int32 GetIntBigEndian(TMCfstream* stream)
{
	Endianint32 temp;
	#if MC_LITTLE_ENDIAN == 1
	(*stream).read(&temp.elements[3], 1);
	(*stream).read(&temp.elements[2], 1);
	(*stream).read(&temp.elements[1], 1);
	(*stream).read(&temp.elements[0], 1);
	#else
	(*stream).read(&temp.elements[0], 4);
	#endif
	return temp.value;
}

inline int32 GetIntLittleEndian(TMCfstream* stream)
{
	Endianint32 temp;
	#if MC_LITTLE_ENDIAN == 1
	(*stream).read(&temp.elements[0], 4);
	#else
	(*stream).read(&temp.elements[3], 1);
	(*stream).read(&temp.elements[2], 1);
	(*stream).read(&temp.elements[1], 1);
	(*stream).read(&temp.elements[0], 1);
	#endif
	return temp.value;
}

inline double GetDoubleLittleEndian(TMCfstream* stream)
{
	Endianreal64 temp;
	#if MC_LITTLE_ENDIAN == 1
	(*stream).read(&temp.elements[0], 8);
	#else
	(*stream).read(&temp.elements[7], 1);
	(*stream).read(&temp.elements[6], 1);
	(*stream).read(&temp.elements[5], 1);
	(*stream).read(&temp.elements[4], 1);
	(*stream).read(&temp.elements[3], 1);
	(*stream).read(&temp.elements[2], 1);
	(*stream).read(&temp.elements[1], 1);
	(*stream).read(&temp.elements[0], 1);
	#endif
	return temp.value;
}

class updateProgressBar{
public:
	TMCCountedPtr<IMCUnknown> progressKey;
	float progressStart, progressRun;

	updateProgressBar (){
		progressStart = 0;
		progressRun = 1.0f;
	}
};


#ifdef __cplusplus
extern "C" {
#endif

	extern void updateProgressBarCallback(void* data, float farAlong);

#ifdef __cplusplus
}
#endif


#endif