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
#include "GroundControlLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "GroundControlLibInternal.h"
#include "GroundControlLibMathHelpers.h"
#include "DCGBinaryTree.h"
#include "GroundControlLibMeshHelpers.h"

#define EARTH_METERS_PER_ARC_SECOND_POLAR 30.87f
#define EARTH_METERS_PER_ARC_SECOND_EQUATOR 30.922f	
#define FEET_TO_METERS 0.3048f





//use this to keep our stack of our fill area in progress
typedef struct gcFillAreaStackElement_struct gcFillAreaStackElement;

typedef struct gcFillAreaStackElement_struct {
	unsigned int elevationIndex;
	gcFillAreaStackElement *next;
};

void ToUpper(char *s) {
	while (*s) {
		if (islower(*s)) {
			*s = toupper(*s);
		}
		s++;
	}
}

unsigned long flipIndex(const unsigned long index, const unsigned long max)
{
    return max - 1 - index;
}

int FileSize( const char * szFileName ) 
{ 
	struct stat fileStat; 
	int err = stat( szFileName, &fileStat ); 
	if (0 != err) return 0; 
	return fileStat.st_size; 
}

unsigned long gcGetElevationIndex(const gcMapInfo *mapInfo, const unsigned long east, const unsigned long north)
{
	//we import into a grid laid out in rows west to east stacked south to north (first element is sw, last ne)
	return north * mapInfo->samples[GC_EAST] + east;
}

unsigned long gcGetFlippedElevationIndex(const gcMapInfo *mapInfo, const unsigned long east, const unsigned long north) {
    gcPrivateData* privateData = mapInfo->privateData;
    unsigned long localEast = east, localNorth = north;
    if (privateData->flipDirection[GC_EAST])
    {
        localEast = flipIndex(localEast, mapInfo->samples[GC_EAST]);
    }
    if (privateData->flipDirection[GC_NORTH])
    {
        localNorth = flipIndex(north, mapInfo->samples[GC_NORTH]);
    }
    return gcGetElevationIndex(mapInfo, localEast, localNorth);
}

unsigned long gcGetFullPolyCount(const gcMapInfo *mapInfo) {
	return (mapInfo->samples[GC_NORTH] - 1) * (mapInfo->samples[GC_EAST] - 1) * 2;
}

void gcGetEastAndNorthFromElevationIndex(const gcMapInfo *mapInfo, const unsigned int elevationIndex, unsigned int *east, unsigned int *north) { 
	//derive our north and east from elevationIndex
	*north = elevationIndex / mapInfo->samples[GC_EAST];
	*east = elevationIndex % mapInfo->samples[GC_EAST];
}


int gcFloodFillPush(gcFillAreaStackElement **stack, const gcMapInfo *mapInfo, unsigned int east, unsigned int north) {
	gcFillAreaStackElement *topElement = (gcFillAreaStackElement*)calloc(1, sizeof(gcFillAreaStackElement));
	if (!topElement) {
		return GC_ERROR;
	}
	topElement->next = *stack;
	*stack = topElement;
	topElement->elevationIndex = gcGetElevationIndex(mapInfo, east, north);
	return GC_OK;
}

int gcFloodFillPop(gcFillAreaStackElement **stack, const gcMapInfo *mapInfo, unsigned int *east, unsigned int *north) {
	gcFillAreaStackElement *topElement = (*stack);
	if (topElement == NULL) {
		return GC_ERROR;
	}
	gcGetEastAndNorthFromElevationIndex(mapInfo, topElement->elevationIndex, east, north); 
	*stack = topElement->next;
	free(topElement);
	return GC_OK;
}


void gcPopulateAvgPerimeter(const gcMapInfo *mapInfo, unsigned int east, unsigned int north, double* perimeterSum, unsigned long* perimeterCount,  NoData originalValue, NoData newValue) {
	gcPrivateData* privateData = mapInfo->privateData;
	gcFillAreaStackElement *stack = NULL;
    boolean spanNorth, spanSouth;

	if(gcFloodFillPush(&stack, mapInfo, east, north) == GC_ERROR) {
		return;
	}
    
    while(gcFloodFillPop(&stack, mapInfo, &east, &north) == GC_OK)
    {   
		unsigned long lowEast = east, highEast = east;
		long currentEast = 0;
		if (privateData->elevation[gcGetElevationIndex(mapInfo, east, north)] == originalValue.floatValue) {

		for (currentEast = east; currentEast >= 0; currentEast--)
		{
			unsigned long elevationIndex = gcGetElevationIndex(mapInfo, currentEast, north);
			float elevation = privateData->elevation[elevationIndex];
			if (elevation != originalValue.floatValue)
			{
				(*perimeterSum)+=elevation;
				(*perimeterCount)++;
				break;
			}
			privateData->elevation[elevationIndex] = newValue.floatValue;
			lowEast = currentEast;
		}

		for (currentEast = east + 1; currentEast < mapInfo->samples[GC_EAST]; currentEast++)
		{
			unsigned long elevationIndex = gcGetElevationIndex(mapInfo, currentEast, north);
			float elevation = privateData->elevation[elevationIndex];
			if (elevation != originalValue.floatValue)
			{
				(*perimeterSum)+=elevation;
				(*perimeterCount)++;
				break;
			}
			privateData->elevation[elevationIndex] = newValue.floatValue;
			highEast = currentEast;
		}
		spanNorth = spanSouth = FALSE;
		for (currentEast = lowEast; currentEast <= highEast; currentEast++)
		{
			if (north > 0)
			{
				float elevation = privateData->elevation[gcGetElevationIndex(mapInfo, currentEast, north - 1)];

				if (elevation == originalValue.floatValue) 
				{
					if (!spanSouth) {
						if(gcFloodFillPush(&stack, mapInfo, currentEast, north - 1) == GC_ERROR) {
							return;
						}					
						spanSouth = TRUE;
					}
				}
				else if (elevation != newValue.floatValue)
				{
					(*perimeterSum)+=elevation;
					(*perimeterCount)++;
					spanSouth = FALSE;
				}
			}
			if (north < mapInfo->samples[GC_NORTH] - 1)
			{
				float elevation = privateData->elevation[gcGetElevationIndex(mapInfo, currentEast, north + 1)];

				if (elevation == originalValue.floatValue)
				{
					if (!spanNorth) {
						if(gcFloodFillPush(&stack, mapInfo, currentEast, north + 1) == GC_ERROR) {
							return;
						}		
						spanNorth = TRUE;
					}
				}
				else if (elevation != newValue.floatValue)
				{
					(*perimeterSum)+=elevation;
					(*perimeterCount)++;
					spanNorth = FALSE;
				}
			}
		}
        }
    }
}

void gcFloodFill(const gcMapInfo *mapInfo, unsigned int east, unsigned int north, NoData originalValue, NoData newValue) {
	gcPrivateData* privateData = mapInfo->privateData;
	gcFillAreaStackElement *stack = NULL;
    boolean spanNorth, spanSouth;


	if(gcFloodFillPush(&stack, mapInfo, east, north) == GC_ERROR) {
		return;
	}
    
	while(gcFloodFillPop(&stack, mapInfo, &east, &north) == GC_OK)
	{   
		unsigned long lowEast = east, highEast = east;
		long currentEast = 0;

		if (privateData->elevation[gcGetElevationIndex(mapInfo, east, north)] == originalValue.floatValue) {

		for (currentEast = east; currentEast >= 0; currentEast--)
		{
			unsigned long elevationIndex = gcGetElevationIndex(mapInfo, currentEast, north);
			float elevation = privateData->elevation[elevationIndex];
			if (elevation != originalValue.floatValue)
			{
				break;
			}
			privateData->elevation[elevationIndex] = newValue.floatValue;
			lowEast = currentEast;
		}

		for (currentEast = east + 1; currentEast < mapInfo->samples[GC_EAST]; currentEast++)
		{
			unsigned long elevationIndex = gcGetElevationIndex(mapInfo, currentEast, north);
			float elevation = privateData->elevation[elevationIndex];
			if (elevation != originalValue.floatValue)
			{
				break;
			}
			privateData->elevation[elevationIndex] = newValue.floatValue;
			highEast = currentEast;
		}
		spanNorth = spanSouth = FALSE;
		for (currentEast = lowEast; currentEast <= highEast; currentEast++)
		{
			if (north > 0)
			{
				float elevation = privateData->elevation[gcGetElevationIndex(mapInfo, currentEast, north - 1)];

				if (elevation == originalValue.floatValue) 
				{
					if (!spanSouth) {
						if(gcFloodFillPush(&stack, mapInfo, currentEast, north - 1) == GC_ERROR) {
							return;
						}					
						spanSouth = TRUE;
					}
				}
				else if (elevation != newValue.floatValue)
				{
					spanSouth = FALSE;
				}
			}
			if (north < mapInfo->samples[GC_NORTH] - 1)
			{
				float elevation = privateData->elevation[gcGetElevationIndex(mapInfo, currentEast, north + 1)];

				if (elevation == originalValue.floatValue)
				{
					if (!spanNorth) {
						if(gcFloodFillPush(&stack, mapInfo, currentEast, north + 1) == GC_ERROR) {
							return;
						}					
						spanNorth = TRUE;
					}
				}
				else if (elevation != newValue.floatValue)
				{
					spanNorth = FALSE;
				}
			}
		}
        }
	}
}

