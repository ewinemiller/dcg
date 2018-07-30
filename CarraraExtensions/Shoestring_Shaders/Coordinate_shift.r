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


resource 'GUID' (152)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Coordinate_Shift
	}
};

resource 'COMP' (152)
{
	kRID_ShaderFamilyID,
	'MDco',
	"Coordinate_Shift",
	"Shoestring Shaders Complex #1",
#if (VERSIONNUMBER < 0x040000)
	 VERSIONNUMBER,
#elif (VERSIONNUMBER >= 0x040000)
	 FIRSTVERSION,
#endif	
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource 'PMap' (152)
{
	{
	'MODE','in32',noFlags,"Offset/Object Mode","",
	'OBNA','s255',0,"Reference Object Name","",
	'XOFF','re32',interpolate,"X Offset","",
	'YOFF','re32',interpolate,"Y Offset","",
	'ZOFF','re32',interpolate,"Z Offset","",
	'GLOX','bool',noFlags,"Global X axis","",
	'GLOY','bool',noFlags,"Global Y axis","",
	'GLOZ','bool',noFlags,"Global Z axis","",
	'LCOX','bool',noFlags,"Local X axis","",
	'LCOY','bool',noFlags,"Local Y axis","",
	'LCOZ','bool',noFlags,"Local Z axis","",	
	'Shd0','comp',interpolate,"Input Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};