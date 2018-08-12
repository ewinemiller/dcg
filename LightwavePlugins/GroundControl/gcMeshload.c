/*  Ground Control - plug-in for LightWave
Copyright (C) 2014 Eric Winemiller

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
#include "boolean.h"
#include <lwserver.h>
#include <lwobjimp.h>
#include <lwsurf.h>
#include <lwhost.h>
#include <lwvolume.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <GroundControlLib.h>
#include <lwpanel.h>
#include "gcMain.h"


int getMeshImportOptions(GlobalFunc *global, const gcMapInfo *mapInfo, int fullPolyCount
	, float *sizeX, float *sizeZ, float *meshDensity, float *adaptiveError, int *meshType
	, float *minElevation, float *maxElevation) {
	int ok = 0;
	LWPanelFuncs *panf = global(LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );         
	if (panf)
	{
		LWPanelID panel;
		LWControl *sizeXControl = NULL;
		LWControl *sizeZControl = NULL;
		LWControl *minElevationControl = NULL;
		LWControl *maxElevationControl = NULL;
		LWControl *percentControl = NULL;
		LWControl *errorControl = NULL;
		LWControl *meshTypeControl = NULL;
		LWControl *verticalScaleControl = NULL;
		int verticalScale = 0;

		LWPanControlDesc desc;           
		static LWValue ival={LWT_INTEGER},fval={LWT_FLOAT};

		char sizeStr[80];
		char instructions1[80];
		char instructions2[80];
		const char *text[2] = {
			sizeStr,
			NULL
		};
		const char *twolineinstructions[3] = {
			instructions1,
			instructions2,
			NULL
		};
		const char *onelineinstructions[2] = {
			instructions1,
			NULL
		};
		const char *meshTypeChoices[5] = {
			"Grid - point sampling",
			"Grid - bilinear sampling",
			"Grid - BS and smart split",
			"Adaptive",
			NULL
		};
		const char *verticalScaleChoices[7] = {
			"Fixed",
			"1/10th",
			"1/100th",
			"1/1000th",
			"1/10000th",
			NULL
		};

		sprintf(sizeStr, "Polygons in a full size grid mesh: %d", fullPolyCount);

		panel = PAN_CREATE( panf, "Ground Control Import Options" );

		if (mapInfo->hasRealSize) {
			sprintf(instructions1, "This elevation map has real world sizes.");
			sprintf(instructions2, "Scale will be applied to all sizes.");
			TEXT_CTL( panf, panel, "", twolineinstructions );	
			sizeXControl = FLOAT_CTL(  panf, panel, "Size X" );
			SET_FLOAT(sizeXControl, *sizeX);
			sizeZControl = FLOAT_CTL(  panf, panel, "Size Z" );
			SET_FLOAT(sizeZControl, *sizeZ);
			minElevationControl = FLOAT_CTL(  panf, panel, "Minimum Elevation" );
			SET_FLOAT(minElevationControl, *minElevation);
			maxElevationControl = FLOAT_CTL(  panf, panel, "Maximum Elevation" );
			SET_FLOAT(maxElevationControl, *maxElevation);
			verticalScaleControl = VCHOICE_CTL( panf, panel, "Scale", verticalScaleChoices );
			SET_INT(verticalScaleControl, verticalScale);
		}
		else if (mapInfo->hasRealElevation) {
			sprintf(instructions1, "This elevation map has real world elevation.");
			sprintf(instructions2, "Vertical scale will be applied to the elevation.");
			TEXT_CTL( panf, panel, "", twolineinstructions );	
			sizeXControl = FLOAT_CTL(  panf, panel, "Size X" );
			SET_FLOAT(sizeXControl, *sizeX);
			sizeZControl = FLOAT_CTL(  panf, panel, "Size Z" );
			SET_FLOAT(sizeZControl, *sizeZ);
			minElevationControl = FLOAT_CTL(  panf, panel, "Minimum Elevation" );
			SET_FLOAT(minElevationControl, *minElevation);
			maxElevationControl = FLOAT_CTL(  panf, panel, "Maximum Elevation" );
			SET_FLOAT(maxElevationControl, *maxElevation);
			verticalScaleControl = VCHOICE_CTL( panf, panel, "Vertical scale", verticalScaleChoices );
			SET_INT(verticalScaleControl, verticalScale);
		}
		else {
			sprintf(instructions1, "This elevation map has no real world sizes.");
			TEXT_CTL( panf, panel, "", onelineinstructions );	
			sizeXControl = FLOAT_CTL(  panf, panel, "Size X" );
			SET_FLOAT(sizeXControl, *sizeX);
			sizeZControl = FLOAT_CTL(  panf, panel, "Size Z" );
			SET_FLOAT(sizeZControl, *sizeZ);
			minElevationControl = FLOAT_CTL(  panf, panel, "Minimum Elevation" );
			SET_FLOAT(minElevationControl, *minElevation);
			maxElevationControl = FLOAT_CTL(  panf, panel, "Maximum Elevation" );
			SET_FLOAT(maxElevationControl, *maxElevation);
		}

		meshTypeControl = VCHOICE_CTL( panf, panel, "Mesh type", meshTypeChoices );
		SET_INT(meshTypeControl, *meshType);
		errorControl = PERCENT_CTL(panf, panel, "Max error for Adaptive" );
		SET_FLOAT(errorControl, *adaptiveError * 100);

		percentControl = PERCENT_CTL(panf, panel, "Mesh density" );
		SET_FLOAT(percentControl, *meshDensity * 100);
		TEXT_CTL( panf, panel, "", text );	

		ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );
		if (ok) {
			GET_FLOAT(sizeXControl, *sizeX);
			GET_FLOAT(sizeZControl, *sizeZ);
			GET_FLOAT(minElevationControl, *minElevation);
			GET_FLOAT(maxElevationControl, *maxElevation);

			if (verticalScaleControl) {
				GET_INT(verticalScaleControl, verticalScale);
			}

			if (mapInfo->hasRealSize) {
				*minElevation /= pow(10, verticalScale);
				*maxElevation /= pow(10, verticalScale);
				*sizeX /= pow(10, verticalScale);
				*sizeZ /= pow(10, verticalScale);
			}
			else if (mapInfo->hasRealElevation) {
				*minElevation /= pow(10, verticalScale);
				*maxElevation /= pow(10, verticalScale);
			}

			//seems to be some errors where this percent control doesn't respect the limits all the time.
			GET_FLOAT(percentControl, *meshDensity);
			*meshDensity = *meshDensity * .01;
			if (*meshDensity > 1.0f)
				*meshDensity = 1.0f;
			else if (*meshDensity < .01f)
				*meshDensity = .01f;
			GET_FLOAT(errorControl, *adaptiveError);
			*adaptiveError = *adaptiveError * .01;
			if (*adaptiveError > 1.0f)
				*adaptiveError = 1.0f;
			else if (*adaptiveError < 0)
				*adaptiveError = 0;
			GET_INT(meshTypeControl, *meshType);
		}
		PAN_KILL( panf, panel );
	}
	return ok;
}

/*
======================================================================
geoLoad()

Activation function for the loader.
====================================================================== */