void gcOverrideElevationRange(gcMapInfo *mapInfo, float minElevation, float maxElevation) {
	gcPrivateData* privateData = mapInfo->privateData;
	privateData->overrideElevationMinMax = GC_TRUE;
	privateData->overrideMinElevation = minElevation / privateData->elevationScale;
	privateData->overrideMaxElevation = maxElevation / privateData->elevationScale;
}

void gcInitialize(gcMapInfo *mapInfo)
{
	gcPrivateData* privateData = NULL;
	mapInfo->privateData = calloc(1, sizeof(gcPrivateData));
	privateData = mapInfo->privateData;
	privateData->elevation = NULL;
	privateData->demRunInfo = NULL;
	privateData->useExternalElevation = GC_FALSE;
	mapInfo->mesh = NULL;
	mapInfo->description = NULL;
	privateData->overrideMinElevation = 0;
	privateData->overrideMaxElevation = 1.0f;
	privateData->overrideElevationMinMax = GC_FALSE;
	privateData->progressCallback = NULL;
	privateData->callbackData = NULL;
    privateData->hasNoData = FALSE;
	privateData->isWholeNumber = FALSE;
}

extern void gcSetExternalBuffer(gcMapInfo *mapInfo, float *externalElevation) {
	gcPrivateData* privateData = mapInfo->privateData;
	mapInfo->bufferSize = mapInfo->samples[GC_EAST] * mapInfo->samples[GC_NORTH];
	privateData->elevation = externalElevation; 
	privateData->useExternalElevation = GC_TRUE;
}

void gcCleanUpMesh(gcMapInfo *mapInfo)
{
	if (mapInfo->mesh) {
		if (mapInfo->mesh->points) {
			free (mapInfo->mesh->points);
			mapInfo->mesh->points = NULL;
		}
		if (mapInfo->mesh->facets) {
			free (mapInfo->mesh->facets);
			mapInfo->mesh->facets = NULL;
		}
		if (mapInfo->mesh->edges) {
			free (mapInfo->mesh->edges);
			mapInfo->mesh->edges = NULL;
		}
		if (mapInfo->mesh->facetEdges) {
			free (mapInfo->mesh->facetEdges);
			mapInfo->mesh->facetEdges = NULL;
		}
		if (mapInfo->mesh->EPercent) {
			free (mapInfo->mesh->EPercent);
			mapInfo->mesh->EPercent = NULL;
		}
		if (mapInfo->mesh->NPercent) {
			free (mapInfo->mesh->NPercent);
			mapInfo->mesh->NPercent = NULL;
		}
		if (mapInfo->description) {
			free (mapInfo->description);
			mapInfo->description = NULL;
		}

		free (mapInfo->mesh);
		mapInfo->mesh = NULL;
	}
}

void gcCleanUp(gcMapInfo *mapInfo)
{
	gcPrivateData* privateData = mapInfo->privateData;

	if (privateData != NULL) {
		if (privateData->elevation && privateData->useExternalElevation == GC_FALSE) {
			free (privateData->elevation);
		}
		privateData->elevation = NULL;
		if (privateData->demRunInfo) {
			free (privateData->demRunInfo);
			privateData->demRunInfo = NULL;
		}
		free(privateData);
		mapInfo->privateData = NULL;
	}

	gcCleanUpMesh(mapInfo);
}

float gcGetElevationByIndex(const gcMapInfo *mapInfo, const unsigned long elevationIndex) {
	gcPrivateData* privateData = mapInfo->privateData;

	if (privateData->elevation && elevationIndex < mapInfo->bufferSize) {
		return privateData->elevation[elevationIndex];
	}
	return 0;
}

float* gcGetElevationLineBuffer(const gcMapInfo *mapInfo, const unsigned long north) {
	gcPrivateData* privateData = mapInfo->privateData;

	if (privateData->elevation && north < mapInfo->samples[GC_NORTH]) {

		return &privateData->elevation[gcGetElevationIndex(mapInfo, 0, north)];
	}
	return 0;
}

float gcGetElevationByCoordinates(const gcMapInfo *mapInfo, const unsigned long east, const unsigned long north) {
	//we import into a grid laid out in rows west to east stacked south to north (first element is sw, last ne)
	unsigned long elevationIndex = north * mapInfo->samples[GC_EAST] + east;
	gcPrivateData* privateData = mapInfo->privateData;

	if (privateData->elevation && elevationIndex < mapInfo->bufferSize) {
		return privateData->elevation[elevationIndex];
	}
	return 0;
}

void gcSetElevationByCoordinates(const gcMapInfo *mapInfo, const unsigned long east, const unsigned long north, const float elevation) {
	//we import into a grid laid out in rows west to east stacked south to north (first element is sw, last ne)
	unsigned long elevationIndex = north * mapInfo->samples[GC_EAST] + east;
	gcPrivateData* privateData = mapInfo->privateData;

	if (privateData->elevation && elevationIndex < mapInfo->bufferSize) {
		privateData->elevation[elevationIndex] = elevation;
	}
}

float gcGetElevationBySample(const gcMapInfo *mapInfo, const unsigned long filtering, const float east, const float north) 
{
	gcPrivateData* privateData = mapInfo->privateData;

	float value = 0;
	float localEast = east, localNorth = north;
	unsigned int blockEast, blockNorth;
	unsigned long swElevationIndex, nwElevationIndex, seElevationIndex, neElevationIndex;
	float fracEast, fracNorth;
	float sw, nw, se, ne;

	if (localEast < 0)
	{
		localEast = 0;
	}
	else if(localEast > 1.0f)
	{
		localEast = 1.0f;
	}

	if (localNorth < 0)
	{
		localNorth = 0;
	}
	else if(localNorth > 1.0f)
	{
		localNorth = 1.0f;
	}

	fracEast = localEast * (mapInfo->samples[GC_EAST] - 1);
	fracNorth = localNorth * (mapInfo->samples[GC_NORTH] - 1);

	blockEast = floor(fracEast);
	blockNorth = floor(fracNorth);

	fracEast -= blockEast;
	fracNorth -= blockNorth;

	switch (filtering) {
		case GC_BILINEAR_FILTERING: {

			// These are the scalars for each texel (ul for texelUL, ur for texelUR, etc.)

			sw = (1.0f - fracEast) * (1.0f - fracNorth);
			nw = (1.0f - fracEast) * fracNorth;
			se = fracEast * (1.0f - fracNorth);
			ne = fracEast * fracNorth;

			swElevationIndex = gcGetElevationIndex (mapInfo, blockEast, blockNorth);

			if (blockNorth < (mapInfo->samples[GC_NORTH] - 1))
			{
				nwElevationIndex = gcGetElevationIndex (mapInfo, blockEast, blockNorth + 1);
			}
			else
			{
				nwElevationIndex = swElevationIndex;
			}
			if (blockEast < (mapInfo->samples[GC_EAST] - 1))
			{
				seElevationIndex = gcGetElevationIndex (mapInfo, blockEast + 1, blockNorth);
			}
			else
			{
				seElevationIndex = swElevationIndex;
			}
			if ((blockNorth < (mapInfo->samples[GC_NORTH] - 1))&&(blockEast < (mapInfo->samples[GC_EAST] - 1)))
			{
				neElevationIndex = gcGetElevationIndex (mapInfo, blockEast + 1, blockNorth + 1);
			}
			else
			{
				neElevationIndex = swElevationIndex;
			}
			value = (privateData->elevation[swElevationIndex] * sw
				+ privateData->elevation[seElevationIndex] * se
				+ privateData->elevation[nwElevationIndex] * nw
				+ privateData->elevation[neElevationIndex] * ne);

			break;
		}
		case GC_POINT_FILTERING:
		default: {
			value = gcGetElevationByCoordinates(mapInfo, blockEast, blockNorth);
			break;
			}
	}
	return value;
}

void toLowerN(char *s, int n)
/* Convert a section of memory to upper case. */
{
	while (--n >= 0) {
		*s = tolower(*s);
        s++;
    };
}

void gcReadDEMBRecord(FILE *stream, USGSDEMBRecord *bRecord) {
	//read each B record
	bRecord->rowId = GetUSGSInt(stream);
	bRecord->colId = GetUSGSInt(stream);
	bRecord->runLength = GetUSGSInt(stream);
	bRecord->alwaysOne = GetUSGSInt(stream);

	bRecord->gpCoordinateX = GetUSGSDouble(stream);
	bRecord->gpCoordinateY = GetUSGSDouble(stream);

	bRecord->localElevation = GetUSGSDouble(stream);
	bRecord->minElevation = GetUSGSDouble(stream);
	bRecord->maxElevation = GetUSGSDouble(stream);
}

void gcFillSizeFromArcSeconds(const gcPrivateData* privateData, const float latitude, const float *arcSeconds, float *cellSize) {
	//west to east changes size/arcsecond as we move from equator to pole
	cellSize[GC_EAST] = arcSeconds[GC_EAST] * privateData->metersPerArcSecondEquator * cos(latitude / 360.0f * 2.0f * M_PI);
	//south to north stays constant/arcsecond from equator to pole
	cellSize[GC_NORTH] = arcSeconds[GC_NORTH] * privateData->metersPerArcSecondPolar;
}


