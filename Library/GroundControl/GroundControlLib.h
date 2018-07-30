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
#ifndef _GC_LIB
#define _GC_LIB 1

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GC_OK 1
#define GC_ERROR 0

#define GC_TRUE 1
#define GC_FALSE 0

#define GC_EAST 0
#define GC_NORTH 1

#define GC_POINT_FILTERING 1
#define GC_BILINEAR_FILTERING 2

#define GC_MESH_GRID_POINT 0
#define GC_MESH_GRID_BILINEAR 1
#define GC_MESH_GRID_SMART 2
#define GC_MESH_ADAPTIVE 3
    
#define GC_NO_DATA_PERIMETER_AVERAGE 0
#define GC_NO_DATA_SPECIFIED 1
#define GC_NO_DATA_MININUM_ELEVATION 2
#define GC_NO_DATA_MAXIMUM_ELEVATION 3
#define GC_NO_DATA_TRANSPARENT 4

#define GC_UNIT_METERS 0
#define GC_UNIT_FEET 1
#define GC_UNIT_DECIMETERS 2
#define GC_UNIT_UNKNOWN 3

#define GC_LL_UNIT_UNKNOWN 0
#define GC_LL_UNIT_METERS 1
#define GC_LL_UNIT_FEET 2
#define GC_LL_UNIT_RADIANS 3
#define GC_LL_UNIT_ARC_SECONDS 4
#define GC_LL_UNIT_LATLONG 5


typedef struct gcPoint_struct {
	long east, north; 
	float elevation;
} gcPoint;

typedef struct gcFacet_struct {
	unsigned long pointIndex[3];
} gcFacet;

typedef struct gcEdge_struct {
	//-1 indicates that this edge only bounds one facet and will always be the second
	long facetIndex[2];
	unsigned long pointIndex[2];
} gcEdge;

typedef struct gcFacetEdge_struct {
	unsigned long edgeIndex[3];
} gcFacetEdge;

typedef struct gcMesh_struct {
	gcFacet *facets;
	gcPoint *points;
	gcEdge *edges;
	gcFacetEdge *facetEdges;
	unsigned long pointCount;
	unsigned long facetCount;
	float *EPercent;
	float *NPercent;
	unsigned long samples[2];
} gcMesh;

typedef struct gcMapInfo_struct{
	unsigned int samples[2];
	unsigned long bufferSize;
	float minElevation, maxElevation;
	unsigned char hasRealSize;
	unsigned char hasRealElevation;
	unsigned char hasLLCorner;
	float size[2];
	float llCorner[2];
	unsigned int llUnit;

	gcMesh* mesh;
	void *privateData;
	char *description;
} gcMapInfo;

struct gcFileCapability_struct {
	int supportsTransparency;
};

//returns GC_OK or GC_ERROR for whether or not a file can be read
extern int gcCanOpenFile(const char *, gcMapInfo *mapInfo);
extern int gcOpenFile(const char *, gcMapInfo *mapInfo);
extern void gcFillNoData(gcMapInfo *mapInfo, const unsigned long, const double);
extern void gcInitialize(gcMapInfo *mapInfo);
extern void gcCleanUp(gcMapInfo *mapInfo);
extern void gcCleanUpMesh(gcMapInfo *mapInfo);

extern void gcSetExternalBuffer(gcMapInfo *mapInfo, float *);

extern unsigned long gcGetElevationIndex(const gcMapInfo *mapInfo, const unsigned long, const unsigned long);
extern unsigned long gcGetFullPolyCount(const gcMapInfo *mapInfo);
extern float gcGetElevationBySample(const gcMapInfo *mapInfo, const unsigned long, const float, const float);
extern float gcGetElevationByCoordinates(const gcMapInfo *mapInfo, const unsigned long, const unsigned long);
extern void gcSetElevationByCoordinates(const gcMapInfo *mapInfo, const unsigned long, const unsigned long, const float);
extern float gcGetElevationByIndex(const gcMapInfo *mapInfo, const unsigned long);
extern float* gcGetElevationLineBuffer(const gcMapInfo *mapInfo, const unsigned long);
extern int gcCreateMesh(gcMapInfo *mapInfo, const unsigned long targetPolyCount, const unsigned long meshType, float adaptiveError);
extern void gcOverrideElevationRange(gcMapInfo *mapInfo, float minElevation, float maxElevation);

extern void gcNormalizeElevation(gcMapInfo *mapInfo);
extern void gcNormalizeAndClipElevation(gcMapInfo *mapInfo);
extern void gcScaleElevation(gcMapInfo *mapInfo);

extern void gcRegisterProgressCallback(gcMapInfo *mapInfo, void (*progressCallback)(void *, float), void* callbackData);
extern void gcDeregisterProgressCallback(gcMapInfo *mapInfo);

extern int gcSetBufferSize(gcMapInfo *mapInfo, const unsigned long, const unsigned long);
extern int gcSetElevationByIndex(gcMapInfo *mapInfo, const unsigned long, const float);
extern void gcSetHasNoData(gcMapInfo *mapInfo, int hasNoData);

extern int gcWriteFile(const char *filename, const char *finalFilename, gcMapInfo *mapInfo,
	int elevationUnit, float minElevation, float maxElevation,
	int targetElevationUnit, float minTargetElevation, float maxTargetElevation);
//extern int gcGetWriteRequirements(const char *finalFilename, gcMapInfo *mapInfo);

extern int gcIsWholeNumberElevation(gcMapInfo *mapInfo);
extern int gcGetElevationUnit(gcMapInfo *mapInfo);

#ifdef __cplusplus
}
#endif

#endif