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
#include <stdio.h>

#define boBigEndian 0
#define boLittleEndian 1

#define gcUnknown 0
#define gcUSGS 1
#define gcVistaPro 2
#define gcWorldMachine 3
#define gcTerragen 4
#define gcHGT 5
#define gcBIL 6
#define gcPGM 7
#define gcArcInfoASCII 8
#define gcGridFloat 9
#define gcPlanetaryDataSystem 10

#define TRUE 1
#define FALSE 0

typedef unsigned char boolean;

typedef union {
	float 	value;
	char	elements[4];
	unsigned int		intValue;
} EndianFloat;

typedef union {
	double 	value;
	char	elements[8];
} EndianDouble;

typedef union
{
	short value;
	char elements[2];
} EndianShort;

typedef union
{
	unsigned short value;
	char elements[2];
} EndianUnsignedShort;

typedef union
{
	int value;
	char elements[4];
	
} EndianInt;

typedef union
{
	char value;
	char elements[1];
} EndianUnsignedChar;

typedef struct
{
	double x, y, z, w;
} TVector4d;

typedef struct
{
	double x, y, z;
} TVector3d;

typedef struct
{
	double east, north;
} TVector2d;

typedef struct {
	double gpCoordinateY;
	unsigned short runLength;
} USGSDEMRunInfo;

typedef union
{
	int intValue;
	unsigned int uintValue;
	float floatValue;
} NoData;


typedef struct gcPrivateData_struct {
	//global
	boolean flipDirection[2];
	unsigned long fileFormat;
	float *elevation;
	boolean useExternalElevation;
	unsigned int dataStart;
	float elevationScale; //this * values read to make min and max elevation in meters.
	NoData noData;
    boolean hasNoData;
	float metersPerArcSecondPolar;
	float metersPerArcSecondEquator;
	boolean isWholeNumber;  //according to wiki a float can accurately represent -16777216 to 16777216 as whole numbers
							//which is good enough for 16bit integer elevations. This flag means we think we're dealing with
							//a format which is represented by whole numbers (at least until we apply scaling and normalization)
	int elevationUnit;

	//used when doing sequenced files and you want succeeding frames to maintain consistent elevations
	float overrideMinElevation, overrideMaxElevation;
	unsigned char overrideElevationMinMax;

	//USGS
	USGSDEMRunInfo* demRunInfo;
	double demLowestCoordinateY;
	long demMaxElevation;
	long demMinElevation;
	double demResolution[3];

	//BIL/HGT
	unsigned int numBits;
	long byteOrder;

	//ArcInfoAscii
	long lastHeaderASCII;

	//PDS
	float baseHeight;
	float scaleFactor;

	//progressbar callback
	void (*progressCallback)(void *, float);
	void* callbackData;
} gcPrivateData;

typedef struct USGSDEMBRecord_struct {
	unsigned short rowId;
	unsigned short colId;
	unsigned short runLength;
	unsigned long alwaysOne;
	double gpCoordinateX, gpCoordinateY;
	double localElevation;
	double minElevation;
	double maxElevation;
} USGSDEMBRecord;

typedef struct gcAdaptiveFacet_struct {
	unsigned long facetIndex;
	double error;
	unsigned long errorLocation[2];
	int worstEdgeIndex;
} gcAdaptiveFacet;

extern char GetTiny(FILE *fp);
extern unsigned char GetUnsignedTiny(FILE *fp);
extern short GetShortBigEndian(FILE *fp);
extern unsigned short GetUnsignedShortBigEndian(FILE *fp);
extern short GetShortLittleEndian(FILE *fp);
extern unsigned short GetUnsignedShortLittleEndian(FILE *fp);
extern int GetIntBigEndian(FILE *fp);
extern int GetIntLittleEndian(FILE *fp);
extern float GetRealLittleEndian(FILE *fp);
extern void WriteRealLittleEndian(FILE *fp, float);
extern float GetRealBigEndian(FILE *fp);
extern double GetDoubleLittleEndian(FILE *fp);
extern double GetUSGSReal(FILE *fp);
extern double GetUSGSDouble(FILE *fp);
extern double GetUSGSRealN(FILE *fp, unsigned long lCharacterLimit);
extern int GetUSGSInt(FILE *fp);
extern char skipPGMLine(FILE *fp);
extern unsigned int getPGMInt(FILE *fp);

extern void WriteUnsignedTiny(FILE *fp, unsigned char value);
extern void WriteShortBigEndian(FILE *fp, short value);
extern void WriteShortLittleEndian(FILE *fp, short value);
extern void WriteIntBigEndian(FILE *fp, int value);
extern void WriteIntLittleEndian(FILE *fp, int value);