//ArcInfo ASCII
int gcReadArcInfoASCIIMeta(FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;

	int retval = GC_OK;

	char* lastspace = NULL;
	char charline[1024]; 
	int i;
	float cellSize = 1;

	privateData->lastHeaderASCII = 0;

	for (i = 0; i < 6; i++)
	{
		privateData->lastHeaderASCII++;
		fgets(charline, 1024, stream);		
		lastspace = strrchr(charline, ' '); 
		ToUpper(charline);
		if (strstr(charline, "NROWS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_NORTH] );
			privateData->lastHeaderASCII = i;
		}
		else if (strstr(charline, "NCOLS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_EAST] );
			privateData->lastHeaderASCII = i;
		}
		else if (strstr(charline, "XLLCORNER"))
		{
			mapInfo->hasLLCorner = GC_TRUE;
			mapInfo->llUnit = GC_LL_UNIT_METERS;
			sscanf(lastspace, "%f", &mapInfo->llCorner[GC_EAST]);
			privateData->lastHeaderASCII = i;
		}
		else if (strstr(charline, "YLLCORNER"))
		{
			mapInfo->hasLLCorner = GC_TRUE;
			mapInfo->llUnit = GC_LL_UNIT_METERS;
			sscanf(lastspace, "%f", &mapInfo->llCorner[GC_NORTH]);
			privateData->lastHeaderASCII = i;
		}
		else if (strstr(charline, "CELLSIZE"))
		{
			//assumed to be meters
			sscanf( lastspace, "%f", &cellSize );
			mapInfo->hasRealSize = GC_TRUE;
			privateData->lastHeaderASCII = i;
		}
		else if (strstr(charline, "NODATA_VALUE"))
		{
			sscanf( lastspace, "%i", &privateData->noData.intValue );
			privateData->lastHeaderASCII = i;
		}
		else
		{
			//doesn't look like an arcinfo ascii
			retval = GC_ERROR;
			break;
		}
	}//end token loop
	if (mapInfo->hasRealSize == GC_TRUE) {
		mapInfo->size[GC_EAST] = cellSize * (mapInfo->samples[GC_EAST] - 1);
		mapInfo->size[GC_NORTH] = cellSize * (mapInfo->samples[GC_NORTH] - 1);
	}
	return retval;
}

//Vista Pro DEM file
int gcReadVistaProDEMMeta(FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;
	//const int VISTA_PRO_CELL_SIZE = 75;

	fseek(stream, 138, SEEK_SET);
	mapInfo->samples[GC_EAST] = GetUnsignedShortBigEndian(stream);
	fseek(stream, 142, SEEK_SET);
	mapInfo->samples[GC_NORTH] =  GetUnsignedShortBigEndian(stream);
	privateData->dataStart = 2048;
	//todo: found mention that scale is in header, but unable to find specifics.
	//mapInfo->hasRealSize = GC_TRUE;
	//mapInfo->size[GC_EAST] = VISTA_PRO_CELL_SIZE * (mapInfo->samples[GC_EAST] - 1);
	//mapInfo->size[GC_NORTH] = VISTA_PRO_CELL_SIZE * (mapInfo->samples[GC_NORTH] - 1);

	return retval;
}

//world machine
int gcReadWorldMachineMeta(const char *filename, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;
	unsigned long lStreamSize = FileSize(filename);

	privateData->elevationUnit = GC_UNIT_UNKNOWN;

	lStreamSize /= 4;
	mapInfo->samples[GC_EAST] = mapInfo->samples[GC_NORTH] = sqrt(lStreamSize);

	return retval;
}

//PGM
int gcReadPGMMeta(FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	char magicNumber;
	fseek(stream, 1, SEEK_SET);
	magicNumber = GetTiny(stream);
	mapInfo->samples[GC_EAST] = getPGMInt(stream);
	mapInfo->samples[GC_NORTH] = getPGMInt(stream);
	privateData->dataStart = ftell(stream);

	return retval;
}

//Classic USGS DEM
int gcReadUSGSDEMMeta(FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	//USGS DEM
	unsigned long east;
	unsigned long DEMScale;
	unsigned long i;
	unsigned long groundUnits;
	float longitude, latitude;
	
	unsigned long progressChunkSize = 1;
	boolean bMinMaxInitialized = FALSE;

	//longitude, latitude
	fseek(stream, 110, SEEK_SET);
	longitude = GetUSGSReal(stream);
	latitude = GetUSGSReal(stream);

	//height and width
	fseek(stream, 853, SEEK_SET);
	mapInfo->samples[GC_NORTH] = GetUSGSInt(stream);
	mapInfo->samples[GC_EAST] = GetUSGSInt(stream);

	//get the resolution
	fseek(stream, 816, SEEK_SET);
	privateData->demResolution[0] = GetUSGSRealN(stream, 12);
	privateData->demResolution[1] = GetUSGSRealN(stream, 12);
	privateData->demResolution[2] = GetUSGSRealN(stream, 12);

	fseek(stream, 529, SEEK_SET);
	groundUnits = GetUSGSInt(stream);

	fseek(stream, 535, SEEK_SET);
	DEMScale = GetUSGSInt(stream);
	if (DEMScale == 1) //FEET
	{
		privateData->elevationScale = FEET_TO_METERS * privateData->demResolution[2];
		privateData->elevationUnit = GC_UNIT_FEET;
	}
	else if (DEMScale == 2) //METERS
	{
		privateData->elevationScale = 1.0f * privateData->demResolution[2];
	}

	//too many DEMs do this incorrectly so spin through
	//the rows and count the run length myself
	privateData->demRunInfo = calloc(mapInfo->samples[GC_EAST], sizeof(USGSDEMRunInfo));

	if ( !privateData->demRunInfo ) {
		retval = GC_ERROR;
		goto Finish;
	}			

	mapInfo->samples[GC_NORTH] = 1;

	privateData->demLowestCoordinateY = 0;

	//go to the first b record
	fseek(stream, 1024, SEEK_SET);

	progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_EAST] / 100.0f));
	//examine b records and find the minimum start and run length
	for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
	{
		USGSDEMBRecord bRecord;

		if (east % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)east * 0.5f / (float)mapInfo->samples[GC_EAST];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}

		gcReadDEMBRecord(stream, &bRecord);

		privateData->demRunInfo[east].runLength = bRecord.runLength;
		privateData->demRunInfo[east].gpCoordinateY = bRecord.gpCoordinateY;

		if (east == 0 || privateData->demRunInfo[east].gpCoordinateY < privateData->demLowestCoordinateY) {
			privateData->demLowestCoordinateY = privateData->demRunInfo[east].gpCoordinateY;
		}

		for (i = 0; i < privateData->demRunInfo[east].runLength; i++) {
			float elevation = GetUSGSReal(stream);
			if ((elevation != -32766)&&(elevation != -32767))
			{
				//we have an elevation
				if (bMinMaxInitialized)
				{
					if (elevation < privateData->demMinElevation)
						privateData->demMinElevation = elevation;
					if (elevation > privateData->demMaxElevation)
						privateData->demMaxElevation = elevation;
				}
				else
				{
					privateData->demMinElevation = elevation;
					privateData->demMaxElevation = elevation;
					bMinMaxInitialized = TRUE;
				}
			}
		}
	}

	//spin through and find our max north
	for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
	{
		unsigned long northstart = ceil((privateData->demRunInfo[east].gpCoordinateY - privateData->demLowestCoordinateY) / privateData->demResolution[1]);
		unsigned long northend = northstart + privateData->demRunInfo[east].runLength;

		if (northend > mapInfo->samples[GC_NORTH])
		{
			mapInfo->samples[GC_NORTH] = northend;
		}
	}

	privateData->dataStart = 1024;

	switch (groundUnits) {
		case 0: { //radians
			mapInfo->hasRealElevation = GC_TRUE;
			break;
			}
		case 1: { //feet
			mapInfo->size[GC_EAST] = FEET_TO_METERS * privateData->demResolution[0] * (mapInfo->samples[GC_EAST] - 1);
			mapInfo->size[GC_NORTH] = FEET_TO_METERS * privateData->demResolution[1] * (mapInfo->samples[GC_NORTH] - 1);
			mapInfo->hasRealSize = GC_TRUE;
			break;
			}
		case 2: { //meters
			mapInfo->size[GC_EAST] = privateData->demResolution[0] * (mapInfo->samples[GC_EAST] - 1);
			mapInfo->size[GC_NORTH] = privateData->demResolution[1] * (mapInfo->samples[GC_NORTH] - 1);
			mapInfo->hasRealSize = GC_TRUE;
			break;
			}
		case 3: { //arc seconds
			float arcSeconds[2];
			float cellSize[2];

			arcSeconds[0] = privateData->demResolution[0];
			arcSeconds[1] = privateData->demResolution[1];

			gcFillSizeFromArcSeconds(privateData, latitude, arcSeconds, cellSize);

			mapInfo->size[GC_EAST] = cellSize[GC_EAST] * (mapInfo->samples[GC_EAST] - 1);
			mapInfo->size[GC_NORTH] = cellSize[GC_NORTH] * (mapInfo->samples[GC_NORTH] - 1);
			mapInfo->hasRealSize = GC_TRUE;
			break;
			}
	};

Finish:
		return retval;
}


