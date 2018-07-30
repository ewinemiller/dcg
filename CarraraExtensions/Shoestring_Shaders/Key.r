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


resource 'GUID' (156)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Key
	}
};

resource 'COMP' (156)
{
	kRID_ShaderFamilyID,
	'MDky',
	"Key",
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
  
resource 'PMap' (156)
{
	{
	'DRED','vec2',interpolate,"Red Range","",
	'DGRN','vec2',interpolate,"Green Range","",
	'DBLU','vec2',interpolate,"Blue Range","",
	'MODE','in32',noFlags,"Key Mode","",
	'KCLR','colo',interpolate,"Key Picker","",
	'Shd0','comp',interpolate,"Input Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};