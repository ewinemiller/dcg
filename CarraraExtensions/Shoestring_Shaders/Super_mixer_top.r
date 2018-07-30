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



resource 'GUID' (122)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Super_mixer_top
	}
};

resource 'COMP' (122)
{
	kRID_ShaderFamilyID,
	'MDst',	
	"Super Mixer Complex",
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

resource 'PMap' (122)
{
	{
		'BMDE','in32',noFlags,"Edge Mode","",
		'Ins1','vec2',interpolate,"Influence Shader 1","",
		'Ins2','vec2',interpolate,"Influence Shader 2","",
		'Ins3','vec2',interpolate,"Influence Shader 3","",
		'Ins4','vec2',interpolate,"Influence Shader 4","",
		'Ins5','vec2',interpolate,"Influence Shader 5","",
		'Ins6','vec2',interpolate,"Influence Shader 6","",
		'Ins7','vec2',interpolate,"Influence Shader 7","",
		'Ins8','vec2',interpolate,"Influence Shader 8","",
		'Ins9','vec2',interpolate,"Influence Shader 9","",
		'Ins0','vec2',interpolate,"Influence Shader 10","",
		'SMOV','bool',interpolate,"Falloff","",
		'LINE','bool',interpolate,"Linear Blend","",
		'FLNL','bool',interpolate,"Fill Null","",
		'BLND','in32',interpolate,"% to blend","",
		'INFL','in32',interpolate,"Influence Setup","",
		'INTY','in32',interpolate,"Intensity","",
		'VALU','in32',interpolate,"Value","",
		'PCKR','colo',interpolate, "Color Picker", "",
		'SMIX','bool',interpolate,"Show Mix Shader","",
		'Shd0','comp',interpolate,"Mix Shader","{fmly shdr MskE 126 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd1','comp',interpolate,"Blend Shader","{fmly shdr MskE 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd2','comp',interpolate,"Shader 1","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd3','comp',interpolate,"Shader 2","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd4','comp',interpolate,"Shader 3","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd5','comp',interpolate,"Shader 4","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd6','comp',interpolate,"Shader 5","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd7','comp',interpolate,"Shader 6","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd8','comp',interpolate,"Shader 7","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Shd9','comp',interpolate,"Shader 8","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Sh10','comp',interpolate,"Shader 9","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
		'Sh11','comp',interpolate,"Shader 10","{fmly shdr MskI 127 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};
