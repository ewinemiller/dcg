/*  Shader Ops 2 - plug-in for Carrara
    Copyright (C) 2010 Eric Winemiller

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
#include "External3DAPI.r"
#include "Copyright.h"
#include "interfaceids.h"
#include "imageiids.h"


#include "MultiAddDef.h"
#include "MultiMultiplyDef.h"
#include "MultiMaxDef.h"
#include "MultiMinDef.h"
#include "MultiSubtractDef.h"
#include "MultiAccumulateDef.h"

#include "SpaceManglerDef.h"
#include "LightManglerDef.h"
#include "MangleDef.h"

#include "LayersListDef.h"
#include "LayerDef.h"

#include "CameraMappingDef.h"
#include "BlurDef.h"
#include "CropUVDef.h"

#include "AnimatedMixerDef.h"
#include "AnimatedMixerTransitionDef.h"

#if (VERSIONNUMBER >= 0x050000)
#include "CommandIDRanges.h"
#include "DCGCommandIDs.h"
#endif

include "ShaderOps2.rsr";

#define COPYRIGHT "Copyright © 2010 Digital Carvers Guild"
#define SHADER_OPS_2_GROUP "Shader Ops 2"
#define SHADER_OPS_2_LIGHTING_MODEL_GROUP "Shader Ops 2 Lighting Models # 16"
#define SHADER_OPS_2_TOP_MOST_GROUP "Shader Ops 2 # 1"

#define LIGHTMANGLERS "Shader Ops 2 Light Manglers"

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (100) 
{
	kRID_ShaderFamilyID,
	'SOMA',
	"Multi Add",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (100) 
{
	{
		'Laye','cmp#',interpolate, "Add","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (100) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiAdd
	}
};

resource 'COMP' (101) 
{
	kRID_ShaderFamilyID,
	'SOMM',
	"Multi Multiply",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (101) 
{
	{
		'Laye','cmp#',interpolate, "Multiply","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (101) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiMultiply
	}
};

resource 'COMP' (102) 
{
	kRID_ShaderFamilyID,
	'SOMa',
	"Multi Max",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (102) 
{
	{
		'Laye','cmp#',interpolate, "Max","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (102) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiMax
	}
};

resource 'COMP' (103) 
{
	kRID_ShaderFamilyID,
	'SOMi',
	"Multi Min",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (103) 
{
	{
		'Laye','cmp#',interpolate, "Min","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (103) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiMin
	}
};

resource 'COMP' (104) 
{
	kRID_ShaderFamilyID,
	'SOMS',
	"Multi Subtract",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (104) 
{
	{
		'Laye','cmp#',interpolate, "Subtract","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (104) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiSubtract
	}
};

resource 'COMP' (105) 
{
	kRID_ShaderFamilyID,
	'SOSM',
	"Space Mangler",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (105) 
{
	{
		'GLOX','actn',noflags,"Global X","",
		'GLOY','actn',noflags,"Global Y","",
		'GLOZ','actn',noflags,"Global Z","",
		'LOCX','actn',noflags,"Local X","",
		'LOCY','actn',noflags,"Local Y","",
		'LOCZ','actn',noflags,"Local Z","",
		'MAPU','actn',noflags,"U","",
		'MAPV','actn',noflags,"V","",
		'SCGX','re32',interpolate,"Global Scale X","",
		'SCGY','re32',interpolate,"Global Scale Y","",
		'SCGZ','re32',interpolate,"Global Scale Z","",
		'SCLX','re32',interpolate,"Local Scale X","",
		'SCLY','re32',interpolate,"Local Scale Y","",
		'SCLZ','re32',interpolate,"Local Scale Z","",
		'SCMU','re32',interpolate,"Scale U","",
		'SCMV','re32',interpolate,"Scale V","",
		'sh00','comp',interpolate, "Mangle","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (105) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_SpaceMangler
	}
};



resource 'COMP' (106) 
{
	kRID_ShaderFamilyID,
	'SOAC',
	"Multi Accumulate",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (106) 
{
	{
		'Laye','cmp#',interpolate, "Accumulate","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (106) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_MultiAccumulate
	}
};


resource 'COMP' (107) 
{
	kRID_ShaderFamilyID,
	'SOLL',
	"Layers List",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (107) 
{
	{
		'Laye','cmp#',interpolate, "Layer","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (107) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_LayersList
	}
};

resource 'COMP' (108) 
{
	kRID_ShaderFamilyID,
	'SOLa',
	"Layer",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (108) 
{
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'sh01','comp',interpolate, "Opacity Mask","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (108) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_Layer
	}
};

resource 'COMP' (109) 
{
	kRID_ShaderFamilyID,
	'SOSN',
	"Space Mangler",
	SHADER_OPS_2_TOP_MOST_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (109) 
{
	{
		'GLOX','actn',noflags,"Global X","",
		'GLOY','actn',noflags,"Global Y","",
		'GLOZ','actn',noflags,"Global Z","",
		'LOCX','actn',noflags,"Local X","",
		'LOCY','actn',noflags,"Local Y","",
		'LOCZ','actn',noflags,"Local Z","",
		'MAPU','actn',noflags,"U","",
		'MAPV','actn',noflags,"V","",
		'SCGX','re32',interpolate,"Global Scale X","",
		'SCGY','re32',interpolate,"Global Scale Y","",
		'SCGZ','re32',interpolate,"Global Scale Z","",
		'SCLX','re32',interpolate,"Local Scale X","",
		'SCLY','re32',interpolate,"Local Scale Y","",
		'SCLZ','re32',interpolate,"Local Scale Z","",
		'SCMU','re32',interpolate,"Scale U","",
		'SCMV','re32',interpolate,"Scale V","",
		'sh00','comp',interpolate, "Mangle","{fmly shdr MskI 1 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (109) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_SpaceMangler
	}
};


resource 'COMP' (110) 
{
	kRID_ShaderFamilyID,
	'SOAM',
	"Multi Mixer",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (110) 
{
	{
		'sh02','comp',interpolate, "Blender","{fmly shdr Subm 1 Sort 1 MskE 16341 Mini 1 Drop 3 }",
		'Laye','cmp#',interpolate, "Source","{fmly shdr MskE 1087 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (110) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_AnimatedMixer
	}
};

resource 'STR#' (111)
{
	{
		"Open",	// string  1
	}
};

/*resource 'COMP' (111) 
{
	kRID_ShaderFamilyID,
	'SOTR',
	"AM Transition",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (111) 
{
	{
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 1 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (111) 
{
	{
		R_IID_I3DExShader,
		R_CLSID_AnimatedMixerTransition
	}
};*/