//Terragen
int gcReadTerragenMeta(FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	unsigned long markerpos = 16;
	long marker;
	unsigned short size = 0, xpts = 0, ypts = 0;
	fseek(stream, markerpos, SEEK_SET);
	marker = GetIntBigEndian(stream);

	while (marker != 'ALTW')//altitude
	{
		switch (marker)
		{
		case 'SIZE'://size
			{
				size = GetUnsignedShortLittleEndian(stream);
				break;
			}
		case 'XPTS':// X points
			{
				xpts = GetUnsignedShortLittleEndian(stream);
				break;
			}
		case 'YPTS':// Y points
			{
				ypts = GetUnsignedShortLittleEndian(stream);
				break;
			}
		case 'SCAL':// scaling
			{
				mapInfo->size[GC_EAST] = GetRealLittleEndian(stream);
				mapInfo->size[GC_NORTH] = GetRealLittleEndian(stream);
				mapInfo->hasRealSize = GC_TRUE;
				privateData->elevationScale = GetRealLittleEndian(stream);
				break;
			}

		}
		markerpos += 8;
		fseek(stream, markerpos, SEEK_SET);
		marker = GetIntBigEndian(stream);
	}
	privateData->dataStart = markerpos + 4;

	if (xpts == 0)
	{
		mapInfo->samples[GC_EAST] = size + 1;
		mapInfo->samples[GC_NORTH] = size + 1;
	}
	else
	{
		mapInfo->samples[GC_EAST] = xpts;
		mapInfo->samples[GC_NORTH] = ypts;
	}
	mapInfo->size[GC_EAST] *= mapInfo->samples[GC_EAST] - 1;
	mapInfo->size[GC_NORTH] *= mapInfo->samples[GC_NORTH] - 1;

	return retval;
}

//HGT
int gcReadHGTMeta(const char *filename, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	unsigned long lStreamSize = FileSize(filename);
	float arcSeconds[2];
	float cellSize[2];
	int latLocation = strlen(filename) - 10;
	int latitude = 0;

	lStreamSize /= 2;
	mapInfo->samples[GC_EAST] = mapInfo->samples[GC_NORTH] = sqrt(lStreamSize);
	if (mapInfo->samples[GC_EAST]== 1201) {
		arcSeconds[0] = arcSeconds[1] = 3;
	}
	else {
		arcSeconds[0] = arcSeconds[1] = 1;
	}

	privateData->byteOrder = boBigEndian;
	privateData->numBits = 16;
	privateData->noData.intValue = -32768;

	if (isdigit(*(filename + latLocation)) && isdigit(*(filename + latLocation + 1))) {
		mapInfo->hasRealSize = GC_TRUE;
		sscanf(filename + latLocation, "%2i", &latitude);

		gcFillSizeFromArcSeconds(privateData, latitude, arcSeconds, cellSize);

		mapInfo->size[GC_EAST] = cellSize[GC_EAST] * (mapInfo->samples[GC_EAST] - 1);
		mapInfo->size[GC_NORTH] = cellSize[GC_NORTH] * (mapInfo->samples[GC_NORTH] - 1);
	}
	else {
		mapInfo->hasRealElevation = GC_TRUE;
	}


	return retval;
}

