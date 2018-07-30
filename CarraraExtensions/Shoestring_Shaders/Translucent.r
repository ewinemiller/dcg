/*  Shoestring Shaders - plug-in for Carrara
    Copyright (C) 2003  Mark DesMarais

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


resource 'GUID' (144)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Translucent
	}
};

resource 'COMP' (144)
{
	kRID_ShaderFamilyID,
	'MDan',
	"Angle",
	"Shoestring Shaders",
#if (VERSIONNUMBER < 0x040000)
	 VERSIONNUMBER,
#elif (VERSIONNUMBER >= 0x040000)
	 FIRSTVERSION,
#endif	
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource 'PMap' (144)
{
	{
	'CAME','s255',0,"Camera","",
	'LGHT','s255',0,"Light","",
	'INVG','bool',interpolate,"Invert Grayscale","",
	'MODE','in32',noFlags,"Falloff Mode","",
	'ANGL','vec2',interpolate,"Angle Limits","",
	'PMDE','in32',noFlags,"Point Mode","",
	'EASE','vec2',interpolate,"Falloff Ease","",
	}
};