/*  Ground Control Library - Elevation map tools
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
#include "GroundControlLibInternal.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//motorola = big-endian
//intel = little-endian

//function names match what the value is encoded as in the file
//functions will rearrange to what is needed in memory
char GetTiny(FILE *fp)
{
	char temp;
	fread( &temp, 1, 1, fp );
	return temp;
}

unsigned char GetUnsignedTiny(FILE *fp)
{
	EndianUnsignedChar temp;
	fread( &temp, 1, 1, fp );
	return temp.value;
}

void WriteUnsignedTiny(FILE *fp, unsigned char value)
{
	fwrite(&value, 1, 1, fp);
}

short GetShortBigEndian(FILE *fp)
{
	EndianShort temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#else
	fread(&temp.elements[0], 1, 2, fp );
	#endif
	return temp.value;
}

void WriteShortBigEndian(FILE *fp, short value)
{
	EndianShort temp;
	temp.value = value;
#ifdef	LITTLE_ENDIAN
	fwrite(&temp.elements[1], 1, 1, fp);
	fwrite(&temp.elements[0], 1, 1, fp);
#else
	fwrite(&temp.elements[0], 1, 2, fp);
#endif
}
unsigned short GetUnsignedShortBigEndian(FILE *fp)
{
	EndianUnsignedShort temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#else
	fread(&temp.elements[0], 1, 2, fp );
	#endif
	return temp.value;
}

short GetShortLittleEndian(FILE *fp)
{
	EndianShort temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[0], 1, 2, fp );
	#else
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#endif
	return temp.value;
}


void WriteShortLittleEndian(FILE *fp, short value)
{
	EndianShort temp;
	temp.value = value;
#ifdef	LITTLE_ENDIAN
	fwrite(&temp.elements[0], 1, 2, fp);
#else
	fwrite(&temp.elements[1], 1, 1, fp);
	fwrite(&temp.elements[0], 1, 1, fp);
#endif
}


unsigned short GetUnsignedShortLittleEndian(FILE *fp)
{
	EndianUnsignedShort temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[0], 1, 2, fp );
	#else
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#endif
	return temp.value;
}

int GetIntBigEndian(FILE *fp)
{
	EndianInt temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[3], 1, 1, fp );
	fread(&temp.elements[2], 1, 1, fp );
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#else
	fread(&temp.elements[0], 1, 4, fp );
	#endif
	return temp.value;
}

void WriteIntBigEndian(FILE *fp, int value)
{
	EndianInt temp;
	temp.value = value;
#ifdef	LITTLE_ENDIAN
	fwrite(&temp.elements[3], 1, 1, fp);
	fwrite(&temp.elements[2], 1, 1, fp);
	fwrite(&temp.elements[1], 1, 1, fp);
	fwrite(&temp.elements[0], 1, 1, fp);
#else
	fwrite(&temp.elements[0], 1, 4, fp);
#endif
}


int GetIntLittleEndian(FILE *fp)
{
	EndianInt temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[0], 1, 4, fp );
	#else
	fread(&temp.elements[3], 1, 1, fp );
	fread(&temp.elements[2], 1, 1, fp );
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#endif
	return temp.value;
}

void WriteIntLittleEndian(FILE *fp, int value) {
	EndianInt temp;
	temp.value = value;
#ifdef	LITTLE_ENDIAN
	fwrite(&temp.elements[0], 1, 4, fp);
#else
	fwrite(&temp.elements[3], 1, 1, fp);
	fwrite(&temp.elements[2], 1, 1, fp);
	fwrite(&temp.elements[1], 1, 1, fp);
	fwrite(&temp.elements[0], 1, 1, fp);
#endif
}

float GetRealLittleEndian(FILE *fp)
{
	EndianFloat temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[0], 1, 4, fp );
	#else
	fread(&temp.elements[3], 1, 1, fp );
	fread(&temp.elements[2], 1, 1, fp );
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#endif
	return temp.value;
}

void WriteRealLittleEndian(FILE *fp, float value) {
	EndianFloat temp;
	temp.value = value;
#ifdef	LITTLE_ENDIAN
	fwrite(&temp.elements[0], 1, 4, fp);
#else
	fwrite(&temp.elements[3], 1, 1, fp);
	fwrite(&temp.elements[2], 1, 1, fp);
	fwrite(&temp.elements[1], 1, 1, fp);
	fwrite(&temp.elements[0], 1, 1, fp);
#endif
}

float GetRealBigEndian(FILE *fp)
{
	EndianFloat temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[3], 1, 1, fp );
	fread(&temp.elements[2], 1, 1, fp );
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#else
	fread(&temp.elements[0], 1, 4, fp );
	#endif
	return temp.value;
}

double GetDoubleLittleEndian(FILE *fp)
{
	EndianDouble temp;
	#ifdef	LITTLE_ENDIAN
	fread(&temp.elements[0], 1, 8, fp );
	#else
	fread(&temp.elements[7], 1, 1, fp );
	fread(&temp.elements[6], 1, 1, fp );
	fread(&temp.elements[5], 1, 1, fp );
	fread(&temp.elements[4], 1, 1, fp );
	fread(&temp.elements[3], 1, 1, fp );
	fread(&temp.elements[2], 1, 1, fp );
	fread(&temp.elements[1], 1, 1, fp );
	fread(&temp.elements[0], 1, 1, fp );
	#endif
	return temp.value;
}
double GetUSGSReal(FILE *fp)
{
	unsigned long i = 0;
	char temp, val[64];
	fread( &temp, 1, 1, fp );
	while ((temp == 32)||(temp == '!')||(temp == 10)||(temp == 13))
	{
		fread( &temp, 1, 1, fp );
	}
	val[i] = temp;
	fread( &temp, 1, 1, fp );
	while ((temp != 32)&&(temp != '-')&&(temp != 10)&&(temp != 13))
	{
		if (temp != '!'){
			i++;
			val[i] = temp;
			if (val[i] == 'D')
			{
				val[i] = 'E';
			}
			fread( &temp, 1, 1, fp );
		}
	}
	/* if we stopped because we hit a negative instead of a space
		, back up one so the next number doesn't loose it */
	if (temp == '-')
	{
		fseek(fp, -1, SEEK_CUR);
	}
	val[++i] = 0;
	return atof(val);
}