//Gridfloat
int gcGridFloatMeta(const char *filename, FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	float arcSeconds[2];
	float latitude = 0;
	char *hdrFilename = NULL;
	FILE *hdrStream = NULL;
	size_t filenamelength = strlen(filename);
	hdrFilename = calloc(filenamelength + 1, sizeof(char));

	if (!hdrFilename)
	{
		retval = GC_ERROR;
		goto Finish;
	}
	strncpy(hdrFilename, filename, filenamelength  - 4);
	strcat(hdrFilename, ".hdr");

	hdrStream = fopen( hdrFilename, "r" );
	if ( !hdrStream ) {
		retval = GC_ERROR;
		goto Finish;
	}
	while (!feof(hdrStream))
	{

		char charline[1024]; 
		char* lastspace = NULL;
		fgets(charline, 1024, hdrStream);

		lastspace = strrchr(charline, ' '); 
		ToUpper(charline);
		if (strstr(charline, "BYTEORDER"))
		{
			if (strstr(charline, "LSBFIRST") != NULL)
			{
				privateData->byteOrder = boLittleEndian;
			}
			else
			{
				privateData->byteOrder = boBigEndian;
			}
		}
		else if (strstr(charline, "NROWS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_NORTH] );
		}
		else if (strstr(charline, "NCOLS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_EAST] );
		}
		else if (strstr(charline, "NODATA_VALUE"))
		{
			sscanf( lastspace, "%i", &privateData->noData.intValue );
		}
		else if (strstr(charline, "CELLSIZE"))
		{
			//assumed to be degrees
			sscanf( lastspace, "%f", &arcSeconds[GC_EAST] );
			mapInfo->hasRealSize = GC_TRUE;
		}
		else if (strstr(charline, "YLLCORNER"))
		{
			mapInfo->hasLLCorner = GC_TRUE;
			mapInfo->llUnit = GC_LL_UNIT_LATLONG;
			sscanf( lastspace, "%f", &mapInfo->llCorner[GC_NORTH]);
			latitude = mapInfo->llCorner[GC_NORTH];
		}
		else if (strstr(charline, "XLLCORNER"))
		{
			mapInfo->hasLLCorner = GC_TRUE;
			mapInfo->llUnit = GC_LL_UNIT_LATLONG;
			sscanf(lastspace, "%f", &mapInfo->llCorner[GC_EAST]);
		}
	}
	if (mapInfo->hasRealSize == GC_TRUE) {
		float cellSize[2];
		arcSeconds[GC_EAST] = arcSeconds[GC_EAST] * 3600;
		arcSeconds[GC_NORTH] = arcSeconds[GC_EAST];

		gcFillSizeFromArcSeconds(privateData, latitude, arcSeconds, cellSize);

		mapInfo->size[GC_EAST] = cellSize[GC_EAST] * (mapInfo->samples[GC_EAST] - 1);
		mapInfo->size[GC_NORTH] = cellSize[GC_NORTH] * (mapInfo->samples[GC_NORTH] - 1);
	}

Finish:
	if (hdrStream) fclose (hdrStream);
	if (hdrFilename) free(hdrFilename);
	return retval;
}

//BIL
int gcReadBILMeta(const char *filename, FILE *stream, gcMapInfo *mapInfo) {
	char *hdrFilename = NULL;
	FILE *hdrStream = NULL;
	char *prjFilename = NULL;
	FILE *prjStream = NULL;
	size_t filenamelength = strlen(filename);
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	float eastDimension = 0, northDimension = 0, latitude = 0;
	float eastUnit = 0, northUnit = 0, latitudeUnit = 0;
	hdrFilename = calloc(filenamelength + 1, sizeof(char));
	
	if (!hdrFilename)
	{
		retval = GC_ERROR;
		goto Finish;
	}

	strncpy(hdrFilename, filename, filenamelength  - 4);
	strcat(hdrFilename, ".hdr");

	hdrStream = fopen( hdrFilename, "r" );
	if ( !hdrStream ) {
		retval = GC_ERROR;
		goto Finish;
	}
	while (!feof(hdrStream))
	{

		char charline[1024]; 
		char* lastspace = NULL;
		fgets(charline, 1024, hdrStream);

		lastspace = strrchr(charline, ' '); 
		ToUpper(charline);
		if (strstr(charline, "BYTEORDER"))
		{
			if (strrchr(charline, 'M'))
			{
				privateData->byteOrder = boBigEndian;
			}
			else
			{
				privateData->byteOrder = boLittleEndian;
			}
		}
		else if (strstr(charline, "NROWS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_NORTH] );
		}
		else if (strstr(charline, "NCOLS"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_EAST] );
		}
		else if (strstr(charline, "NBITS"))
		{
			sscanf( lastspace, "%u", &privateData->numBits );
		}
		else if (strstr(charline, "MAPUNITS"))
		{
			if (strstr(charline, "FEET"))
			{
				privateData->elevationScale = FEET_TO_METERS;
				privateData->elevationUnit = GC_UNIT_FEET;
			}
			else if (strstr(charline, "METERS"))
			{
				privateData->elevationScale = 1.0f;
			}
		}
		else if (strstr(charline, "NODATA"))
		{
			sscanf( lastspace, "%i", &privateData->noData.intValue );
		}
		else if (strstr(charline, "ULYMAP"))
		{
			sscanf( charline + 6, "%f", &latitude);
			if (strstr(lastspace, "M")) {
				latitudeUnit = 1;
			}
			else if (strstr(lastspace, "FT")) {
				latitudeUnit = 0.3048f;
			}
			else {
				//assume degrees, figure out with latitude
			}
		}
		else if (strstr(charline, "XDIM"))
		{
			sscanf( charline + 4, "%f", &eastDimension);
			if (strstr(lastspace, "M")) {
				eastUnit = 1;
			}
			else if (strstr(lastspace, "FT")) {
				eastUnit = 0.3048f;
			}
			else {
				//assume degrees, figure out with latitude
			}
		}
		else if (strstr(charline, "YDIM"))
		{
			sscanf( charline + 4, "%f", &northDimension);
			if (strstr(lastspace, "M")) {
				northUnit = 1;
			}
			else if (strstr(lastspace, "FT")) {
				northUnit = 0.3048f;
			}
			else {
				//assume degrees, figure out with latitude
			}
		}
	}

	//optionally, look for more meta data in prj file
	prjFilename = calloc(filenamelength + 1, sizeof(char));
	
	if (!prjFilename)
	{
		retval = GC_ERROR;
		goto Finish;
	}

	strncpy(prjFilename, filename, filenamelength  - 4);
	strcat(prjFilename, ".prj");

	prjStream = fopen( prjFilename, "r" );
	if ( prjStream) { 
		while (!feof(prjStream))
		{

			char charline[1024]; 
			char* lastspace = NULL;
			fgets(charline, 1024, prjStream);

			lastspace = strrchr(charline, ' '); 
			if (strstr(charline, "Zunits"))
			{
				if (strstr(charline, "FEET"))
				{
					privateData->elevationScale = FEET_TO_METERS;
					privateData->elevationUnit = GC_UNIT_FEET;
				}
				else if (strstr(charline, "METERS"))
				{
					privateData->elevationScale = 1.0f;
				}
				else if (strstr(charline, "DECIMETERS"))
				{
					privateData->elevationScale = 0.1f;
					privateData->elevationUnit = GC_UNIT_DECIMETERS;
				}
			}
		}
	} //end prj file 

	//does make the assumption that unitls will be the same in both directions
	if (eastDimension != 0 && northDimension != 0) {
		float arcSeconds[2];
		float cellSize[2];

		mapInfo->hasRealSize = GC_TRUE;
		if (eastUnit == 0) {
			//convert from degrees to arcseconds
			arcSeconds[GC_EAST] = eastDimension * 3600;
			arcSeconds[GC_NORTH] = northDimension * 3600;

			if (latitudeUnit != 0) {
				//rotation given in linear unit, figure out latitude from there
				latitude = (latitude * latitudeUnit) / 10002000; 
			}

			gcFillSizeFromArcSeconds(privateData, latitude, arcSeconds, cellSize);
		}
		else {
			cellSize[GC_EAST] = eastDimension * eastUnit;
			cellSize[GC_NORTH] = northDimension * northUnit;
		}

		mapInfo->size[GC_EAST] = cellSize[GC_EAST] * (mapInfo->samples[GC_EAST] - 1);
		mapInfo->size[GC_NORTH] = cellSize[GC_NORTH] * (mapInfo->samples[GC_NORTH] - 1);
	}

Finish:

	if (hdrStream) fclose (hdrStream);
	if (hdrFilename) free(hdrFilename);
	if (prjStream) fclose (prjStream);
	if (prjFilename) free(prjFilename);
	return retval;
}

//PDS
int gcReadPlanetaryDataSystemMeta(const char *filename, FILE *stream, gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_OK;

	char* lastspace = NULL;
	char charline[1024]; 
	unsigned int recordBytes = 0, recordLocation = 1;
	float mapScale;

	fgets(charline, 1024, stream);		
	ToUpper(charline);
	if (!strstr(charline, "PDS_VERSION_ID")) 
	{
		retval = GC_ERROR;
		goto Finish;
	}

	privateData->baseHeight = 0;
	privateData->scaleFactor = 1.0f;

	while (!feof(stream))
	{
		fgets(charline, 1024, stream);		
		lastspace = strrchr(charline, ' '); 
		ToUpper(charline);
		if (strstr(charline, "/*")) {
			continue;
		}
		else if (strstr(charline, "LINES"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_NORTH] );
		}
		else if (strstr(charline, "LINE_SAMPLES"))
		{
			sscanf( lastspace, "%u", &mapInfo->samples[GC_EAST] );
		}
		else if (strstr(charline, "SAMPLE_BITS"))
		{
			sscanf( lastspace, "%u", &privateData->numBits );
		}
		else if (strstr(charline, "RECORD_BYTES"))
		{
			sscanf( lastspace, "%u", &recordBytes );
		}
		else if (strstr(charline, "^IMAGE"))
		{
			sscanf( lastspace, "%u", &recordLocation);
		}
		else if (strstr(charline, "SCALING_FACTOR"))
		{
			sscanf( lastspace, "%f", &privateData->scaleFactor);
		}
		else if (strstr(charline, "OFFSET"))
		{
			sscanf( lastspace, "%f", &privateData->baseHeight);
		}
		else if (strstr(charline, "MISSING_CONSTANT"))
		{
			sscanf( lastspace, " 16#%8x#", &privateData->noData.uintValue);
		}

		else if (strstr(charline, "SAMPLE_TYPE"))
		{
			if (strstr(charline, "PC_REAL"))
			{
				privateData->byteOrder = boLittleEndian;
			}
			else
			{
				privateData->byteOrder = boBigEndian;
			}
		}
		else if (strstr(charline, "MAP_SCALE"))
		{
			char* equals = strrchr(charline, '=');
			sscanf( equals, "=%f", &mapScale );
			if (strstr(charline, "<METERS/PIXEL>"))
			{
				//mapScale is already in meters
				//mapScale *= 1.0f;
			}
			mapInfo->hasRealSize = GC_TRUE;
		}				
		else if (strstr(charline, "END") && !strstr(charline, "END_OBJECT = ")) 
		{
			break;
		}
	}	
	if (mapInfo->hasRealSize == GC_TRUE) {
		mapInfo->size[GC_EAST] = mapScale * (mapInfo->samples[GC_EAST] - 1);
		mapInfo->size[GC_NORTH] = mapScale * (mapInfo->samples[GC_NORTH] - 1);
	}

	privateData->dataStart = recordBytes * (recordLocation - 1);

Finish:
	return retval;
}

unsigned long getFileFormatFromExtension(const char *filename) {
	unsigned long fileFormat = gcUnknown;

	size_t filenamelength = strlen(filename);
	char extension[3];

	/* look at the file extension and see what this may be */
	strncpy(extension, &filename[filenamelength - 3], 3);
	toLowerN(extension, 3);
	if (strncmp(extension, "dem", 3) == 0) {
		fileFormat = gcUSGS;
	}
	else if (strncmp(extension, "r32", 3) == 0) {
		fileFormat = gcWorldMachine;
	}
	else if (strncmp(extension, "ter", 3) == 0) {
		fileFormat = gcTerragen;
	}
	else if (strncmp(extension, "bil", 3) == 0) {
		fileFormat = gcBIL;
	}
	else if (strncmp(extension, "hgt", 3) == 0) {
		fileFormat = gcHGT;
	}
	else if (strncmp(extension, "pgm", 3) == 0) {
		fileFormat = gcPGM;
	}
	else if (strncmp(extension, "asc", 3) == 0) {
		fileFormat = gcArcInfoASCII;
	}
	else if (strncmp(extension, "flt", 3) == 0) {
		fileFormat = gcGridFloat;
	}
	else if (strncmp(extension, "img", 3) == 0) {
		fileFormat = gcPlanetaryDataSystem;
	}

	return fileFormat;
}

int gcCanOpenFile(const char *filename, gcMapInfo *mapInfo)
{
	gcPrivateData* privateData = mapInfo->privateData;

	FILE *stream = NULL;
	int retval = GC_OK;

	privateData->fileFormat = getFileFormatFromExtension(filename);

	if (privateData->fileFormat == gcUnknown){	  
		return GC_ERROR;
	}

	/* attempt to open the file */
	stream = fopen( filename, "rb" );
	if ( !stream ) {
		return GC_ERROR;
	}

	if (privateData->fileFormat == gcUSGS)
	{
		char vistapro[41];
		int i;
		// dem extension, check to see if it's a vista pro dem 
		fread( vistapro, 1, 40, stream );
		vistapro[40] = 0;
		for (i = 39; i > 0;i--)
		{
			if (vistapro[i] != 32)
			{
				i=0;
			}
			else
			{
				vistapro[i] = 0;
			}
		}
		if ( strcmp( vistapro, "Vista DEM File") == 0) 
		{
			privateData->fileFormat = gcVistaPro;
		}
		else {
			mapInfo->description = calloc(41, sizeof(char));
			strncpy(mapInfo->description, vistapro, 41);
		}
	}

	privateData->flipDirection[0] = privateData->flipDirection[1] = FALSE;
	privateData->dataStart = 0;
	privateData->elevationScale = 1.0f; //if nothing specifies, assume meters
	privateData->elevationUnit = GC_UNIT_METERS;
	privateData->noData.intValue = -9999;
	mapInfo->hasRealSize = GC_FALSE;
	mapInfo->hasRealElevation = GC_FALSE;
	mapInfo->hasLLCorner = GC_FALSE;
	mapInfo->size[0] = mapInfo->size[1] = 20.0f;
	privateData->metersPerArcSecondPolar = EARTH_METERS_PER_ARC_SECOND_POLAR;
	privateData->metersPerArcSecondEquator = EARTH_METERS_PER_ARC_SECOND_EQUATOR;

	switch (privateData->fileFormat)
	{
	case gcVistaPro:
		{
			retval = gcReadVistaProDEMMeta(stream, mapInfo);
			break;
		}
	case gcUSGS:
		{
			retval = gcReadUSGSDEMMeta(stream, mapInfo);
			break;
		}
	case gcWorldMachine:
		{
			retval = gcReadWorldMachineMeta(filename, mapInfo);
			break;
		}
	case gcHGT:
		{
			retval = gcReadHGTMeta(filename, mapInfo);
			break;
		}
	case gcPlanetaryDataSystem: 
		{
			retval = gcReadPlanetaryDataSystemMeta(filename, stream, mapInfo);
			break;
		}
	case gcGridFloat:
		{
			retval = gcGridFloatMeta(filename, stream, mapInfo);
			break;
		}
	case gcBIL:
		{
			retval = gcReadBILMeta(filename, stream, mapInfo);
			break;
		}
	case gcTerragen:
		{
			retval = gcReadTerragenMeta(stream, mapInfo);
			break;
		}
	case gcPGM:
		{
			retval = gcReadPGMMeta(stream, mapInfo);
			break;
		}
	case gcArcInfoASCII:
		{
			retval = gcReadArcInfoASCIIMeta(stream, mapInfo);
			break;
		}	
	}//end format switch

	mapInfo->bufferSize = mapInfo->samples[GC_EAST] * mapInfo->samples[GC_NORTH];

	if ( stream ) fclose( stream );
	return retval;
}

void valueToBuffers(const unsigned long east, const unsigned long north, const gcMapInfo *mapInfo
					, double value, double minvalue, double maxvalue)
{
	gcPrivateData* privateData = mapInfo->privateData;

	unsigned long elevationIndex;
	double localMinValue = minvalue, localMaxValue = maxvalue;

	if (privateData->overrideElevationMinMax == GC_TRUE) {
		localMinValue = privateData->overrideMinElevation;
		localMaxValue = privateData->overrideMaxElevation;
	}

	elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

	((float *)privateData->elevation)[elevationIndex] = (value - localMinValue) / (localMaxValue - localMinValue);
}

void fillGapsAvgPerimeter(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long north, east;
	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			if (privateData->elevation[gcGetElevationIndex(mapInfo, east, north)] == FLT_MAX)
			{
				double perimeterSum = 0;
				unsigned long perimeterCount = 0;
				NoData originalValue, countedValue, averagedValue;
				originalValue.floatValue = FLT_MAX;
				countedValue.floatValue = FLT_MIN;
				gcPopulateAvgPerimeter(mapInfo, east, north, &perimeterSum, &perimeterCount, originalValue, countedValue);
				averagedValue.floatValue = perimeterSum / (double)(perimeterCount);
				gcFloodFill(mapInfo, east, north, countedValue, averagedValue);
				
			}
		}
	}
}

