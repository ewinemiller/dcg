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
#include <lwserver.h>
#include <lwimage.h>
#include <lwhost.h>
#include <lwpanel.h>
#include <sys/stat.h>
#include <GroundControlLib.h>
#include <stdlib.h>
#include "gcMain.h"

#define GREYSCALE_FP 0
#define GREYSCALE_8 1
#define RGB_FP 2
#define RGB_8 3


int getImageImportOptions(GlobalFunc *global, int *importAs) {
	int ok = 0;
	LWPanelFuncs *panf = global(LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );         
	if (panf)
	{
		LWPanelID panel;
		LWControl *importAsControl = NULL;
		LWPanControlDesc desc;           
		static LWValue ival={LWT_INTEGER};


		const char *importAsChoices[5] = {
			"Greyscale floating point",
			"Greyscale 8-bit",
			"RGB floating point",
			"RGB 8-bit",
			NULL
		};

		panel = PAN_CREATE( panf, "Ground Control Import Options" );

		importAsControl = VCHOICE_CTL( panf, panel, "Import As", importAsChoices );
		SET_INT(importAsControl, *importAs);

		ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );
		if (ok) {
			GET_INT(importAsControl, *importAs);

		}
		PAN_KILL( panf, panel );
	}
	return ok;
}


XCALL_( int ) gcImageLoad( int version, GlobalFunc *global, LWImageLoaderLocal *local, void *serverData )
{
	gcMapInfo fullMapInfo;
	unsigned long north;
	LWImageProtocolID ip = NULL;
	int importAs = GREYSCALE_FP;

	gcInitialize(&fullMapInfo);

	if (gcCanOpenFile(local->filename, &fullMapInfo)== GC_ERROR)
	{
		local->result = IPSTAT_NOREC;
		goto Finish;
	}	

	if (gcOpenFile(local->filename, &fullMapInfo)== GC_ERROR)
	{
		local->result = IPSTAT_BADFILE;
		goto Finish;
	}

	gcNormalizeAndClipElevation(&fullMapInfo);

	gcFillNoData(&fullMapInfo, GC_NO_DATA_PERIMETER_AVERAGE, 0);

	if (!getImageImportOptions(global, &importAs)) {
		local->result = IPSTAT_ABORT;
		goto Finish;
	}

	switch (importAs) {
		case GREYSCALE_FP : {
			ip = local->begin( local->priv_data, LWIMTYP_GREYFP );
			break;
		}
		case GREYSCALE_8 : {
			ip = local->begin( local->priv_data, LWIMTYP_GREY8 );
			break;
		}
		case RGB_FP : {
			ip = local->begin( local->priv_data, LWIMTYP_RGBFP );
			break;
		}
		case RGB_8 : {
			ip = local->begin( local->priv_data, LWIMTYP_RGBA32 );
			break;
		}
	}

	LWIP_SETSIZE( ip, fullMapInfo.samples[GC_EAST], fullMapInfo.samples[GC_NORTH]);

	//texture origin in LW is upper left
	switch (importAs) {
		case GREYSCALE_FP : {
			for (north = 0; north < fullMapInfo.samples[GC_NORTH]; north++) {
				LWIP_SENDLINE( ip, fullMapInfo.samples[GC_NORTH] - 1 - north, gcGetElevationLineBuffer(&fullMapInfo, north) );
			}			
			break;
		}
		case GREYSCALE_8 : {
			unsigned char* buffer = NULL;
			unsigned long east;
			buffer = calloc( fullMapInfo.samples[GC_EAST], sizeof( unsigned char ));
			if ( !buffer )  {
				local->result = IPSTAT_FAILED;
				goto Finish;
			}
			for (north = 0; north < fullMapInfo.samples[GC_NORTH]; north++) {
				for (east = 0; east < fullMapInfo.samples[GC_EAST]; east++) {
					buffer[east] = gcGetElevationByCoordinates(&fullMapInfo, east, north) * 255.0f;
				}
				LWIP_SENDLINE( ip, fullMapInfo.samples[GC_NORTH] - 1 - north, buffer );
			}			
			free(buffer);
			break;
		}		case RGB_FP : {
			LWPixelRGBFP* buffer = NULL;
			unsigned long east;
			buffer = calloc( fullMapInfo.samples[GC_EAST], sizeof( LWPixelRGBFP ));
			if ( !buffer )  {
				local->result = IPSTAT_FAILED;
				goto Finish;
			}
			for (north = 0; north < fullMapInfo.samples[GC_NORTH]; north++) {
				for (east = 0; east < fullMapInfo.samples[GC_EAST]; east++) {
					buffer[east].r = buffer[east].g = buffer[east].b = gcGetElevationByCoordinates(&fullMapInfo, east, north);
				}
				LWIP_SENDLINE( ip, fullMapInfo.samples[GC_NORTH] - 1 - north, buffer );
			}			
			free(buffer);
			break;
		}
		case RGB_8 : {
			LWPixelRGBA32* buffer = NULL;
			unsigned long east;
			buffer = calloc( fullMapInfo.samples[GC_EAST], sizeof( LWPixelRGBA32 ));
			if ( !buffer )  {
				local->result = IPSTAT_FAILED;
				goto Finish;
			}
			for (north = 0; north < fullMapInfo.samples[GC_NORTH]; north++) {
				for (east = 0; east < fullMapInfo.samples[GC_EAST]; east++) {
					buffer[east].r = buffer[east].g = buffer[east].b = gcGetElevationByCoordinates(&fullMapInfo, east, north) * 255.0f;
					buffer[east].a = 255;
				}
				LWIP_SENDLINE( ip, fullMapInfo.samples[GC_NORTH] - 1 - north, buffer );
			}			
			free(buffer);
			break;
		}
	}

	local->result = LWIP_DONE( ip, IPSTAT_OK );
	local->done( local->priv_data, ip );
Finish:
	gcCleanUp(&fullMapInfo);

	return AFUNC_OK;
}