XCALL_( int ) gcMeshLoad( long version, GlobalFunc *global, LWObjectImport *local, void *serverData ) {

	LWFVector pos = { 0.0f };
	LWPntID *pntID = NULL;
	struct stat s;
	char *err = NULL;
	unsigned long east, north, point, facet;

	float* XLines = NULL;
	float* ZLines = NULL;
	float sizeZ = 20.0f, sizeX = 20.0f, sizeY = 1.0f, meshDensity = .1f, adaptiveError = .01f;
	int meshType = GC_MESH_GRID_SMART;
	int fullPolyCount, targetPolyCount;

	char tag[ 40 ] = "GroundControl";

	gcMapInfo fullMapInfo;

	gcInitialize(&fullMapInfo);

	/* check the activation version */
	if ( version != LWOBJECTIMPORT_VERSION ) return AFUNC_BADVERSION;

	/* get the file size */
	if ( stat( local->filename, &s )) {
		local->result = LWOBJIM_BADFILE;
		goto Finish;
	}

	if (gcCanOpenFile(local->filename, &fullMapInfo)== GC_ERROR)
	{
		local->result = LWOBJIM_NOREC;
		goto Finish;
	}

	if (gcOpenFile(local->filename, &fullMapInfo)== GC_ERROR)
	{
		local->result = LWOBJIM_BADFILE;
		goto Finish;
	}	

	gcNormalizeAndClipElevation(&fullMapInfo);

	gcFillNoData(&fullMapInfo, GC_NO_DATA_PERIMETER_AVERAGE, 0);
	
	fullPolyCount = gcGetFullPolyCount(&fullMapInfo);

	if (fullMapInfo.hasRealSize == GC_TRUE) {
		sizeZ = fullMapInfo.size[GC_NORTH];
		sizeX = fullMapInfo.size[GC_EAST];
	}
	else {
		if (fullMapInfo.hasRealElevation == GC_FALSE) {
			fullMapInfo.minElevation = 0;
			fullMapInfo.maxElevation = 1.0f;
		}
		if (fullMapInfo.samples[GC_EAST] > fullMapInfo.samples[GC_NORTH]) {
			sizeZ *= (float)fullMapInfo.samples[GC_NORTH] / (float)fullMapInfo.samples[GC_EAST];
		}
		else if (fullMapInfo.samples[GC_EAST] < fullMapInfo.samples[GC_NORTH]) {
			sizeX *= (float)fullMapInfo.samples[GC_EAST] / (float)fullMapInfo.samples[GC_NORTH];
		}
	}

	if (fullPolyCount > 100000000) {
		meshDensity = .001f;
	}
	else if (fullPolyCount > 10000000) {
		meshDensity = .01f;
	}

	if (!getMeshImportOptions(global, &fullMapInfo, fullPolyCount, &sizeX, &sizeZ, &meshDensity, &adaptiveError, &meshType, 
		&fullMapInfo.minElevation, &fullMapInfo.maxElevation)) {
		local->result = LWOBJIM_ABORTED;
		goto Finish;
	}

	/* assume this until we succeed */
	local->result = LWOBJIM_FAILED;   

	/* initialize the layer */

	local->layer( local->data, 1, NULL );
	local->pivot( local->data, pos );

	targetPolyCount = (float)fullPolyCount * meshDensity;

	if (gcCreateMesh(&fullMapInfo, targetPolyCount, meshType, adaptiveError) == GC_ERROR) {
		local->result = LWOBJIM_FAILED;
		goto Finish;
	}

	//allocate space to store point IDs 
	pntID = calloc( fullMapInfo.mesh->pointCount, sizeof( LWPntID ));

	if ( !pntID )  {
		err = "Couldn't allocate memory for points.";
		local->result = LWOBJIM_FAILED;
		goto Finish;
	}

	XLines = calloc( fullMapInfo.mesh->samples[GC_EAST], sizeof( float ));
	if ( !XLines ) goto Finish;

	ZLines = calloc( fullMapInfo.mesh->samples[GC_NORTH], sizeof( float ));
	if ( !ZLines ) goto Finish;

	local->result = LWOBJIM_OK;

	for (east = 0; east < fullMapInfo.mesh->samples[GC_EAST]; east++){
		XLines[east] = sizeX * fullMapInfo.mesh->EPercent[east] - sizeX * 0.5f;
	}

	for (north = 0; north < fullMapInfo.mesh->samples[GC_NORTH]; north++){
		ZLines[north] = sizeZ * fullMapInfo.mesh->NPercent[north] - sizeZ * 0.5f;
	}

	sizeY = fullMapInfo.maxElevation - fullMapInfo.minElevation;

	for (point = 0; point < fullMapInfo.mesh->pointCount; point++) {
		pos[0] = XLines[fullMapInfo.mesh->points[point].east];
		pos[1] = fullMapInfo.minElevation + fullMapInfo.mesh->points[point].elevation * sizeY;
		pos[2] = ZLines[fullMapInfo.mesh->points[point].north];
		if ( !( pntID[ point ] = local->point( local->data, pos ))) {
			err = "Couldn't create point.";
			goto Finish;
		}
	}
	for (facet = 0; facet < fullMapInfo.mesh->facetCount; facet++) {
		LWPolID polID;
		static LWPntID vID[ 3 ];

		vID[0] = pntID[ fullMapInfo.mesh->facets[facet].pointIndex[0] ];
		vID[1] = pntID[ fullMapInfo.mesh->facets[facet].pointIndex[1] ];
		vID[2] = pntID[ fullMapInfo.mesh->facets[facet].pointIndex[2] ];
		if ( !(  polID = local->polygon( local->data, LWPOLTYPE_FACE, 0, 3, vID ))) {
			err = "Couldn't create facet.";
			goto Finish;
		}
		local->polTag( local->data, polID, LWPTAG_SURF, tag );

	}

	/* create the surface descriptions */

	if ( local->result == LWOBJIM_OK )
		local->surface( local->data, tag, NULL, 0, NULL );

	/* we're done */
	local->done( local->data );

Finish:
	if (pntID) free (pntID);
	if (XLines) free (XLines);
	if (ZLines) free (ZLines);

	gcCleanUp(&fullMapInfo);

	if (( local->result != LWOBJIM_OK ) && err && ( local->failedLen > 0 ))
		strncpy( local->failedBuf, err, local->failedLen );

	return AFUNC_OK;
}

