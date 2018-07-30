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


resource 'GUID' (138)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Gray
	}
};

resource 'GUID' (158)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Gray_top
	}
};

resource 'COMP' (138)
{
	kRID_ShaderFamilyID,
	'MDds',
	"Grayscale-Desat",
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

resource 'COMP' (158)
{
	kRID_ShaderFamilyID,
	'MDdt',
	"Grayscale-Desat",
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
  
resource 'PMap' (138)
{
	{
	'GRVL','re32',interpolate,"Master Level","",
	'DRED','re32',interpolate,"Red Level","",
	'DGRN','re32',interpolate,"Green Level","",
	'DBLU','re32',interpolate,"Blue Level","",
	'MODE','in32',noFlags,"Grayscale Mode","",
	'Shd0','comp',interpolate,"Input Shader","{fmly shdr MskI 15 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};

resource 'PMap' (158)
{
	{
	'GRVL','re32',interpolate,"Master Level","",
	'DRED','re32',interpolate,"Red Level","",
	'DGRN','re32',interpolate,"Green Level","",
	'DBLU','re32',interpolate,"Blue Level","",
	'MODE','in32',noFlags,"Grayscale Mode","",
	'Shd0','comp',interpolate,"Input Shader","{fmly shdr MskI 15 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	'DOCO','bool',noflags,"Modify Color","",
	'DOGL','bool',noflags,"Modify Color","",
	'DOSP','bool',noflags,"Modify Color","",
	'DOTR','bool',noflags,"Modify Color","",
	}
};