double GetUSGSDouble(FILE *fp)
{
	unsigned long i = 0;
	char temp, val[64];
	fread( &temp, 1, 1, fp );
	while ((temp == 32)||(temp == '!')||(temp == 10)||(temp == 13))
	{
		fread( &temp, 1, 1, fp );
	}
	val[i] = temp;
	fread( &temp, 1, 1, fp );
	while ((temp != 32)&&(temp != '-')&&(temp != 10)&&(temp != 13))
	{
		if (temp != '!'){
			i++;
			val[i] = temp;
			if (val[i] == 'D')
			{
				val[i] = 'E';
			}
			fread( &temp, 1, 1, fp );
		}
	}
	/* if we stopped because we hit a negative instead of a space
		, back up one so the next number doesn't loose it */
	if (temp == '-')
	{
		fseek(fp, -1, SEEK_CUR);
	}
	val[++i] = 0;
	return atof(val);
}

double GetUSGSRealN(FILE *fp, unsigned long lCharacterLimit)
{
	char temp[64];
	unsigned long i;
	fread( &temp, 1, lCharacterLimit, fp );
	for (i = 0; i < lCharacterLimit; i++)
	{	
		if (temp[i] == 'D')
		{
			temp[i] = 'E';
		}
	}
	temp[lCharacterLimit] = 0;
    return atof(temp);
}

int GetUSGSInt(FILE *fp)
{
	int retval;

	fscanf( fp, "%d", &retval );
	return retval;
}

char skipPGMLine(FILE *fp)
{
	char junk;
	fread( &junk, 1, 1, fp );
	while ((junk != 10)&&(junk != 13)){
		fread( &junk, 1, 1, fp );
	}
	return junk;
}

unsigned int getPGMInt(FILE *fp)
{
	unsigned long i = 0;
	char junk, val[64];
	fread( &junk, 1, 1, fp );
	if (junk == '#')
	{
		junk = skipPGMLine(fp);
	}
	while ((junk == 32)||(junk == 10)||(junk == 13)||(junk == 9))
	{
		fread( &junk, 1, 1, fp );
		if (junk == '#')
		{
			junk = skipPGMLine(fp);
		}
	}
	val[i] = junk;
	fread( &junk, 1, 1, fp );
	while ((junk != 32)&&(junk != 10)&&(junk != 13)&&(junk != 9))
	{
		val[++i] = junk;
		fread( &junk, 1, 1, fp );
	}
	val[++i] = 0;
	return atoi(val);
}

