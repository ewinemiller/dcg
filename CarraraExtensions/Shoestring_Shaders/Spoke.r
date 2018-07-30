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


resource 'GUID' (146)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Spoke
	}
};

resource 'COMP' (146)
{
	kRID_ShaderFamilyID,
	'MDsk',
	"Spokes",
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
  
resource 'PMap' (146)
{
	{
	'ULOC','re32',interpolate,"Center U location","",
	'VLOC','re32',interpolate,"Center V location","",
	'HGHT','re32',interpolate,"Height","",
	'WIDT','re32',interpolate,"Width","",
	'NUMB','in32',interpolate,"Number","",
	'OFST','in32',interpolate,"Offset","",
	'TAPE','re32',interpolate,"Taper","",
	'CIFL','re32',interpolate,"Circular Falloff","",
	'RDST','re32',interpolate,"Radial Start","",
	'RDEN','re32',interpolate,"Radial End","",
	'STFL','re32',interpolate,"Start Radial Falloff","",
	'ENFL','re32',interpolate,"End Radial Falloff","",
	'COWD','bool',noflags,"Constant Width","",
	'FLCN','bool',noflags,"Fill Center","",
	}
};