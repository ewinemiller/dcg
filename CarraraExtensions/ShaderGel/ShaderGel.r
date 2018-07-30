/*  Shader Gel and Strobe - plug-in for Carrara
    Copyright (C) 2003 Michael Clarke

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
#include "ShaderGelDef.h"
#include "interfaceids.h"

include "ShaderGel.rsr";


#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

#include "Strobe.r"

resource 'COMP' (4000)
{
	kRID_LightsourceGelFamilyID,
	'SHGL',
	"ShaderGel",
	"Digital Carvers Guild",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (4000)
{
	{	
		'OFSX', 're32', interpolate, "X Offset", "",
		'OFSY', 're32', interpolate, "Y Offset", "",
		'SCLX', 're32', interpolate, "X Scale", "",
		'SCLY', 're32', interpolate, "Y Scale", "",
		'RPTX', 're32', interpolate, "X Repeat", "",
		'RPTY', 're32', interpolate, "Y Repeat", "",

		'ROTA', 're32', interpolate, "Rotation", "",

		'RDMP', 'bool', noFlags,     "Radial Mapping?", "",

		'ROTC', 'bool', noFlags,     "Rotation from Cognito?", "",
		'OFXC', 'bool', noFlags,     "Offset X from Cognito?", "",
		'OFYC', 'bool', noFlags,     "Offset Y from Cognito?", "",
		'SCXC', 'bool', noFlags,     "Scale X from Cognito?", "",
		'SCYC', 'bool', noFlags,     "Scale Y from Cognito?", "",
		'RPXC', 'bool', noFlags,     "Repeat X from Cognito?", "",
		'RPYC', 'bool', noFlags,     "Repeat Y from Cognito?", "",

		'SHDR', 's255', noFlags,     "Name of Shader","",

		'SHPU', 'actn', noFlags,     "Shader Pop-up action","",	
	}
};

resource 'GUID' (4000)
{
	{
		R_IID_I3DExLightsourceGel,
		R_CLSID_ShaderGel
	} 
};


