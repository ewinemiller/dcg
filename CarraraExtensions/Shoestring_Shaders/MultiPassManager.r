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



resource 'GUID' (142)
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiPassManager
	}
};

resource 'COMP' (142)
{
	kRID_ShaderFamilyID,
	'MDmm',	
	"MultiPass Manager",
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

resource 'PMap' (142)
{
	{
		'PAS0','s255',0,"Pass 1","",
		'PAS1','s255',0,"Pass 2","",
		'PAS2','s255',0,"Pass 3","",
		'PAS3','s255',0,"Pass 4","",
		'PAS4','s255',0,"Pass 5","",
		'PAS5','s255',0,"Pass 6","",
		'PAS6','s255',0,"Pass 7","",
		'PAS7','s255',0,"Pass 8","",
		'PAS8','s255',0,"Pass 9","",
		'PAS9','s255',0,"Pass 10","",
		'Shd0','comp',interpolate,"Pass Control","{fmly shdr MskE 126 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd1','comp',interpolate,"Pass 1","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd2','comp',interpolate,"Pass 2","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd3','comp',interpolate,"Pass 3","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd4','comp',interpolate,"Pass 4","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd5','comp',interpolate,"Pass 5","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd6','comp',interpolate,"Pass 6","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd7','comp',interpolate,"Pass 7","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd8','comp',interpolate,"Pass 8","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd9','comp',interpolate,"Pass 9","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Sh10','comp',interpolate,"Pass 10","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};
