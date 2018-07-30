/*  DCG Importer - plug-in for Carrara
    Copyright (C) 2012 Eric Winemiller

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
#include "interfaceids.h"

#include "SequencedObjPrimDef.h"
#include "SequencedObjImpDef.h"

include "DCGImporter.rsr"; 
#define COPYRIGHT "Copyright © 2012 Digital Carvers Guild"
#define DCGMODGROUP "Digital Carvers Guild"

resource 'GUID' (100)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_SequencedObjPrim
	}
};

resource 'COMP' (100)
{
	kRID_GeometricPrimitiveFamilyID,
	'DISO',
	"",
	"Hidden",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (100) {
	{	
		'ANIM','re32',interpolate,"Animate","",
		'PREV','actn',noFlags,"Preview","",
		'TrCo','bool',noFlags,"Map OBJ Y to Carrara Z axis","",
		'Conv','re32',noFlags,"Conversion","",
		'Smoo','re32',noFlags,"Smoothing","",
		'Obj0','in32',noFlags,"Object Type","",
		'UV01','bool',noFlags,"UV Between 0 and 1",""
	}
};

resource 'STR#' (100)
{
	{
		"The file has moved, please find a file from the sequence",	// string  1
		"Calculating bounding box",	// string  2
	}
};

resource 'GUID' (130)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_SequencedObjImp
	}
};

resource 'COMP' (130)
{
	kRID_ImportFilterFamilyID,
	'DCOI',
	"Sequenced OBJ Import",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (130)
{
	'3Din',
	'DCOI',
	isAlienType,
	'ttxt',					// creator
	{
		'DCOI', 			// Reference
		"Sequenced OBJ by DCG",	// Name
		{'TEXT' },		// List of MacOS types
		{"OBJ"},		// List of extensions
	};
};

resource 'PMap' (130) {
	{	
		'TrCo','bool',noFlags,"Map OBJ Y to Carrara Z axis","",
		'Conv','re32',noFlags,"Conversion","",
		'Smoo','re32',noFlags,"Smoothing","",
		'Obj0','in32',noFlags,"Object Type","",
		'UV01','bool',noFlags,"UV Between 0 and 1",""
	}
};
