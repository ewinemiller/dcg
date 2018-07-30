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


resource 'GUID' (120)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Weave
	}
};

resource 'COMP' (120)
{
	kRID_ShaderFamilyID,
	'MDwv',	
	"Weave",
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

resource 'PMap' (120)
{
	{
		'MODE','in32',noflags,"Mode","",
		'CR51','bool',interpolate,"5 1","",
		'CR52','bool',interpolate,"5 2","",
		'CR53','bool',interpolate,"5 3","",
		'CR54','bool',interpolate,"5 4","",
		'CR55','bool',interpolate,"5 5","",
		'CR41','bool',interpolate,"4 1","",
		'CR42','bool',interpolate,"4 2","",
		'CR43','bool',interpolate,"4 3","",
		'CR44','bool',interpolate,"4 4","",
		'CR45','bool',interpolate,"4 5","",
		'CR31','bool',interpolate,"3 1","",
		'CR32','bool',interpolate,"3 2","",
		'CR33','bool',interpolate,"3 3","",
		'CR34','bool',interpolate,"3 4","",
		'CR35','bool',interpolate,"3 5","",
		'CR21','bool',interpolate,"2 1","",
		'CR22','bool',interpolate,"2 2","",
		'CR23','bool',interpolate,"2 3","",
		'CR24','bool',interpolate,"2 4","",
		'CR25','bool',interpolate,"2 5","",
		'CR11','bool',interpolate,"1 1","",
		'CR12','bool',interpolate,"1 2","",
		'CR13','bool',interpolate,"1 3","",
		'CR14','bool',interpolate,"1 4","",
		'CR15','bool',interpolate,"1 5","",
		'WGIN','in32',interpolate,"Wiggle Intensity","",
		'EDGE','in32',interpolate,"Edge Intensity","",
		'ACRS','bool',noFlags,"3-D Across","",
		'ALNG','bool',noFlags,"3-D Along","",
		'ACNT','re32',interpolate,"Across Intensity","",
		'ALNT','re32',interpolate,"Along Intensity","",
		'UBSP','in32',interpolate,"Horizontal Blank","",
		'VBSP','in32',interpolate,"Vertical Blank","",
		'NTLU','in32',interpolate,"Number of Tiles U","",
		'NTLV','in32',interpolate,"Number of Tiles V","",
		'WUo1','in32',interpolate,"Horizontal Width 1","",
		'WUo2','in32',interpolate,"Horizontal Width 2","",
		'WUo3','in32',interpolate,"U Width 3","",
		'WUo4','in32',interpolate,"U Width 4","",
		'WUo5','in32',interpolate,"U Width 5","",
		'WVe1','in32',interpolate,"V Width 1","",
		'WVe2','in32',interpolate,"V Width 2","",
		'WVe3','in32',interpolate,"V Width 3","",
		'WVe4','in32',interpolate,"V Width 4","",
		'WVe5','in32',interpolate,"V Width 5","",
		'MASK','bool',noFlags,"Mask only","",
		'INVM','bool',noFlags,"Invert mask","",
 		'ShdW','comp',interpolate,"Wiggle","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
 		'ShdE','comp',interpolate,"Edge","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd0','comp',interpolate,"Background","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd1','comp',interpolate,"U 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd2','comp',interpolate,"U 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd3','comp',interpolate,"U 3","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd4','comp',interpolate,"U 4","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd5','comp',interpolate,"U 5","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd6','comp',interpolate,"V 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd7','comp',interpolate,"V 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd8','comp',interpolate,"V 3","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd9','comp',interpolate,"V 4","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Sh10','comp',interpolate,"V 5","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'LCOM','bool',noFlags,"Compression Along","",
		'CCOM','bool',noFlags,"Compression Across","",
	}
};
