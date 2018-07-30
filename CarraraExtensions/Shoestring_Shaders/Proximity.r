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


resource 'GUID' (124)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Proximity
	}
};

resource 'COMP' (124)
{
	kRID_ShaderFamilyID,
	'MDpx',
	"Proximity",
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
  
resource 'PMap' (124)
{
	{
	'OBNA','s255',0,"Object Name","",
	'RNDC','bool',noFlags,"Use shader","",
	'RAND','in32',interpolate,"Rand strength","",
	'PMDE','in32',noFlags,"Point Mode","",
	'LKIX','bool',noFlags,"X axis","",
	'LKIY','bool',noFlags,"Y axis","",
	'LKIZ','bool',noFlags,"Z axis","",
	'LIMT','re32',interpolate,"Limit","",
	'DIST','re32',interpolate,"Distance to fade over","",
	'OFFS','re32',interpolate,"Offset","",
	'SNGL','bool',noFlags,"Single object mode","",
	'Shd0','comp',interpolate,"Randomize","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	}
};