/*  Ground Control - plug-in for DAZStudio
 Copyright (C) 2015 Eric Winemiller
 
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
/*****************************
   Include files
*****************************/
#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "GroundControlImporter.h"

/*****************************
   Plugin Definition
*****************************/

DZ_PLUGIN_DEFINITION( "Ground Control" );

DZ_PLUGIN_AUTHOR( "DCG" );

DZ_PLUGIN_VERSION( PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD );

DZ_PLUGIN_DESCRIPTION( QString(
                      "<a href=\"http://www.digitalcarversguild.com/plugin.php?ProductId=7\">Digital Carvers Guild Ground Control</a><br><br>"
                      "Ground Control is a digital elevation map importer plug-in with "
                      "support for GridFloat FLT, HiRISE Planetary Data System IMG files, "
                      "classic USGS DEMs, VistaPro format DEMs, World Machine 32-bit "
                      "RAW FP, Terragen TER files, Shuttle Radar Topography Mission "
					  "HGT files, ArcInfo ASCII Grid, text format PGM, and BIL files."
                      ));

DZ_PLUGIN_CLASS_GUID( GroundControlImporter, F3808728-DA87-440d-9541-D9E5424CF3B7);