void fillGapsSpecifiedElevation(gcMapInfo *mapInfo, const double value) {
    gcPrivateData* privateData = mapInfo->privateData;
    double localElevation = (value - mapInfo->minElevation) / (mapInfo->maxElevation - mapInfo->minElevation);
    unsigned long north, east, elevationIndex;
    for (north = 0; north < mapInfo->samples[GC_NORTH]; north++) {
        for (east = 0; east < mapInfo->samples[GC_EAST]; east++) {
            elevationIndex = gcGetElevationIndex(mapInfo, east, north);
            if (privateData->elevation[elevationIndex] == FLT_MAX) {
                privateData->elevation[elevationIndex] = localElevation;

            }
        }
    }
}

void fillGapsMinElevation(gcMapInfo *mapInfo) {
    fillGapsSpecifiedElevation(mapInfo, mapInfo->minElevation);
}

void fillGapsMaxElevation(gcMapInfo *mapInfo) {
    fillGapsSpecifiedElevation(mapInfo, mapInfo->maxElevation);
}

void gcSetHasNoData(gcMapInfo *mapInfo, int hasNoData) {
	gcPrivateData* privateData = mapInfo->privateData;
	privateData->hasNoData = hasNoData;
}

void gcFillNoData(gcMapInfo *mapInfo, const unsigned long fillType, const double value) {
    gcPrivateData* privateData = mapInfo->privateData;
    if (privateData->hasNoData == TRUE) {
        switch (fillType) {
            case GC_NO_DATA_PERIMETER_AVERAGE:
                fillGapsAvgPerimeter(mapInfo);
                break;
            case GC_NO_DATA_SPECIFIED:
                fillGapsSpecifiedElevation(mapInfo, value);
                break;
            case GC_NO_DATA_MININUM_ELEVATION:
                fillGapsMinElevation(mapInfo);
                break;
            case GC_NO_DATA_MAXIMUM_ELEVATION:
               fillGapsMaxElevation(mapInfo);
               break;
            case GC_NO_DATA_TRANSPARENT:
                break;
            default:
                break;
                
        }
        
    }
}


void ReadWorldMachine(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long north, east, elevationIndex;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	//World Machine files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;
    
    //World Machine always 0 to 1
	mapInfo->minElevation = 0;
	mapInfo->maxElevation = 1.0f;

	//draw to our buffer
	fseek(stream, 0, SEEK_SET);

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
			privateData->elevation[elevationIndex] = GetRealLittleEndian(stream);
		}
	}

}




void ReadDEM(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long east, elevationIndex;
	unsigned long progressChunkSize = 1;

	progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_EAST] / 100.0f));
	//draw to our buffer
	for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
	{
		unsigned long north;
		unsigned long northstart = ceil((privateData->demRunInfo[east].gpCoordinateY - privateData->demLowestCoordinateY) / privateData->demResolution[1]);
		unsigned long northend = northstart + privateData->demRunInfo[east].runLength;

		USGSDEMBRecord bRecord;

		if (east % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  0.5f + (float)east / (float)mapInfo->samples[GC_EAST];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}

		gcReadDEMBRecord(stream, &bRecord);

		for (north = northstart; north < northend; north++)
		{
			float elevation = GetUSGSReal(stream);
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
			if ((elevation == -32766)||(elevation == -32767)) {
                privateData->elevation[elevationIndex] = FLT_MAX;
                privateData->hasNoData = TRUE;
			}
            else {
				privateData->elevation[elevationIndex] = elevation;
            }
		}
	}

	mapInfo->minElevation = privateData->demMinElevation;
	mapInfo->maxElevation = privateData->demMaxElevation;

}

void ReadVistaProDEM(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;

	//VistaPro files are laid out in rows west to east stacked south to north
	unsigned long north, east, elevationIndex;
	EndianShort lastValue;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	privateData->isWholeNumber = GC_TRUE;

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		unsigned short runLength;
		unsigned short uncompressedLength = 0;
		unsigned long compIndex = 0;
		unsigned long uncompIndex = 0;
		unsigned char currentRun = 0;
		unsigned char* compressed = NULL;
		unsigned char* uncompressed = NULL;
		unsigned long i;

		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}

		runLength = GetUnsignedShortBigEndian(stream);
		compressed = calloc(runLength, sizeof(unsigned char));
		fread(compressed, sizeof(unsigned char), runLength, stream);

		//figure out the total length of our uncompressed run
		do 
		{
			//decompress the stream
			currentRun = compressed[compIndex];
			if (currentRun >= 128)
			{
				currentRun = 257 - currentRun;
				compIndex++;
				uncompressedLength += currentRun;
				compIndex++;
			}
			else
			{//0-127 means take the next currentRun + 1 bytes as is
				currentRun++;
				compIndex++;
				uncompressedLength += currentRun;
				compIndex += currentRun;
			}
		}while (compIndex < runLength);

		compIndex = 0;
		uncompressed = calloc(uncompressedLength, sizeof(unsigned char));

		//actually uncompress our current run
		do 
		{
			//decompress the stream
			currentRun = compressed[compIndex];
			if (currentRun >= 128)
			{
				currentRun = 257 - currentRun;
				compIndex++;
				for(i = 0; i < currentRun; i++)
				{
					uncompressed[uncompIndex + i] = compressed[compIndex];
				}
				compIndex++;
				uncompIndex += currentRun;
			}
			else
			{//0-127 means take the next currentRun + 1 bytes as is
				currentRun++;
				compIndex++;
				for(i = 0; i < currentRun; i++)
				{
					uncompressed[uncompIndex + i] = compressed[compIndex + i];
				}
				compIndex += currentRun;
				uncompIndex += currentRun;
			}
		}while (compIndex < runLength);

		east = 0;
		uncompIndex = 0;

		//use the uncompressed line to build our values
		do
		{
			unsigned char currentDiff;
			if (east == 0)
			{
				currentDiff = 128;
			}
			else
			{
				currentDiff = uncompressed[uncompIndex];
				uncompIndex++;
			}
			if(currentDiff > 128)
			{
				lastValue.value -= (256 - currentDiff);
			}
			else if(currentDiff < 128)
			{
				lastValue.value += currentDiff;
			}
			else if (currentDiff == 128)
			{
				lastValue.elements[1] = uncompressed[uncompIndex];
				uncompIndex++;
				lastValue.elements[0] = uncompressed[uncompIndex];
				uncompIndex++;
			}
			if ((east == 0)&&(north == 0))
			{
				mapInfo->maxElevation = lastValue.value;
				mapInfo->minElevation = lastValue.value;
			}
			else if (lastValue.value < mapInfo->minElevation)
			{
				mapInfo->minElevation = lastValue.value;				
			}
			else if (lastValue.value > mapInfo->maxElevation)
			{
				mapInfo->maxElevation = lastValue.value;				
			}

			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
            privateData->elevation[elevationIndex] = lastValue.value;

			east++;
		} while (east < mapInfo->samples[GC_EAST]);
		if (compressed)
			free(compressed);
		if(uncompressed)
			free(uncompressed);

	}

}

void ReadTerragenDEM(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	float currentElevation;
	unsigned long north, east, elevationIndex;
	short heightScale, baseHeight;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	heightScale = GetShortLittleEndian(stream);
	baseHeight = GetShortLittleEndian(stream);

	//Terragen files are laid out in rows west to east stacked south to north
	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

			currentElevation = baseHeight + (float)(GetShortLittleEndian(stream) * heightScale) / 65536.0f;
			privateData->elevation[elevationIndex] = currentElevation;

			if ((east == 0)&&(north == 0))
			{
				mapInfo->maxElevation = currentElevation;
				mapInfo->minElevation = currentElevation;
			}
			else if (currentElevation < mapInfo->minElevation)
			{
				mapInfo->minElevation = currentElevation;				
			}
			else if (currentElevation > mapInfo->maxElevation)
			{
				mapInfo->maxElevation = currentElevation;				
			}
		}
	}
}

