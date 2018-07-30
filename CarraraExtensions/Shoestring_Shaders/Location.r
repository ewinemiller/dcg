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


resource 'GUID' (134)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Location
	}
};

resource 'COMP' (134)
{
	kRID_ShaderFamilyID,
	'MDlo',
	"Location",
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
  
resource 'PMap' (134)
{
	{
	'XBD1','re32',interpolate,"X boundary 1","",
	'XBD2','re32',interpolate,"X boundary 2","",
	'XAXS','bool',noFlags,"Use Axis X","",
	'GRLX','in32',noFlags,"Greater or Less X","",
	'YBD1','re32',interpolate,"Y boundary 1","",
	'YBD2','re32',interpolate,"Y boundary 2","",
	'YAXS','bool',noFlags,"Use Axis Y","",
	'GRLY','in32',noFlags,"Greater or Less Y","",
	'ZBD1','re32',interpolate,"Z bound 1","",
	'ZBD2','re32',interpolate,"Z bound 2","",
	'ZAXS','bool',noFlags,"Use Axis Z","",
	'GRLZ','in32',noFlags,"Greater or Less Z","",
	'MODE','in32',noFlags,"And/or mode","",
	'CORD','in32',noFlags,"Local/Global mode","",
	'RNDC','bool',noFlags,"Use shader","",
	'RAND','in32',interpolate,"Rand strength","",
	'Shd0','comp',interpolate,"Randomize","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};