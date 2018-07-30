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
#include "ExternalAPI.r"
#include "copyright.h"
#include "DEMDef.h"
#include "DEMPrimDef.h"
#include "GCWaterDef.h"
#include "GCBumpDef.h"
#if (VERSIONNUMBER >= 0x040000)
#include "GCShapefileDef.h"
#include "GCGeneratorDef.h"
#include "ShaderFilterDef.h"
#endif 
#include "interfaceids.h"
#include "External3DAPI.r"
//#include "Types.r"
#if (VERSIONNUMBER < 0x050000)
//#include "imageAPI.r"
#endif 

include "GroundControl.rsr"; 
#define COPYRIGHT "Copyright © 2003 Digital Carvers Guild"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'GUID' (130)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_DEM
	}
};

resource 'COMP' (130)
{
	kRID_ImportFilterFamilyID,
	'DDEM',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (130)
{
	'3Din',
	'DDEM',
	isAlienType,
	'ttxt',					// creator
	{
		'DDEM', 			// Reference
		"DEM by Ground Control",	// Name
		{'TEXT' },		// List of MacOS types
		{"DEM"},		// List of extensions
	};
};

resource 'STR#' (130)
{
	{
		"Loading Ground Control object",	// string  1
		"Building Ground Control Mesh",			// string  2
		"Open Elevation Map", // string 3
		"Open Shapefile", // string 4
	}
};

resource 'PMap' (130) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (131)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_DEMPrim
	}
};