void readPGM(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;

	unsigned long north, east, elevationIndex;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	privateData->isWholeNumber = GC_TRUE;
	//PGM files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;

	mapInfo->minElevation = 0; 
	mapInfo->maxElevation = getPGMInt(stream);

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
			privateData->elevation[elevationIndex] = getPGMInt(stream);
		}
	}
}

void ReadHGT(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;

	unsigned long north, east, elevationIndex;
	int currentElevation;

	boolean minMaxSet = FALSE;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	privateData->isWholeNumber = GC_TRUE;
	//HGT and BIL files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;

	fseek(stream, 0, SEEK_SET);

	//first pass find high and low
	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

			switch (privateData->numBits)
			{
			case 8:
				currentElevation = GetUnsignedTiny(stream);
				break;
			case 16:
				if (privateData->byteOrder == boBigEndian)
				{	
					currentElevation = GetShortBigEndian(stream);
				}
				else
				{
					currentElevation = GetShortLittleEndian(stream);
				}
				break;
			case 32:
				if (privateData->byteOrder == boBigEndian)
				{	
					currentElevation = GetIntBigEndian(stream);
				}
				else
				{
					currentElevation = GetIntLittleEndian(stream);
				}
				break;
			}

			if (currentElevation != privateData->noData.intValue)
			{
				privateData->elevation[elevationIndex] = currentElevation;
				if (minMaxSet == FALSE)
				{	mapInfo->maxElevation = currentElevation;
					mapInfo->minElevation = currentElevation;
					minMaxSet = TRUE;
				}
				else if (currentElevation < mapInfo->minElevation)
				{
					mapInfo->minElevation = currentElevation;				
				}
				else if (currentElevation > mapInfo->maxElevation)
				{
					mapInfo->maxElevation = currentElevation;				
				}
			}
			else
			{
				privateData->elevation[elevationIndex] = FLT_MAX;
                privateData->hasNoData = TRUE;
			}
		}
	}
}

void readPlanetaryDataSystem(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long east, north, elevationIndex;	
	float currentElevation;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	boolean minMaxSet = FALSE;

	//PDS files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

			if (privateData->byteOrder == boBigEndian)
			{	
				currentElevation = GetRealBigEndian(stream);
			}
			else
			{
				currentElevation = GetRealLittleEndian(stream);
			}

			if (currentElevation != privateData->noData.floatValue)
			{
				privateData->elevation[elevationIndex] = privateData->baseHeight + privateData->scaleFactor * currentElevation;
				if (minMaxSet == FALSE)
				{
					mapInfo->maxElevation = currentElevation;
					mapInfo->minElevation = currentElevation;
					minMaxSet = TRUE;
				}
				else if (currentElevation < mapInfo->minElevation)
				{
					mapInfo->minElevation = currentElevation;				
				}
				else if (currentElevation > mapInfo->maxElevation)
				{
					mapInfo->maxElevation = currentElevation;				
				}
			}
			else
			{
				privateData->elevation[elevationIndex] = FLT_MAX;
                privateData->hasNoData = TRUE;
			}
		}
	}
}

void readArcInfoASCII(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long east, north, elevationIndex, i;	
	float currentElevation;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));


	boolean minMaxSet = FALSE;

	//ArcInfoASCII files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;

	//get to start of data
	for (i = 0;i < privateData->lastHeaderASCII + 1;i++)
	{
		char charline[1024]; 
		fgets(charline, 1024, stream);		
	}

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

			fscanf(stream, "%f", &currentElevation);


			if (currentElevation != privateData->noData.intValue)
			{
				privateData->elevation[elevationIndex] = currentElevation;
				if (minMaxSet == FALSE)
				{
					mapInfo->maxElevation = currentElevation;
					mapInfo->minElevation = currentElevation;
					minMaxSet = TRUE;
				}
				else if (currentElevation < mapInfo->minElevation)
				{
					mapInfo->minElevation = currentElevation;				
				}
				else if (currentElevation > mapInfo->maxElevation)
				{
					mapInfo->maxElevation = currentElevation;				
				}
			}
			else
			{
				privateData->elevation[elevationIndex] = FLT_MAX;
                privateData->hasNoData = TRUE;
			}
		}
	}
}

void readGridFloat(gcMapInfo *mapInfo, FILE* stream)
{
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long east, north, elevationIndex;	
	float currentElevation;

	boolean minMaxSet = FALSE;
	unsigned long progressChunkSize = (unsigned long)(ceil((float)mapInfo->samples[GC_NORTH] / 100.0f));

	//GridFloat files are laid out in rows west to east stacked north to south
	privateData->flipDirection[GC_NORTH] = TRUE;

	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		if (north % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)north / (float)mapInfo->samples[GC_NORTH];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

			if (privateData->byteOrder == boBigEndian)
			{	
				currentElevation = GetRealBigEndian(stream);
			}
			else
			{
				currentElevation = GetRealLittleEndian(stream);
			}

			if (currentElevation != privateData->noData.intValue)
			{
				privateData->elevation[elevationIndex] = currentElevation;
				if (minMaxSet == FALSE)
				{
					mapInfo->maxElevation = currentElevation;
					mapInfo->minElevation = currentElevation;
					minMaxSet = TRUE;
				}
				else if (currentElevation < mapInfo->minElevation)
				{
					mapInfo->minElevation = currentElevation;				
				}
				else if (currentElevation > mapInfo->maxElevation)
				{
					mapInfo->maxElevation = currentElevation;				
				}
			}
			else
			{
				privateData->elevation[elevationIndex] = FLT_MAX;
                privateData->hasNoData = TRUE;
			}
		}
	}
}

void gcNormalizeElevation(gcMapInfo *mapInfo) {
    gcPrivateData* privateData = mapInfo->privateData;
    unsigned long east, north, elevationIndex;
    for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
    {
        for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
        {
            elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
            //if it's a known value then normalize
            if (privateData->elevation[elevationIndex] != FLT_MAX) {
                valueToBuffers(east, north, mapInfo, privateData->elevation[elevationIndex], mapInfo->minElevation, mapInfo->maxElevation);
            }
        }
    }
}

void gcNormalizeAndClipElevation(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long east, north, elevationIndex;
	float elevation;
	for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
	{
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
		{
			elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);
			elevation = gcGetElevationByIndex(mapInfo, elevationIndex);

			//if it's a known value then normalize
			if (elevation != FLT_MAX) {
				if (elevation < mapInfo->minElevation) {
					privateData->elevation[elevationIndex] = 0;
				} else if (elevation > mapInfo->maxElevation) {
					privateData->elevation[elevationIndex] = 1.0f;
				}
				else {
					valueToBuffers(east, north, mapInfo, privateData->elevation[elevationIndex], mapInfo->minElevation, mapInfo->maxElevation);
				}
			}
		}
	}
}

void gcScaleElevation(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	if (privateData->elevationScale != 1.0f) {
		unsigned long east, north, elevationIndex;

		//once we scale it, everything is in meters
		privateData->elevationUnit = GC_UNIT_METERS;

		mapInfo->minElevation *= privateData->elevationScale;
		mapInfo->maxElevation *= privateData->elevationScale;

		for (north = 0; north < mapInfo->samples[GC_NORTH]; north++)
		{
			for (east = 0; east < mapInfo->samples[GC_EAST]; east++)
			{
				elevationIndex = gcGetFlippedElevationIndex(mapInfo, east, north);

				//if it's a known value then scale
				if (privateData->elevation[elevationIndex] != FLT_MAX) {
					privateData->elevation[elevationIndex] *= privateData->elevationScale;
				}
			}
		}
	}
}

int gcOpenFile(const char *filename, gcMapInfo *mapInfo) {

	gcPrivateData* privateData = mapInfo->privateData;
	FILE *stream = NULL;

	if (privateData->useExternalElevation == GC_FALSE) {
		privateData->elevation = calloc( mapInfo->bufferSize, sizeof( float ));
	}

	if ( !privateData->elevation ) {
		return GC_ERROR;
	}

	/* attempt to open the file */
	stream = fopen( filename, "rb" );
	if ( !stream ) {
		return GC_ERROR;
	}

	fseek(stream, privateData->dataStart, SEEK_SET);

	switch (privateData->fileFormat)
	{
	case gcVistaPro:
		{
			ReadVistaProDEM(mapInfo, stream);
			break;
		}
	case gcWorldMachine:
		{
			ReadWorldMachine(mapInfo, stream);
			break;
		}
	case gcUSGS:
		{
			ReadDEM(mapInfo, stream);
			break;
		}
	case gcHGT:case gcBIL:
		{
			ReadHGT(mapInfo, stream);
			break;
		}
	case gcTerragen:
		{
			ReadTerragenDEM(mapInfo, stream);
			break;
		}
	case gcPGM:
		{
			readPGM(mapInfo, stream);
			break;
		}
	case gcArcInfoASCII:
		{
			readArcInfoASCII(mapInfo, stream);
			break;
		}
	case gcGridFloat:
		{
			readGridFloat(mapInfo, stream);
			break;
		}
	case gcPlanetaryDataSystem:
		{
			readPlanetaryDataSystem(mapInfo, stream);
			break;
		}
	}
	if ( stream ) fclose( stream );



	return GC_OK;
}