resource 'COMP' (338) {
	kRID_ShaderFamilyID,
	'SPLM',
	"Shader Ops 2 Light Mangler",
	SHADER_OPS_2_LIGHTING_MODEL_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (338) {
	{
		'CBAK','bool',noflags,"Calculate Background","",
		'CILL','bool',noflags,"Calculate Illumination","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 19 bnon 0 Subm 1 Sort 1 Mini 1 }",
		'maal','comp',interpolate, "Alpha","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'maam','comp',interpolate, "Ambient Light","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		//'maao','comp',interpolate, "Ambient Occ.","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'maba','comp',interpolate, "Background","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'maca','comp',interpolate, "Caustics","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'madi','comp',interpolate, "Diffuse","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'magi','comp',interpolate, "GI","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'magl','comp',interpolate, "Glow","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'mals','comp',interpolate, "Light Shadow","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'marf','comp',interpolate, "Reflection","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'mare','comp',interpolate, "Refraction","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'masp','comp',interpolate, "Specular","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
		'mass','comp',interpolate, "Subsurface Scattering","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (338) {
{
		R_IID_I3DExShader,
		R_CLSID_LightMangler
}
};

resource 'COMP' (340) {
	kRID_ShaderFamilyID,
	'LMsp',
	"Calculated Specular",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (340) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleSpecular
}
};

resource 'COMP' (341) {
	kRID_ShaderFamilyID,
	'LMdi',
	"Calculated Diffuse",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (341) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleDiffuse
}
};


resource 'COMP' (342) {
	kRID_ShaderFamilyID,
	'LMgl',
	"Calculated Glow",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (342) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleGlow
}
};


resource 'COMP' (343) {
	kRID_ShaderFamilyID,
	'LMam',
	"Calculated Ambient",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (343) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleAmbient
}
};

resource 'COMP' (344) {
	kRID_ShaderFamilyID,
	'LMrf',
	"Calculated Reflection",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (344) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleReflection
}
};

resource 'COMP' (345) {
	kRID_ShaderFamilyID,
	'LMil',
	"Calculated Illumination",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (345) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleIllumination
}
};


resource 'COMP' (346) {
	kRID_ShaderFamilyID,
	'LMco',
	"Calculated Color",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (346) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleColor
}
};

resource 'COMP' (347) {
	kRID_ShaderFamilyID,
	'LMba',
	"Calculated Background",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (347) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleBackground
}
};


resource 'COMP' (348) {
	kRID_ShaderFamilyID,
	'LMls',
	"Calculated Light Shadow",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (348) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleLightShadow
}
};


resource 'COMP' (349) {
	kRID_ShaderFamilyID,
	'LMre',
	"Calculated Refraction",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (349) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleRefraction
}
};


