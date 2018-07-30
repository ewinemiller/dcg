/*  Shader Gel and Strobe - plug-in for Carrara
    Copyright (C) 2003 Michael Clarke

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

#include "ExternalAPI.r"
#include "Copyright.h"
#include "StrobeDef.h"
#include "interfaceids.h"


include "Strobe.rsr";


resource 'GUID' (3000)
{
	{
		R_IID_I3DExShader,
		R_CLSID_MCStrobe
	}
};

resource 'COMP' (3000)
{
	kRID_ShaderFamilyID,
	'STBM',
	"Multi-Channel Strobe",
	"Digital Carvers Guild # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (3000)
{
	{
		'STPS','in32',interpolate,"Number of steps","",
		'SEED','in32',interpolate,"PRN Seed","",
		
		'ROTA','re32',interpolate,"Rotation in this frame","",
		'CLEN','re32',interpolate,"Cycle length","",
		'CPHY','re32',interpolate,"Cycle phase","",
		
		'FCOG','bool',noFlags,"From Cognito","",
		'ICOG','bool',noFlags,"Invert Cognito value","",
		
		'STP0','in32',noFlags,"Step index 0","",
		'STP1','in32',noFlags,"Step index 1","",
		'STP2','in32',noFlags,"Step index 2","",
		'STP3','in32',noFlags,"Step index 3","",
		'STP4','in32',noFlags,"Step index 4","",
		'STP5','in32',noFlags,"Step index 5","",
		'STP6','in32',noFlags,"Step index 6","",
		'STP7','in32',noFlags,"Step index 7","",
		'STP8','in32',noFlags,"Step index 8","",
		'STP9','in32',noFlags,"Step index 9","",
		'STPA','in32',noFlags,"Step index A","",
		'STPB','in32',noFlags,"Step index B","",

		'SH10','re32',interpolate,"Shader 1 Strength 0","",
		'SH11','re32',interpolate,"Shader 1 Strength 1","",
		'SH12','re32',interpolate,"Shader 1 Strength 2","",
		'SH13','re32',interpolate,"Shader 1 Strength 3","",
		'SH14','re32',interpolate,"Shader 1 Strength 4","",
		'SH15','re32',interpolate,"Shader 1 Strength 5","",
		'SH16','re32',interpolate,"Shader 1 Strength 6","",
		'SH17','re32',interpolate,"Shader 1 Strength 7","",
		'SH18','re32',interpolate,"Shader 1 Strength 8","",
		'SH19','re32',interpolate,"Shader 1 Strength 9","",
		'SH1A','re32',interpolate,"Shader 1 Strength A","",
		'SH1B','re32',interpolate,"Shader 1 Strength B","",

		'SH20','re32',interpolate,"Shader 2 Strength 0","",
		'SH21','re32',interpolate,"Shader 2 Strength 1","",
		'SH22','re32',interpolate,"Shader 2 Strength 2","",
		'SH23','re32',interpolate,"Shader 2 Strength 3","",
		'SH24','re32',interpolate,"Shader 2 Strength 4","",
		'SH25','re32',interpolate,"Shader 2 Strength 5","",
		'SH26','re32',interpolate,"Shader 2 Strength 6","",
		'SH27','re32',interpolate,"Shader 2 Strength 7","",
		'SH28','re32',interpolate,"Shader 2 Strength 8","",
		'SH29','re32',interpolate,"Shader 2 Strength 9","",
		'SH2A','re32',interpolate,"Shader 2 Strength A","",
		'SH2B','re32',interpolate,"Shader 2 Strength B","",

		'SH30','re32',interpolate,"Shader 3 Strength 0","",
		'SH31','re32',interpolate,"Shader 3 Strength 1","",
		'SH32','re32',interpolate,"Shader 3 Strength 2","",
		'SH33','re32',interpolate,"Shader 3 Strength 3","",
		'SH34','re32',interpolate,"Shader 3 Strength 4","",
		'SH35','re32',interpolate,"Shader 3 Strength 5","",
		'SH36','re32',interpolate,"Shader 3 Strength 6","",
		'SH37','re32',interpolate,"Shader 3 Strength 7","",
		'SH38','re32',interpolate,"Shader 3 Strength 8","",
		'SH39','re32',interpolate,"Shader 3 Strength 9","",
		'SH3A','re32',interpolate,"Shader 3 Strength A","",
		'SH3B','re32',interpolate,"Shader 3 Strength B","",

		'EFX0','in32',noFlags,"Effect 0","",
		'EFX1','in32',noFlags,"Effect 1","",
		'EFX2','in32',noFlags,"Effect 2","",
		'EFX3','in32',noFlags,"Effect 3","",
		'EFX4','in32',noFlags,"Effect 4","",
		'EFX5','in32',noFlags,"Effect 5","",
		'EFX6','in32',noFlags,"Effect 6","",
		'EFX7','in32',noFlags,"Effect 7","",
		'EFX8','in32',noFlags,"Effect 8","",
		'EFX9','in32',noFlags,"Effect 9","",
		'EFXA','in32',noFlags,"Effect A","",
		'EFXB','in32',noFlags,"Effect B","",

		'STRB','s255',noFlags,"Strobe pattern","",

		'Shd0','comp',noFlags,"Shader 1","{fmly shdr MskI 15 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Shd1','comp',noFlags,"Shader 2","{fmly shdr MskI 15 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Shd2','comp',noFlags,"Shader 3","{fmly shdr MskI 15 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};


resource 'CPUI' (3000) 
{
	3000,					// Id of your main part
	3001,					// Id of your mini-part
	15,					// Style
	kParamsBeforeChildren,	                // Where Param part is shown
	0					// Is Collapsable 
};

resource 'GUID' (3005)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Strobe
	}
};

resource 'COMP' (3005)
{
	kRID_ShaderFamilyID,
	'STBS',
	"Strobe",
	"Digital Carvers Guild",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (3005)
{
	{
		'STPS','in32',interpolate,"Number of steps","",
		'SEED','in32',interpolate,"PRN Seed","",
		
		'ROTA','re32',interpolate,"Rotation in this frame","",
		'CLEN','re32',interpolate,"Cycle length","",
		'CPHY','re32',interpolate,"Cycle phase","",
		
		'FCOG','bool',noFlags,"From Cognito","",
		'ICOG','bool',noFlags,"Invert Cognito value","",
		
		'STP0','in32',noFlags,"Step index 0","",
		'STP1','in32',noFlags,"Step index 1","",
		'STP2','in32',noFlags,"Step index 2","",
		'STP3','in32',noFlags,"Step index 3","",
		'STP4','in32',noFlags,"Step index 4","",
		'STP5','in32',noFlags,"Step index 5","",
		'STP6','in32',noFlags,"Step index 6","",
		'STP7','in32',noFlags,"Step index 7","",
		'STP8','in32',noFlags,"Step index 8","",
		'STP9','in32',noFlags,"Step index 9","",
		'STPA','in32',noFlags,"Step index A","",
		'STPB','in32',noFlags,"Step index B","",

		'SH10','re32',interpolate,"Shader 1 Strength 0","",
		'SH11','re32',interpolate,"Shader 1 Strength 1","",
		'SH12','re32',interpolate,"Shader 1 Strength 2","",
		'SH13','re32',interpolate,"Shader 1 Strength 3","",
		'SH14','re32',interpolate,"Shader 1 Strength 4","",
		'SH15','re32',interpolate,"Shader 1 Strength 5","",
		'SH16','re32',interpolate,"Shader 1 Strength 6","",
		'SH17','re32',interpolate,"Shader 1 Strength 7","",
		'SH18','re32',interpolate,"Shader 1 Strength 8","",
		'SH19','re32',interpolate,"Shader 1 Strength 9","",
		'SH1A','re32',interpolate,"Shader 1 Strength A","",
		'SH1B','re32',interpolate,"Shader 1 Strength B","",

		'SH20','re32',interpolate,"Shader 2 Strength 0","",
		'SH21','re32',interpolate,"Shader 2 Strength 1","",
		'SH22','re32',interpolate,"Shader 2 Strength 2","",
		'SH23','re32',interpolate,"Shader 2 Strength 3","",
		'SH24','re32',interpolate,"Shader 2 Strength 4","",
		'SH25','re32',interpolate,"Shader 2 Strength 5","",
		'SH26','re32',interpolate,"Shader 2 Strength 6","",
		'SH27','re32',interpolate,"Shader 2 Strength 7","",
		'SH28','re32',interpolate,"Shader 2 Strength 8","",
		'SH29','re32',interpolate,"Shader 2 Strength 9","",
		'SH2A','re32',interpolate,"Shader 2 Strength A","",
		'SH2B','re32',interpolate,"Shader 2 Strength B","",

		'SH30','re32',interpolate,"Shader 3 Strength 0","",
		'SH31','re32',interpolate,"Shader 3 Strength 1","",
		'SH32','re32',interpolate,"Shader 3 Strength 2","",
		'SH33','re32',interpolate,"Shader 3 Strength 3","",
		'SH34','re32',interpolate,"Shader 3 Strength 4","",
		'SH35','re32',interpolate,"Shader 3 Strength 5","",
		'SH36','re32',interpolate,"Shader 3 Strength 6","",
		'SH37','re32',interpolate,"Shader 3 Strength 7","",
		'SH38','re32',interpolate,"Shader 3 Strength 8","",
		'SH39','re32',interpolate,"Shader 3 Strength 9","",
		'SH3A','re32',interpolate,"Shader 3 Strength A","",
		'SH3B','re32',interpolate,"Shader 3 Strength B","",

		'EFX0','in32',noFlags,"Effect 0","",
		'EFX1','in32',noFlags,"Effect 1","",
		'EFX2','in32',noFlags,"Effect 2","",
		'EFX3','in32',noFlags,"Effect 3","",
		'EFX4','in32',noFlags,"Effect 4","",
		'EFX5','in32',noFlags,"Effect 5","",
		'EFX6','in32',noFlags,"Effect 6","",
		'EFX7','in32',noFlags,"Effect 7","",
		'EFX8','in32',noFlags,"Effect 8","",
		'EFX9','in32',noFlags,"Effect 9","",
		'EFXA','in32',noFlags,"Effect A","",
		'EFXB','in32',noFlags,"Effect B","",

		'STRB','s255',noFlags,"Strobe pattern","",

		'Shd0','comp',interpolate,"Shader 1","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Shd1','comp',interpolate,"Shader 2","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Shd2','comp',interpolate,"Shader 3","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};


resource 'CPUI' (3005) 
{
	3005,					// Id of your main part
	3001,					// Id of your mini-part
	15,					// Style
	kParamsBeforeChildren,	                // Where Param part is shown
	0					// Is Collapsable 
};