int gcCreateMesh(gcMapInfo *mapInfo, const unsigned long targetPolyCount, const unsigned long meshType, float adaptiveError) {
	int retval = GC_ERROR;

	mapInfo->mesh = calloc(1, sizeof(gcMesh));
	if ( !mapInfo->mesh ) goto Finish;

	mapInfo->mesh->facetCount = 0;
	mapInfo->mesh->pointCount = 0;
	mapInfo->mesh->facets = NULL;
	mapInfo->mesh->facetEdges = NULL;
	mapInfo->mesh->edges = NULL;
	mapInfo->mesh->points = NULL;
	mapInfo->mesh->EPercent = NULL;
	mapInfo->mesh->NPercent = NULL;

	if (meshType != GC_MESH_ADAPTIVE) {
		retval = gcCreateGridMesh(mapInfo, targetPolyCount, meshType);
	}
	else {
		retval = gcCreateAdaptiveMesh(mapInfo, targetPolyCount, adaptiveError);
	}

Finish:
	return retval;
}

void gcRegisterProgressCallback(gcMapInfo *mapInfo, void (*progressCallback)(void *, float), void* callbackData) {
	gcPrivateData* privateData = mapInfo->privateData;
	privateData->progressCallback = progressCallback;
	privateData->callbackData = callbackData;
}

void gcDeregisterProgressCallback(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;
	privateData->progressCallback = NULL;
	privateData->callbackData = NULL;
}

int gcSetBufferSize(gcMapInfo *mapInfo, const unsigned long east, const unsigned long north) {
	gcPrivateData* privateData = mapInfo->privateData;

	mapInfo->samples[GC_EAST] = east;
	mapInfo->samples[GC_NORTH] = north;
	mapInfo->bufferSize = mapInfo->samples[GC_EAST] * mapInfo->samples[GC_NORTH];

	if (privateData->useExternalElevation == GC_FALSE) {
		privateData->elevation = calloc(mapInfo->bufferSize, sizeof(float));
	}

	if (!privateData->elevation) {
		return GC_ERROR;
	}

	return GC_OK;
}

int gcSetElevationByIndex(gcMapInfo *mapInfo, const unsigned long elevationIndex, const float elevation) {
	gcPrivateData* privateData = mapInfo->privateData;

	privateData->elevation[elevationIndex] = elevation;

	return GC_OK;
}



int WriteWorldMachine(gcMapInfo *mapInfo, FILE* stream) {
	gcPrivateData* privateData = mapInfo->privateData;
	long north, east, elevationIndex;
	float elevation;

	//World Machine files are laid out in rows west to east stacked north to south

	for (north = mapInfo->samples[GC_NORTH] - 1; north >= 0; north--) {

		for (east = 0; east < mapInfo->samples[GC_EAST]; east++) {
			elevationIndex = gcGetElevationIndex(mapInfo, east, north);
			elevation = gcGetElevationByIndex(mapInfo, elevationIndex);
			WriteRealLittleEndian(stream, elevation);
		}
	}

	return GC_OK;
}

int WriteArcInfoASCII(gcMapInfo *mapInfo, FILE* stream) {
	long north, east, elevationIndex;
	gcPrivateData* privateData = mapInfo->privateData;
	float elevation;

	fprintf(stream, "nrows %u\n", mapInfo->samples[GC_NORTH]);
	fprintf(stream, "ncols %u\n", mapInfo->samples[GC_EAST]);
	//TODO: XLLCORNER, YLLCORNER
	fprintf(stream, "xllcorner 0.000000\n");
	fprintf(stream, "yllcorner 0.000000\n");
	fprintf(stream, "cellsize 1.000000\n");
	fprintf(stream, "NODATA_value -9999\n");

	//HGT and BIL files are laid out in rows west to east stacked north to south

	for (north = mapInfo->samples[GC_NORTH] - 1; north >= 0; north--) {
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++) {
			elevationIndex = gcGetElevationIndex(mapInfo, east, north);
			elevation = gcGetElevationByIndex(mapInfo, elevationIndex);
			if (elevation == FLT_MAX) {
				elevation = privateData->noData.intValue;
			}
			fprintf(stream, "%f ", elevation);
		}
		fprintf(stream, "\n");
	}


	return GC_OK;
}

int WriteHGT(gcMapInfo *mapInfo, FILE* stream) {
	gcPrivateData* privateData = mapInfo->privateData;
	long north, east, elevationIndex;
	float elevation;

	//HGT and BIL files are laid out in rows west to east stacked north to south

	for (north = mapInfo->samples[GC_NORTH] - 1; north >= 0; north--) {
		for (east = 0; east < mapInfo->samples[GC_EAST]; east++) {
			elevationIndex = gcGetElevationIndex(mapInfo, east, north);
			elevation = gcGetElevationByIndex(mapInfo, elevationIndex);
			if (elevation == FLT_MAX) {
				elevation = privateData->noData.intValue;
			}
			switch (privateData->numBits)
			{
			case 8:
				WriteUnsignedTiny(stream, elevation);
				break;
			case 16: {
				short localElevation = elevation;
				if (privateData->byteOrder == boBigEndian)
				{
					WriteShortBigEndian(stream, localElevation);
				}
				else
				{
					WriteShortLittleEndian(stream, localElevation);
				}
				break;
			}

			case 32:
				if (privateData->byteOrder == boBigEndian)
				{
					WriteIntBigEndian(stream, elevation);
				}
				else
				{
					WriteIntLittleEndian(stream, elevation);
				}
				break;
			}		
		}
	}

	return GC_OK;
}

void gcScaleToTargetElevation(gcMapInfo *mapInfo,
	int elevationUnit, float minElevation, float maxElevation,
	int targetElevationUnit, float minTargetElevation, float maxTargetElevation,
	float minFormatElevation, float maxFormatElevation) {

	//if there's nothing to transform, just leave
	if (minElevation == minTargetElevation && maxElevation == maxTargetElevation
		&& minElevation >= minFormatElevation && maxElevation <= maxFormatElevation) {
	}
	else {
		gcPrivateData* privateData = mapInfo->privateData;
		float elevation;

		float currentRange = maxElevation - minElevation;
		float targetRange = maxTargetElevation - minTargetElevation;
		unsigned long elevationIndex;

		for (elevationIndex = 0; elevationIndex < mapInfo->bufferSize; elevationIndex++) {
			if (privateData->elevation[elevationIndex] != FLT_MAX) {
				elevation = (privateData->elevation[elevationIndex] - minElevation) / currentRange;
				elevation = minTargetElevation + elevation * targetRange;
				privateData->elevation[elevationIndex] = elevation;
				//clip to what our format allows
				if (privateData->elevation[elevationIndex] < minFormatElevation) {
					privateData->elevation[elevationIndex] = minFormatElevation;
				}
				else if (privateData->elevation[elevationIndex] > maxFormatElevation) {
					privateData->elevation[elevationIndex] = maxFormatElevation;
				}
			}
		}
	}
}

int gcWriteFile(const char *filename, const char *finalFilename, gcMapInfo *mapInfo, 
	int elevationUnit, float minElevation, float maxElevation,
	int targetElevationUnit, float minTargetElevation, float maxTargetElevation) {
	int retval = GC_ERROR;

	FILE *stream = NULL;

	gcPrivateData* privateData = mapInfo->privateData;
	privateData->fileFormat = getFileFormatFromExtension(finalFilename);

	if (privateData->fileFormat == gcUnknown) {
		return GC_ERROR;
	}

	stream = fopen(filename, "wb");

	switch (privateData->fileFormat) {
		case gcVistaPro: {
			break;
		}
		case gcUSGS: {
			break;
		}
		case gcWorldMachine: {
			gcScaleToTargetElevation(mapInfo, elevationUnit, minElevation, maxElevation,
				GC_UNIT_UNKNOWN, 0, 1.0f, 0, 1.0f);
			retval = WriteWorldMachine(mapInfo, stream);
			break;
		}
		case gcHGT: {
			gcScaleToTargetElevation(mapInfo, elevationUnit, minElevation, maxElevation,
				GC_UNIT_METERS, minTargetElevation, maxTargetElevation, -32767.0f, 32767.0f);
			privateData->byteOrder = boBigEndian;
			privateData->numBits = 16;
			privateData->noData.intValue = -32768;
			retval = WriteHGT(mapInfo, stream);
			break;
		}
		case gcPlanetaryDataSystem: {
			break;
		}
		case gcGridFloat: {
			break;
		}
		case gcBIL: {
			break;
		}
		case gcTerragen: {
			break;
		}
		case gcPGM: {
			break;
		}
		case gcArcInfoASCII: {
			if (minTargetElevation > -9999) {
				privateData->noData.intValue = -9999;
			}
			else {
				privateData->noData.intValue = floor(minTargetElevation - 1);
			}
			gcScaleToTargetElevation(mapInfo, elevationUnit, minElevation, maxElevation,
				GC_UNIT_METERS, minTargetElevation, maxTargetElevation, FLT_MIN, FLT_MAX);
			retval = WriteArcInfoASCII(mapInfo, stream);
			break;
		}
	}//end format switch
	

	fclose(stream);

	return retval;
}

int gcIsWholeNumberElevation(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;

	return privateData->isWholeNumber;
}

int gcGetElevationUnit(gcMapInfo *mapInfo) {
	gcPrivateData* privateData = mapInfo->privateData;

	return privateData->elevationUnit;
}

