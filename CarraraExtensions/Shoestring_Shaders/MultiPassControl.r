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


resource 'GUID' (132)
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiPassControl
	}
};

resource 'COMP' (132)
{
	kRID_ShaderFamilyID,
	'MDmp',
	"MultiPass Control",
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
  
resource 'PMap' (132)
{
	{
	'PAS0','s255',0,"Label 1","",
	'PAS1','s255',0,"Label 2","",
	'PAS2','s255',0,"Label 3","",
	'PAS3','s255',0,"Label 4","",
	'PAS4','s255',0,"Label 5","",
	'PAS5','s255',0,"Label 6","",
	'PAS6','s255',0,"Label 7","",
	'PAS7','s255',0,"Label 8","",
	'PAS8','s255',0,"Label 9","",
	'PAS9','s255',0,"Label 10","",
	'LEVL','in32',noFlags,"Value","",
	'Shd0','comp',interpolate,"Ref Shader","{fmly shdr MskE 126 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};