resource 'COMP' (350) {
	kRID_ShaderFamilyID,
	'LMgi',
	"Calculated Global Illumination",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (350) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleGlobalIllumination
}
};


resource 'COMP' (351) {
	kRID_ShaderFamilyID,
	'LMca',
	"Calculated Caustics",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (351) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleCaustics
}
};


resource 'COMP' (352) {
	kRID_ShaderFamilyID,
	'LMss',
	"Calculated Subsurface Scattering",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (352) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleSubsurfaceScattering
}
};


resource 'COMP' (353) {
	kRID_ShaderFamilyID,
	'LMao',
	"Calculated Ambient Occlusion",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (353) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleAmbientOcclusionFactor
}
};

resource 'COMP' (354) {
	kRID_ShaderFamilyID,
	'LMal',
	"Calculated Alpha",
	LIGHTMANGLERS,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'GUID' (354) {
{
		R_IID_I3DExShader,
		R_CLSID_MangleAlpha
}
};



resource 'COMP' (1001) {
	kRID_ShaderFamilyID,
	'SPCM',
	"Shader Ops 2 Camera Mapping",
	"Projection Mapping # 2",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1001) {
	{
		'dire','in32', noflags, "Direction", "",
		'mapt','mqua', noflags,	"Mapping Transform","",
		'igno','bool', noflags,	"Ignore Normals","",
		'uvid','actn', noflags, "UV space Id","",
		'shly','cmp#', interpolate, "Shaders","{ fmly shdr Sort 1 Subm 1 bnon 0 MskE 0 Drop 4 }",
		'reqs','in32', noflags, "Requestor","",
		'MCAM','s255', noflags, "Mapping Camera","",
		'XRAT','in32', noflags, "X Ratio","",
		'YRAT','in32', noflags, "Y Ratio","",
	}
};


resource 'GUID' (1001) {
{
		R_IID_I3DExShader,
		R_CLSID_CameraMapping
}
};

resource 'COMP' (1002) {
	kRID_ShaderFamilyID,
	'SPCm',
	"Camera Mapping",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1002) {
	{
		'dire','in32', noflags, "Direction", "",
		'mapt','mqua', noflags,	"Mapping Transform","",
		'igno','bool', noflags,	"Ignore Normals","",
		'uvid','actn', noflags, "UV space Id","",
		'shly','cmp#', interpolate, "Shaders","{ fmly shdr Sort 1 Subm 1 bnon 0 MskE 0 Drop 4 }",
		'reqs','in32', noflags, "Requestor","",
		'MCAM','s255', noflags, "Mapping Camera","",
		'XRAT','in32', noflags, "X Ratio","",
		'YRAT','in32', noflags, "Y Ratio","",
	}
};


resource 'GUID' (1002) {
{
		R_IID_I3DExShader,
		R_CLSID_CameraMappingInChannel
}
};

resource 'COMP' (304) {
	kRID_ShaderFamilyID,
	'S2BL',
	"Blur",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (304) {
	{	
		'BLAM','in32',interpolate,"Blur Iterations","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (304) {
{
		R_IID_I3DExShader,
		R_CLSID_Blur
}
};

resource 'CPUI' (304) 
{
	304,					// Id of your main part
	1304,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (305) {
	kRID_ShaderFamilyID,
	'S2BM',
	"Blur",
	SHADER_OPS_2_TOP_MOST_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (305) {
	{	
		'BLAM','in32',interpolate,"Blur Iterations","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 1 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (305) {
{
		R_IID_I3DExShader,
		R_CLSID_Blur
}
};

resource 'CPUI' (305) 
{
	304,					// Id of your main part
	1304,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};


resource 'COMP' (306) {
	kRID_ShaderFamilyID,
	'S2UC',
	"Crop UV",
	SHADER_OPS_2_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (306) {
	{	
		'CROU','vec2',noflags,"Crop U","",
		'CROV','vec2',noflags,"Crop V","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskE 1085 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (306) {
{
		R_IID_I3DExShader,
		R_CLSID_CropUV
}
};

resource 'CPUI' (306) 
{
	306,					// Id of your main part
	1306,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'COMP' (307) {
	kRID_ShaderFamilyID,
	'S2UM',
	"Crop UV",
	SHADER_OPS_2_TOP_MOST_GROUP,
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};


resource 'PMap' (307) {
	{	
		'CROU','vec2',noflags,"Crop U","",
		'CROV','vec2',noflags,"Crop V","",
		'sh00','comp',interpolate, "Shader","{fmly shdr MskI 1 Subm 1 Sort 1 Mini 1 Drop 3 }",
	}
};


resource 'GUID' (307) {
{
		R_IID_I3DExShader,
		R_CLSID_CropUV
}
};

resource 'CPUI' (307) 
{
	306,					// Id of your main part
	1306,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};