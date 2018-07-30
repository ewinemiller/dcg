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


resource 'GUID' (136)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Iridescent
	}
};

resource 'COMP' (136)
{
	kRID_ShaderFamilyID,
	'MDir',
	"Iridescent",
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
  
resource 'PMap' (136)
{
	{
	'OBNA','s255',0,"Object Name","",
	'COL1',	'colo',	interpolate, "Color One", "",
	'CTR1','re32',interpolate,"Center One","",
	'FLF1','re32',interpolate,"Falloff One","",
	'COL2',	'colo',	interpolate, "Color Two", "",
	'CTR2','re32',interpolate,"Center Two","",
	'FLF2','re32',interpolate,"Falloff Two","",
	'COL3',	'colo',	interpolate, "Color Three", "",
	'CTR3','re32',interpolate,"Center Three","",
	'FLF3','re32',interpolate,"Falloff Three","",
	'COL4',	'colo',	interpolate, "Color Four", "",
	'CTR4','re32',interpolate,"Center Four","",
	'FLF4','re32',interpolate,"Falloff Four","",
	'COL5',	'colo',	interpolate, "Color Five", "",
	'CTR5','re32',interpolate,"Center Five","",
	'FLF5','re32',interpolate,"Falloff Five","",
	'COL6',	'colo',	interpolate, "Color Six", "",
	'CTR6','re32',interpolate,"Center Six","",
	'FLF6','re32',interpolate,"Falloff Six","",
	'COL7',	'colo',	interpolate, "Color Seven", "",
	'CTR7','re32',interpolate,"Center Seven","",
	'FLF7','re32',interpolate,"Falloff Seven","",
	'LRST','bool',interpolate,"Last is rest","",
	'CFLF','bool',interpolate,"Copy falloff","",
	'GRAY','bool',interpolate,"Grayscale","",
	'INVG','bool',interpolate,"Invert Grayscale","",
	'DYAW','re32',interpolate,"Yaw","",
	'DPIT','re32',interpolate,"Pitch","",
	'MODE','in32',noFlags,"Edge Mode","",
	'VMDE','in32',noFlags,"Vector Mode","",
	'Shd0','comp',interpolate,"Edge Control","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};