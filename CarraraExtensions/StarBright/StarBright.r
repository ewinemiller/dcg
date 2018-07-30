/*  StarBright - plug-in for Carrara
    Copyright (C) 2000 Michael Clarke

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
#include "ExternalAPI.r"
#include "External3DAPI.r"
#include "Copyright.h"
#include "StarBrightDef.h"
#include "interfaceids.h"


include "StarBright.rsr";

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif
   

resource 'GUID' (135)
{
	{
		R_IID_I3DExBackground,
		R_CLSID_StarBright
	}
};

resource 'COMP' (135)
{
	kRID_BackgroundFamilyID,
	'STBR',
	"StarBright",
	"Miks",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource 'PMap' (135)
{
	{
		'SEED',	'in32',	noFlags, "PRN Seed", "",
		'NSTR',	'in32',	interpolate, "Number of Stars", "",
		'FIRM',	'colo',	interpolate, "Firmament colour", "",
		'HORZ',	'colo',	interpolate, "Horizon colour", "",
		'CO01',	'colo',	interpolate, "Star colour  1", "",
		'CO02',	'colo',	interpolate, "Star colour  2", "",
		'CO03',	'colo',	interpolate, "Star colour  3", "",
		'CO04',	'colo',	interpolate, "Star colour  4", "",
		'CO05',	'colo',	interpolate, "Star colour  5", "",
		'CO06',	'colo',	interpolate, "Star colour  6", "",
		'CO07',	'colo',	interpolate, "Star colour  7", "",
		'CO08',	'colo',	interpolate, "Star colour  8", "",
		'CO09',	'colo',	interpolate, "Star colour  9", "",
		'CO10',	'colo',	interpolate, "Star colour 10", "",
		'CO11',	'colo',	interpolate, "Star colour 11", "",
		'CO12',	'colo',	interpolate, "Star colour 12", "",
		'MAGN',	're32',	interpolate, "Magnification", "",
		'INTS',	're32',	interpolate, "Intensity", "",
		'ROTA',	're32',	interpolate, "Rotation", "",
		'ELEV',	're32',	interpolate, "Elevation", "",
		'XPOS',	'bool',	noFlags, "X Positive Values", "",
		'YPOS',	'bool',	noFlags, "Y Positive Values", "",
		'ZNEG',	'bool',	noFlags, "Z Negative Values", "",
	}
};
