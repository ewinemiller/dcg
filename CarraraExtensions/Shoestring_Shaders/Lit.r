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


resource 'GUID' (154)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Lit
	}
};

resource 'COMP' (154)
{
	kRID_ShaderFamilyID,
	'MDlt',
	"Lit",
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
  
resource 'PMap' (154)
{
	{
	'LGHT','s255',0,"Light","",
	'INVG','bool',noFlags,"Invert Grayscale","",
	'COLO','bool',noFlags,"Pass color","",
	'SHIN','bool',noFlags,"Shine thru","",
	'ANGL','vec2',interpolate,"Angle","",
	'PMDE','in32',noFlags,"Point Mode","",
	'NMDE','bool',noFlags,"Shine thru","",
	}
};