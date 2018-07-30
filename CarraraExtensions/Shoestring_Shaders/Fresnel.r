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

#define MAC
#ifdef MAC
#include "ExternalAPI.r"
#include "Copyright.h"
#include "FresnelDef.h"
#include "interfaceids.h"
#endif
#ifndef qUsingResourceLinker
#ifdef MAC
//include "Fresnel.rsrc";
#endif
#endif




resource 'GUID' (130)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Fresnel
	}
};

resource 'GUID' (170)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Fresnel2
	}
};

resource 'COMP' (130)
{
	kRID_ShaderFamilyID,
	'MDfr',
	"Fresnel",
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
 
resource 'COMP' (170)
{
	kRID_ShaderFamilyID,
	'MDfn',
	"Easy Fresnel",
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
 
resource 'PMap' (130)
{
	{
	'OBNA','s255', 0,          "Object Name"     ,"",
	'MODE','in32', noFlags,    "Mode","",
	'Shd0','comp', interpolate,"Edge Control","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	'N1Re','re32', interpolate, "n1r"             ,"",
	'N1Im','re32', interpolate, "n1i"             ,"",
	'N2Re','re32', interpolate, "n2r"             ,"",
	'N2Im','re32', interpolate, "n2i"             ,"",
	'N1Mr','re32', interpolate, "mu1r"            ,"",
	'N2Mr','re32', interpolate, "mu2r"            ,"",
	'PerP','re32', interpolate, "Pper"            ,"",
	'TRMD','in32', noFlags    , "TRMode"          ,"",
	'N2rs','re32', interpolate, "n2rslider"       ,"",
	'N2is','re32', interpolate, "n2islider"       ,"",
	'N2Ms','re32', interpolate, "mu2rslider"      ,"",
	'WAVE','re32', interpolate, "lambdaslider"    ,"",
	'Diss','re32', interpolate, "disp      "      ,"",
	'N2RD','re32', interpolate, "n2rd"            ,"",
	'N2ID','re32', interpolate, "n2id"            ,"",
	'PRS1','in32', noflags,		"Preset n1"		  ,"",
	'PRS2','in32', noflags,		"Preset n2"		  ,"",
	'ILUM','in32', noflags,		"Preset illum"    ,"",
	'COLO','bool', interpolate, "Color Object"    ,"",
	'SPEC','colo', interpolate, "Spectral Color"  ,"",
	'RedS','re32', interpolate, "Spectral Red"    ,"",
	'GrnS','re32', interpolate, "Spectral Green"  ,"",
	'BluS','re32', interpolate, "Spectral Blue"   ,"",
	'ILLU','colo', interpolate, "illuminant Color","",
	'ILLK','re32', interpolate, "illuminant"      ,"",
	'xWHT','re32', interpolate, "xWhite"          ,"",
	'yWHT','re32', interpolate, "yWhite    "      ,""
	}
};

resource 'PMap' (170)
{
	{
	'OBNA','s255', 0,          "Object Name"     ,"",
	'MODE','in32', noFlags,    "Mode","",
	'Shd0','comp', interpolate,"Edge Control","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}", // = 1024+32+16+8+4+2+1
	'N1Re','re32', interpolate, "n1r"             ,"",
	'N1Im','re32', interpolate, "n1i"             ,"",
	'N2Re','re32', interpolate, "n2r"             ,"",
	'N2Im','re32', interpolate, "n2i"             ,"",
	'N1Mr','re32', interpolate, "mu1r"            ,"",
	'N2Mr','re32', interpolate, "mu2r"            ,"",
	'PerP','re32', interpolate, "Pper"            ,"",
	'TRMD','in32', noFlags    , "TRMode"          ,"",
	'N2rs','re32', interpolate, "n2rslider"       ,"",
	'N2is','re32', interpolate, "n2islider"       ,"",
	'N2Ms','re32', interpolate, "mu2rslider"      ,"",
	'WAVE','re32', interpolate, "lambdaslider"    ,"",
	'Diss','re32', interpolate, "disp      "      ,"",
	'N2RD','re32', interpolate, "n2rd"            ,"",
	'N2ID','re32', interpolate, "n2id"            ,"",
	'PRS1','in32', noflags,		"Preset n1"		  ,"",
	'PRS2','in32', noflags,		"Preset n2"		  ,"",
	'ILUM','in32', noflags,		"Preset illum"    ,"",
	'COLO','bool', interpolate, "Color Object"    ,"",
	'SPEC','colo', interpolate, "Spectral Color"  ,"",
	'RedS','re32', interpolate, "Spectral Red"    ,"",
	'GrnS','re32', interpolate, "Spectral Green"  ,"",
	'BluS','re32', interpolate, "Spectral Blue"   ,"",
	'ILLU','colo', interpolate, "illuminant Color","",
	'ILLK','re32', interpolate, "illuminant"      ,"",
	'xWHT','re32', interpolate, "xWhite"          ,"",
	'yWHT','re32', interpolate, "yWhite    "      ,""
	}
};