resource 'COMP' (131)
{
	kRID_GeometricPrimitiveFamilyID,
	'DEMP',
	"",
	"Hidden",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (131) {
	{	
		'RESO','re32',1,"Resolution","",
		'SIZX','re32',1,"North","",
		'SIZY','re32',1,"East","",
		'SIZZ','re32',1,"Top Elevation", "",
		'STAR','re32',1,"Bottom Elevation", "",
		'PREV','in32',0,"Preview Size","",
		'DESC','s255',0,"Description","",
		'NATI','s255',0,"Resolution","",
		'erro','re32',1,"Max Error","",
		'FACE','s255',0,"Facets","",
		'ANIM','re32',interpolate,"Animate","",
		'INTE','bool',0,"Interpolate Frames","",
		'METY','actn',0,"Mesh type","",
	}
};

resource 'COMP' (304) {
	kRID_ShaderFamilyID,
	'GCWT',
	"Ground Control Water",
	"Ground Control",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (304) {
	{	
		'SAMP','in32',0,"Samples","",
		'THRS','re32',0,"Threshold","",
		'DIST','re32',0,"Distance","",
		'HEIG','re32',0,"Height","",
		'sh00','comp',noFlags, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (304) {
{
		R_IID_I3DExShader,
		R_CLSID_GCWater
}
};

resource 'CPUI' (304) 
{
	304,					// Id of your main part
	1304,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (305) {
	kRID_ShaderFamilyID,
	'GCSP',
	"Ground Control SRTM Shapefile",
	"Ground Control",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (305) {
	{	
		'INVT', 'bool', noflags, "Invert", "",
		'LONG','vec2',noflags,"Longitude","",
		'LATI','vec2',noflags,"Latitude","",
	}
};


resource 'GUID' (305) {
{
		R_IID_I3DExShader,
		R_CLSID_GCShapefile
}
};

resource 'CPUI' (305) 
{
	305,					// Id of your main part
	1305,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'FTYP' (305)
{
{
	'SHPF', 			// Reference
	"SRTM Shapefile by Ground Control",	// Name
	{'SHPF' },		// List of MacOS types
	#if (VERSIONNUMBER >= 0x040000)
	{"SHP"},		// List of extensions
	#else
	{'SHP ' },		// List of extensions
	#endif
	};
};

resource 'COMP' (306) {
	kRID_ShaderFamilyID,
	'GCST',
	"Ground Control SRTM Shapefile",
	"Terrain Distribution # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (306) {
	{	
		'INVT', 'bool', noflags, "Invert", "",
		'LONG','vec2',noflags,"Longitude","",
		'LATI','vec2',noflags,"Latitude","",
	}
};


resource 'GUID' (306) {
{
		R_IID_I3DExShader,
		R_CLSID_GCShapefile
}
};

resource 'CPUI' (306) 
{
	305,					// Id of your main part
	1305,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (307) {
	kRID_ShaderFamilyID,
	'GCSH',
	"Ground Control Primitive",
	"Ground Control",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (307) {
{
		R_IID_I3DExShader,
		R_CLSID_GCBump
}
};

resource 'GUID' (132)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_r32
	}
};

resource 'COMP' (132)
{
	kRID_ImportFilterFamilyID,
	'GCWM',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (132)
{
	'3Din',
	'GCWM',
	isAlienType,
	'ttxt',					// creator
	{
		'GCWM', 			// Reference
		"World Machine by Ground Control",	// Name
		{'TEXT' },		// List of MacOS types
		#if (VERSIONNUMBER >= 0x040000)
		{"r32"},		// List of extensions
		#else
		{'r32 ' },		// List of extensions
		#endif
	};
};

resource 'PMap' (132) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (133)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_TER
	}
};

resource 'COMP' (133)
{
	kRID_ImportFilterFamilyID,
	'GCTG',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (133)
{
	'3Din',
	'GCTG',
	isAlienType,
	'ttxt',					// creator
	{
		'GCTG', 			// Reference
		"Terragen by Ground Control",	// Name
		{'TEXT' },		// List of MacOS types
		#if (VERSIONNUMBER >= 0x040000)
		{"TER"},		// List of extensions
		#else
		{'TER ' },		// List of extensions
		#endif
	};
};

resource 'PMap' (133) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (134)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_BIL
	}
};

resource 'COMP' (134)
{
	kRID_ImportFilterFamilyID,
	'GCBI',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (134)
{
	'3Din',
	'GCBI',
	isAlienType,
	'ttxt',					// creator
	{
		'GCBI', 			// Reference
		"BIL by Ground Control",	// Name
		{'TEXT' },		// List of MacOS types
		#if (VERSIONNUMBER >= 0x040000)
		{"BIL"},		// List of extensions
		#else
		{'BIL ' },		// List of extensions
		#endif
	};
};

resource 'PMap' (134) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (135)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_HGT
	}
};

resource 'COMP' (135)
{
	kRID_ImportFilterFamilyID,
	'GCHG',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (135)
{
	'3Din',
	'GCHG',
	isAlienType,
	'ttxt',					// creator
	{
		'GCHG', 			// Reference
		"SRTM by Ground Control",	// Name
		{'TEXT' },		// List of MacOS types
		#if (VERSIONNUMBER >= 0x040000)
		{"HGT"},		// List of extensions
		#else
		{'HGT ' },		// List of extensions
		#endif
	};
};

resource 'PMap' (135) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (136)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_PGM
	}
};

resource 'COMP' (136)
{
	kRID_ImportFilterFamilyID,
	'GCPG',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (136)
{
	'3Din',
	'GCPG',
	isAlienType,
	'ttxt',					// creator
	{
		'GCPG', 			// Reference
		"PGM by Ground Control",	// Name
		{'TEXT', 'TEXT' },		// List of MacOS types
		#if (VERSIONNUMBER >= 0x040000)
		{"PGM", "PGMA"},		// List of extensions
		#else
		{'PGM ',"PGMA"},		// List of extensions
		#endif
	};
};

resource 'PMap' (136) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (138)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_ASC
	}
};

resource 'COMP' (138)
{
	kRID_ImportFilterFamilyID,
	'GCAA',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (138)
{
	'3Din',
	'GCAA',
	isAlienType,
	'ttxt',					// creator
	{
		'GCAA', 			// Reference
		"ArcInfo ASCII Grid by Ground Control",	// Name
		{'TEXT', 'TEXT' },		// List of MacOS types
		{"ASC"},		// List of extensions
	};
};

resource 'PMap' (138) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (139)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_FLT
	}
};

resource 'COMP' (139)
{
	kRID_ImportFilterFamilyID,
	'GCGF',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (139)
{
	'3Din',
	'GCGF',
	isAlienType,
	'ttxt',					// creator
	{
		'GCGF', 			// Reference
		"GridFloat by Ground Control",	// Name
		{'TEXT', 'TEXT' },		// List of MacOS types
		{"FLT"},		// List of extensions
	};
};

resource 'PMap' (139) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (140)
{
	{
		R_IID_I3DExImportFilter,
		R_CLSID_PDS
	}
};

resource 'COMP' (140)
{
	kRID_ImportFilterFamilyID,
	'GCPD',
	"Ground Control",
	"SDK Examples",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Din' (140)
{
	'3Din',
	'GCPD',
	isAlienType,
	'ttxt',					// creator
	{
		'GCPD', 			// Reference
		"Planetary Data System by Ground Control",	// Name
		{'TEXT', 'TEXT' },		// List of MacOS types
		{"IMG"},		// List of extensions
	};
};

resource 'PMap' (140) {
	{	
		'IMAS','actn',0,"Import As","",
		'RESO','re32',0,"Resolution","",
		'SIZX','re32',1,"X","",
		'SIZY','re32',1,"Y","",
		'SIZZ','re32',1,"Z","",
		'SHDR','actn',0,"Shading", "",
		'SCLE','actn',0,"Scaling", "",
		'METY','actn',0,"Mesh type","",
		'erro','re32',1,"Max Error","",
		'LSEQ','bool',0,"Look for sequenced files","",
	}
};

resource 'GUID' (200)
{
	{
		R_IID_I3DExTerrainFilter,
		R_CLSID_GCGenerator
	}
};

resource 'COMP' (200)
{
	kRID_TerrainFilterFamilyID,
	'GCTF',
	"GC Importer",
	"Ground Control",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (200) {
	{	
		'DESC','s255',0,"Description","",
		'RESO','s255',0,"Resolution","",
		'UFHE','bool',0,"Use File Elevations", "",
		'STAR','re32',1,"Bottom Elevation", "",
		'HEIG','re32',1,"Top Elevation", "",
		'FILT','actn',0,"Filtering","",
		'RENS','re32',1,"Real World North South", "",
		'REEW','re32',1,"Real World East West", "",
	}
};

resource 'GUID' (201)
{
	{
		R_IID_I3DExTerrainFilter,
		R_CLSID_ShaderFilter
	}
};

resource 'COMP' (201)
{
	kRID_TerrainFilterFamilyID,
	'GCSF',
	"GC Shader Filter",
	"Ground Control",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (201) {
	{	
		'SSHA','s255',0,"Shader","",
		'SSHP','actn',0,"Shader Picker","",
		'HEIG','re32',1,"Height", "",
		'OPER','actn',0,"Operator","",
		'PASS','in32',0,"Passes","",
		'EFSH','s255',0,"Effect","",
		'EFPI','actn',0,"Effect Picker","",
		'USHE','bool',0,"Use Existing Height", "",
	}
};

