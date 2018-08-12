/*  Anything Goos - plug-in for LightWave
Copyright (C) 2009 Eric Winemiller

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
#include <lwserver.h>
#include <lwnodes.h>
#include <lwsurf.h>
#include <lwxpanel.h>
#include <lwmath.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lwmeshes.h>
#include <lwmtutil.h>
#include <lwvparm.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "EdgeTree.h"
#include "WorkingStructs.h"
#include "MathHelper.h"

LWMTUtilFuncs		*mtutil;

/***************************************************************
*
*  Function globals.
*
***************************************************************/
static  LWNodeInputFuncs    *inputfuncs;
static  LWNodeOutputFuncs   *outputfuncs;
static  LWNodeFuncs         *nodefuncs;
static	LWNodeUtilityFuncs  *nodeutilfuncs;
static  LWXPanelFuncs       *xpanf;
static  LWInstUpdate        *lwupdate;
static  LWItemInfo          *iteminfo;
static  LWObjectInfo        *objinfo;
static  LWEnvelopeFuncs		*envfuncs;
static  LWVParmFuncs		*vparmfuncs;


/***************************************************************
*
*  XPanel control id's.
*
***************************************************************/
enum { AG_baseColor = 0x8601, AG_goosColor, AG_noise, AG_opacity, AG_perimeter, AG_inner, AG_outer, AG_edgeangle, AG_falloff, AG_distance
, AG_blend, AG_invert, AG_innerpreview, AG_outerpreview, AG_perimeterpreview, AG_bumpamplitude, AG_baseSpecular, AG_goosSpecular
, AG_baseGlossiness, AG_goosGlossiness, AG_baseLuminosity, AG_goosLuminosity, AG_baseReflection, AG_goosReflection
, AG_baseTransparency, AG_goosTransparency, AG_baseRefractionIndex, AG_goosRefractionIndex, AG_baseTranslucency, AG_goosTranslucency
, AG_baseDiffuse, AG_goosDiffuse, AG_includeAlpha, AG_includeAlphaBlend//, AG_falloffangle
};
enum { AG_FalloffAbsolute, AG_FalloffRadial, AG_FalloffLinear, AG_FalloffInvRadial };
enum { AG_BlendAdditive, AG_BlendSubtractive, AG_BlendMultiple, AG_BlendMax, AG_BlendMin };
static const char *FallOffSelections[] = {"Absolute", "Radial", "Linear", "Inv Radial",NULL};		
static const char *BlendSelections[] = {"Normal", "Additive", "Subtractive", "Multiply",
	"Screen", "Darken", "Lighten", "Difference", "Negative", "Color Dodge", "Color Burn", "Red", "Green", "Blue", NULL};

static const char *AlphaBlendSelections[] = {"Additive", "Subtractive", "Multiply",
	"Max", "Min", NULL};

/***************************************************************
*
*  Block id's for saving and loading parameter values.
*
***************************************************************/
#define	ID_AG_BASE_COLOR	LWID_( 'B','C','O','L' )
#define	ID_AG_GOOS_COLOR	LWID_( 'G','C','O','L' )
#define	ID_AG_NOISE			LWID_( 'N','O','I','S' )
#define	ID_AG_OPACITY		LWID_( 'O','P','A','C' )
#define	ID_AG_PERIMETER		LWID_( 'P','E','R','I' )
#define	ID_AG_INNER			LWID_( 'I','N','N','E' )
#define	ID_AG_OUTER			LWID_( 'O','U','T','E' )
#define	ID_AG_EDGEANGLE		LWID_( 'E','D','G','E' )
#define	ID_AG_FALLOFF		LWID_( 'F','A','L','L' )
#define	ID_AG_DIST			LWID_( 'D','I','S','T' )
#define	ID_AG_BLEND			LWID_( 'B','L','E','N' )
#define	ID_AG_INVERT		LWID_( 'I','N','V','E' )
#define	ID_AG_BUMP_AMPLITUDE LWID_( 'B','A','M','P' )
#define	ID_AG_BASE_SPECULAR	LWID_( 'B','S','P','E' )
#define	ID_AG_GOOS_SPECULAR	LWID_( 'G','S','P','E' )
#define	ID_AG_BASE_GLOSSINESS	LWID_( 'B','G','L','O' )
#define	ID_AG_GOOS_GLOSSINESS	LWID_( 'G','G','L','O' )
#define	ID_AG_BASE_LUMINOSITY	LWID_( 'B','L','U','M' )
#define	ID_AG_GOOS_LUMINOSITY	LWID_( 'G','L','U','M' )
#define	ID_AG_BASE_REFLECTION	LWID_( 'B','R','E','F' )
#define	ID_AG_GOOS_REFLECTION	LWID_( 'G','R','E','F' )
#define	ID_AG_BASE_TRANSPARENCY	LWID_( 'B','T','R','A' )
#define	ID_AG_GOOS_TRANSPARENCY	LWID_( 'G','T','R','R' )
#define	ID_AG_BASE_REFRACTIONINDEX	LWID_( 'B','R','F','I' )
#define	ID_AG_GOOS_REFRACTIONINDEX	LWID_( 'G','R','F','I' )
#define	ID_AG_BASE_TRANSLUCENCY	LWID_( 'B','T','L','U' )
#define	ID_AG_GOOS_TRANSLUCENCY	LWID_( 'G','T','L','U' )
#define	ID_AG_BASE_DIFFUSE	LWID_( 'B','D','I','F' )
#define	ID_AG_GOOS_DIFFUSE	LWID_( 'G','D','I','F' )
#define	ID_AG_INCLUDE_ALPHA	LWID_( 'I','N','A','L' )
#define	ID_AG_INCLUDE_ALPHA_BLEND	LWID_( 'I','N','B','L' )
#define	ID_AG_FALLOFF_ANGLE	LWID_( 'C','A','N','G' )

enum { AGB_baseColor, AGB_goosColor, AGB_noise, AGB_opacity, AGB_perimeter, AGB_inner, AGB_outer, AGB_edgeangle, AGB_falloff
	, AGB_distance, AGB_blend, AGB_invert, AGB_bumpamplitude, AGB_baseSpecular, AGB_goosSpecular, AGB_baseGlossiness, AGB_goosGlossiness
	, AGB_baseLuminosity, AGB_goosLuminosity, AGB_baseReflection, AGB_goosReflection, AGB_baseTransparency, AGB_goosTransparency
	, AGB_baseRefractionIndex, AGB_goosRefractionIndex, AGB_baseTranslucency, AGB_goosTranslucency, AGB_baseDiffuse, AGB_goosDiffuse
	, AGB_includeAlpha, AGB_includeAlphaBlend//, AGB_FalloffAngle
};

#define FIELD_NAME_BASE_COLOR "Base Color"
#define FIELD_NAME_GOOS_COLOR "Goos Color"
#define FIELD_NAME_NOISE "Noise" 
#define FIELD_NAME_OPACITY "Opacity"
#define FIELD_NAME_PERIMETER_EDGES "Perimeter Edges"
#define FIELD_NAME_INNER_EDGES "Inner Edges"
#define FIELD_NAME_OUTER_EDGES "Outer Edges"
#define FIELD_NAME_EDGE_ANGLE "Edge Angle"
#define FIELD_NAME_FALLOFF "Falloff"
#define FIELD_NAME_DISTANCE "Distance"
#define FIELD_NAME_BLEND "Blend"
#define FIELD_NAME_INVERT "Invert"
#define FIELD_NAME_BUMP_AMPLITUDE "Bump Amplitude"
#define FIELD_NAME_BUMP_AMPLITUDE_SHORT "BumpAmp"
#define FIELD_NAME_BASE_SPECULAR "Base Specular"
#define FIELD_NAME_GOOS_SPECULAR "Goos Specular"
#define FIELD_NAME_BASE_GLOSSINESS "Base Glossiness"
#define FIELD_NAME_GOOS_GLOSSINESS "Goos Glossiness"
#define FIELD_NAME_BASE_LUMINOSITY "Base Luminosity"
#define FIELD_NAME_GOOS_LUMINOSITY "Goos Luminosity"
#define FIELD_NAME_BASE_REFLECTION "Base Reflection"
#define FIELD_NAME_GOOS_REFLECTION "Goos Reflection"
#define FIELD_NAME_BASE_TRANSPARENCY "Base Transparency"
#define FIELD_NAME_GOOS_TRANSPARENCY "Goos Transparency"
#define FIELD_NAME_BASE_REFRACTIONINDEX "Base Refraction Index"
#define FIELD_NAME_GOOS_REFRACTIONINDEX "Goos Refraction Index"
#define FIELD_NAME_BASE_TRANSLUCENCY "Base Translucency"
#define FIELD_NAME_GOOS_TRANSLUCENCY "Goos Translucency"
#define FIELD_NAME_BASE_DIFFUSE "Base Diffuse"
#define FIELD_NAME_GOOS_DIFFUSE "Goos Diffuse"
#define FIELD_NAME_INCLUDE_ALPHA "Include Alpha"
#define FIELD_NAME_INCLUDE_ALPHA_BLEND "Include Alpha Blend"
#define FIELD_NAME_FALLOFF_ANGLE "Face Away Limit"

static char *fieldNames[] = {
		FIELD_NAME_BASE_COLOR,
		FIELD_NAME_GOOS_COLOR,
		FIELD_NAME_NOISE,
		FIELD_NAME_OPACITY,
		FIELD_NAME_PERIMETER_EDGES,
		FIELD_NAME_INNER_EDGES,
		FIELD_NAME_OUTER_EDGES,
		FIELD_NAME_EDGE_ANGLE,
		FIELD_NAME_FALLOFF,
		FIELD_NAME_DISTANCE,
		FIELD_NAME_BLEND,
		FIELD_NAME_INVERT,
		FIELD_NAME_BUMP_AMPLITUDE,
		FIELD_NAME_BASE_SPECULAR,
		FIELD_NAME_GOOS_SPECULAR,
		FIELD_NAME_BASE_GLOSSINESS,
		FIELD_NAME_GOOS_GLOSSINESS,
		FIELD_NAME_BASE_LUMINOSITY,
		FIELD_NAME_GOOS_LUMINOSITY,
		FIELD_NAME_BASE_REFLECTION,
		FIELD_NAME_GOOS_REFLECTION,
		FIELD_NAME_BASE_TRANSPARENCY,
		FIELD_NAME_GOOS_TRANSPARENCY,
		FIELD_NAME_BASE_REFRACTIONINDEX,
		FIELD_NAME_GOOS_REFRACTIONINDEX,
		FIELD_NAME_BASE_TRANSLUCENCY,
		FIELD_NAME_GOOS_TRANSLUCENCY,
		FIELD_NAME_BASE_DIFFUSE,
		FIELD_NAME_GOOS_DIFFUSE,
		FIELD_NAME_INCLUDE_ALPHA,
		FIELD_NAME_INCLUDE_ALPHA_BLEND,
		FIELD_NAME_FALLOFF_ANGLE};

static LWBlockIdent AG_Block[] = {
	ID_AG_BASE_COLOR,			FIELD_NAME_BASE_COLOR,
	ID_AG_GOOS_COLOR,			FIELD_NAME_GOOS_COLOR,
	ID_AG_NOISE,				FIELD_NAME_NOISE,
	ID_AG_OPACITY,				FIELD_NAME_OPACITY,
	ID_AG_PERIMETER,			FIELD_NAME_PERIMETER_EDGES,
	ID_AG_INNER,				FIELD_NAME_INNER_EDGES,
	ID_AG_OUTER,				FIELD_NAME_OUTER_EDGES,
	ID_AG_EDGEANGLE,			FIELD_NAME_EDGE_ANGLE,
	ID_AG_FALLOFF,				FIELD_NAME_FALLOFF,
	ID_AG_DIST,					FIELD_NAME_DISTANCE,
	ID_AG_BLEND,				FIELD_NAME_BLEND,
	ID_AG_INVERT,				FIELD_NAME_INVERT,
	ID_AG_BUMP_AMPLITUDE,		FIELD_NAME_BUMP_AMPLITUDE,
	ID_AG_BASE_SPECULAR,		FIELD_NAME_BASE_SPECULAR,
	ID_AG_GOOS_SPECULAR,		FIELD_NAME_GOOS_SPECULAR,
	ID_AG_BASE_GLOSSINESS,		FIELD_NAME_BASE_GLOSSINESS,
	ID_AG_GOOS_GLOSSINESS,		FIELD_NAME_GOOS_GLOSSINESS,
	ID_AG_BASE_LUMINOSITY,		FIELD_NAME_BASE_LUMINOSITY,
	ID_AG_GOOS_LUMINOSITY,		FIELD_NAME_GOOS_LUMINOSITY,
	ID_AG_BASE_REFLECTION,		FIELD_NAME_BASE_REFLECTION,
	ID_AG_GOOS_REFLECTION,		FIELD_NAME_GOOS_REFLECTION,
	ID_AG_BASE_TRANSPARENCY,		FIELD_NAME_BASE_TRANSPARENCY,
	ID_AG_GOOS_TRANSPARENCY,		FIELD_NAME_GOOS_TRANSPARENCY,
	ID_AG_BASE_REFRACTIONINDEX,		FIELD_NAME_BASE_REFRACTIONINDEX,
	ID_AG_GOOS_REFRACTIONINDEX,		FIELD_NAME_GOOS_REFRACTIONINDEX,
	ID_AG_BASE_TRANSLUCENCY,		FIELD_NAME_BASE_TRANSLUCENCY,
	ID_AG_GOOS_TRANSLUCENCY,		FIELD_NAME_GOOS_TRANSLUCENCY,
	ID_AG_BASE_DIFFUSE,		FIELD_NAME_BASE_DIFFUSE,
	ID_AG_GOOS_DIFFUSE,		FIELD_NAME_GOOS_DIFFUSE,
	ID_AG_INCLUDE_ALPHA,		FIELD_NAME_INCLUDE_ALPHA,
	ID_AG_INCLUDE_ALPHA_BLEND,		FIELD_NAME_INCLUDE_ALPHA_BLEND,
	ID_AG_FALLOFF_ANGLE, FIELD_NAME_FALLOFF_ANGLE,
	0
};


/***************************************************************
*
*  Instance structure definition.
*
***************************************************************/
typedef struct AnythingGoos_t {

	LWXPanelID		panel;              // The id of the xpanel user interface
	NodeID			node;               // The id of the node this instance is applied to

	LWDVector baseColor, goosColor;
	double noise, edgeAngle, distance, opacity, bumpAmplitude, baseSpecular, goosSpecular, baseGlossiness, goosGlossiness
		, baseLuminosity, goosLuminosity, baseReflection, goosReflection, baseTransparency, goosTransparency
		, baseRefractionIndex, goosRefractionIndex, baseTranslucency, goosTranslucency, baseDiffuse, goosDiffuse
		//, falloffAngle
		;
	int perimeterEdges, innerEdges, outerEdges, falloff, invert, includeAlphaBlend;
	BlendingMode blend;

	LWVParmID paramNoise, paramBaseColor, paramGoosColor, paramDistance, paramOpacity, paramBumpAmplitude, paramBaseSpecular, paramGoosSpecular
		, paramBaseGlossiness, paramGoosGlossiness, paramBaseLuminosity, paramGoosLuminosity, paramBaseReflection, paramGoosReflection
		, paramBaseTransparency, paramGoosTransparency, paramBaseRefractionIndex, paramGoosRefractionIndex, paramBaseTranslucency, paramGoosTranslucency
		, paramBaseDiffuse, paramGoosDiffuse, paramEdgeAngle//, paramFalloffAngle
		;

	// Input id's for the incoming connections
	NodeInputID inputBaseColor, inputGoosColor, inputNoise, inputDistance
		, inputOpacity, inputBlend, inputBaseBump, inputGoosBump
		, inputBumpAmplitude, inputBaseSpecular, inputGoosSpecular
		, inputBaseGlossiness, inputGoosGlossiness
		, inputBaseLuminosity, inputGoosLuminosity
		, inputBaseReflection, inputGoosReflection
		, inputBaseTransparency, inputGoosTransparency
		, inputBaseRefractionIndex, inputGoosRefractionIndex
		, inputBaseTranslucency, inputGoosTranslucency
		, inputBaseDiffuse, inputGoosDiffuse
		, inputIncludeAlpha, inputInvert
		//, inputFalloffAngle
		;

	// Output ids
	NodeOutputID outputColor, outputAlpha, outputBump, outputSpecular, outputGlossiness
		, outputLuminosity, outputReflection, outputTransparency, outputRefractionIndex, outputTranslucency, outputDiffuse;            

	EdgeList_p edgeList;
	LWMTGroupID threadGroupId;


} AnythingGoos, *AnythingGoos_p;

/***************************************************************
*
*  Input event function
*
*  This function is called whenever the user connects to
*  this input, disconnects a connection from the input,
*  or destroys a node connected to the input this callback
*  was assigned to.
*
*  The connection type can be used when you want to know which
*  type the connection to this input is.
*  Useful when you do not want to allow the connection to occur
*  if the types do not match. In this case you can call
*  inputfuncs->disconnect, and the connection will not occur.
*
***************************************************************/
static int InputEvent( void *userData, NodeInputID input, LWNodalEvent event, ConnectionType type )
{
	AnythingGoos_p inst = (AnythingGoos_p)userData;
	// Update the user interface
	if( inst->panel )
		xpanf->viewRefresh( inst->panel );

	// Update the instance data when the node connected to the input is destroyed
	if( lwupdate && event == NIE_INPUTNODEDESTROY ) 
		lwupdate( LWNODE_HCLASS, inst );

	return 1;
}

/*
======================================================================
create_vpenv()

Create variant parameters for envelopeable settings.

This is called from our create() callback to create and initialize the
vparms in our instance data.  We only support envelopes for these, not
textures.
====================================================================== */

LWVParmID create_vpenv( int type, char *name, LWChanGroupID group,
   double *ival )
{
   LWVParmID vp;

    if ( (vp = vparmfuncs->create( type, LWVPDT_NOTXTR ))) {
      vparmfuncs->setup( vp, name, group, NULL, NULL, NULL, NULL );
      vparmfuncs->setVal( vp, ival );
   }
   return vp;
}

/*
======================================================================
get_vpvalues()

Get the values of our instance parameters at a given time.
====================================================================== */

void get_vpvalues( AnythingGoos_p inst, LWTime t )
{
   double d[ 3 ];

   vparmfuncs->getVal( inst->paramBaseColor, t, NULL, d );
   inst->baseColor[ 0 ] = d[ 0 ];
   inst->baseColor[ 1 ] = d[ 1 ];
   inst->baseColor[ 2 ] = d[ 2 ];

   vparmfuncs->getVal( inst->paramGoosColor, t, NULL, d );
   inst->goosColor[ 0 ] = d[ 0 ];
   inst->goosColor[ 1 ] = d[ 1 ];
   inst->goosColor[ 2 ] = d[ 2 ];

   vparmfuncs->getVal( inst->paramNoise, t, NULL, d );
   inst->noise = d[ 0 ];

   vparmfuncs->getVal( inst->paramDistance, t, NULL, d );
   inst->distance = d[ 0 ];

   vparmfuncs->getVal( inst->paramOpacity, t, NULL, d );
   inst->opacity = d[ 0 ];

   vparmfuncs->getVal( inst->paramBumpAmplitude, t, NULL, d );
   inst->bumpAmplitude = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseSpecular, t, NULL, d );
   inst->baseSpecular = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosSpecular, t, NULL, d );
   inst->goosSpecular = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseGlossiness, t, NULL, d );
   inst->baseGlossiness = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosGlossiness, t, NULL, d );
   inst->goosGlossiness = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseLuminosity, t, NULL, d );
   inst->baseLuminosity = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosLuminosity, t, NULL, d );
   inst->goosLuminosity = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseReflection, t, NULL, d );
   inst->baseReflection = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosReflection, t, NULL, d );
   inst->goosReflection = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseTransparency, t, NULL, d );
   inst->baseTransparency = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosTransparency, t, NULL, d );
   inst->goosTransparency = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseRefractionIndex, t, NULL, d );
   inst->baseRefractionIndex = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosRefractionIndex, t, NULL, d );
   inst->goosRefractionIndex = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseTranslucency, t, NULL, d );
   inst->baseTranslucency = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosTranslucency, t, NULL, d );
   inst->goosTranslucency = d[ 0 ];

   vparmfuncs->getVal( inst->paramBaseDiffuse, t, NULL, d );
   inst->baseDiffuse = d[ 0 ];

   vparmfuncs->getVal( inst->paramGoosDiffuse, t, NULL, d );
   inst->goosDiffuse = d[ 0 ];

   vparmfuncs->getVal( inst->paramEdgeAngle, t, NULL, d );
   inst->edgeAngle = d[ 0 ];

   //vparmfuncs->getVal( inst->paramFalloffAngle, t, NULL, d );
   //inst->falloffAngle = d[ 0 ];


}

/***************************************************************
*
*  Create
*
*  Allocate instance data, create inputs and outputs for
*  the node, and setup default parameters.
*
***************************************************************/
XCALL_( static LWInstance )
Create( void *priv, NodeID node, LWError *err )
{
	AnythingGoos_p inst;
	LWChanGroupID parentGroup = NULL;

	// Allocate memory for the instance.
	inst = calloc( 1, sizeof( AnythingGoos ) );
	if( !inst )
	{
		// Return an error, if the allocation failed.
		*err = "Couldn't allocate memory for instance.";
		return NULL;
	}

	inst->edgeList = NULL;

	// Create inputs, and assign InputEvent as the input event callback.
	inst->inputBaseColor = inputfuncs->create( node, NOT_COLOR, fieldNames[AGB_baseColor], InputEvent );
	VSET( inst->baseColor, 1.0 );

	inst->inputGoosColor = inputfuncs->create( node, NOT_COLOR,  fieldNames[AGB_goosColor], InputEvent );
	VSET( inst->goosColor, 0.0 );

	inst->inputBlend = inputfuncs->create( node, NOT_INTEGER, fieldNames[AGB_blend], InputEvent );
	inst->blend = Blend_Normal;

	inst->inputOpacity = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_opacity], InputEvent );
	inst->opacity = 1.0 ;

	inst->inputInvert = inputfuncs->create( node, NOT_INTEGER, fieldNames[AGB_invert], InputEvent );
	inst->invert = 0;

	inst->inputNoise = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_noise], InputEvent );
	inst->noise = 1.0 ;

	inst->inputDistance = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_distance], InputEvent );
	inst->distance = 0.5f;

	//inst->inputFalloffAngle = inputfuncs->create(node, NOT_SCALAR, fieldNames[AGB_FalloffAngle], InputEvent);
	//inst->falloffAngle = M_PI;

	inst->inputIncludeAlpha = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_includeAlpha], InputEvent );

	inst->inputBumpAmplitude = inputfuncs->create( node, NOT_SCALAR, FIELD_NAME_BUMP_AMPLITUDE_SHORT, InputEvent );
	inst->bumpAmplitude = 1.0 ;

	inst->inputBaseLuminosity = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseLuminosity], InputEvent );
	inst->baseLuminosity = 0 ;
	inst->inputBaseDiffuse = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseDiffuse], InputEvent );
	inst->baseDiffuse = 1.0 ;
	inst->inputBaseSpecular = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseSpecular], InputEvent );
	inst->baseSpecular = 0.4 ;
	inst->inputBaseGlossiness = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseGlossiness], InputEvent );
	inst->baseGlossiness = 0.4 ;
	inst->inputBaseReflection = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseReflection], InputEvent );
	inst->baseReflection = 0 ;
	inst->inputBaseTransparency = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseTransparency], InputEvent );
	inst->baseTransparency = 0 ;
	inst->inputBaseRefractionIndex = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseRefractionIndex], InputEvent );
	inst->baseRefractionIndex = 1.0 ;
	inst->inputBaseTranslucency = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_baseTranslucency], InputEvent );
	inst->baseTranslucency = 0 ;
	inst->inputBaseBump = inputfuncs->create( node, NOT_VECTOR, "Base Bump", InputEvent );

	inst->inputGoosLuminosity = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosLuminosity], InputEvent );
	inst->goosLuminosity = 0 ;
	inst->inputGoosDiffuse = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosDiffuse], InputEvent );
	inst->goosDiffuse = 1.0 ;
	inst->inputGoosSpecular = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosSpecular], InputEvent );
	inst->goosSpecular = 0.4 ;
	inst->inputGoosGlossiness = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosGlossiness], InputEvent );
	inst->goosGlossiness = 0.4 ;
	inst->inputGoosReflection = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosReflection], InputEvent );
	inst->goosReflection = 0 ;
	inst->inputGoosTransparency = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosTransparency], InputEvent );
	inst->goosTransparency = 0 ;
	inst->inputGoosRefractionIndex = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosRefractionIndex], InputEvent );
	inst->goosRefractionIndex = 1.0 ;
	inst->inputGoosTranslucency = inputfuncs->create( node, NOT_SCALAR, fieldNames[AGB_goosTranslucency], InputEvent );
	inst->goosTranslucency = 0 ;
	inst->inputGoosBump = inputfuncs->create( node, NOT_VECTOR, "Goos Bump", InputEvent );

	inst->perimeterEdges = 1;
	inst->innerEdges = 1;
	inst->outerEdges = 1;
	inst->edgeAngle = M_PI / 6.0f;
	inst->falloff = AG_FalloffInvRadial;
	inst->includeAlphaBlend = AG_BlendAdditive;

	// Create a single output for the node.
	inst->outputColor = outputfuncs->create( node, NOT_COLOR, "Color" );
	inst->outputAlpha = outputfuncs->create( node, NOT_SCALAR, "Alpha" );
	inst->outputLuminosity = outputfuncs->create( node, NOT_SCALAR, "Luminosity" );
	inst->outputDiffuse = outputfuncs->create( node, NOT_SCALAR, "Diffuse" );
	inst->outputSpecular = outputfuncs->create( node, NOT_SCALAR, "Specular" );
	inst->outputGlossiness = outputfuncs->create( node, NOT_SCALAR, "Glossiness" );
	inst->outputReflection = outputfuncs->create( node, NOT_SCALAR, "Reflection" );
	inst->outputTransparency = outputfuncs->create( node, NOT_SCALAR, "Transparency" );
	inst->outputRefractionIndex = outputfuncs->create( node, NOT_SCALAR, "Refraction Index" );
	inst->outputTranslucency = outputfuncs->create( node, NOT_SCALAR, "Translucency" );
	inst->outputBump = outputfuncs->create( node, NOT_VECTOR, "Bump" );

	// Store the node id.
	inst->node = node;
	inst->threadGroupId = NULL;
	inst->panel = NULL;

	if ( mtutil )
		inst->threadGroupId = mtutil->groupCreate(1);

	parentGroup = nodefuncs->chanGrp( node );
	inst->paramNoise = create_vpenv( LWVP_FLOAT, fieldNames[AGB_noise], parentGroup, &inst->noise );
	inst->paramDistance = create_vpenv( LWVP_DIST, fieldNames[AGB_distance], parentGroup, &inst->distance );
	inst->paramEdgeAngle = create_vpenv( LWVP_ANGLE, fieldNames[AGB_edgeangle], parentGroup, &inst->edgeAngle );
	//inst->paramFalloffAngle = create_vpenv( LWVP_ANGLE, fieldNames[AGB_FalloffAngle], parentGroup, &inst->falloffAngle );
	inst->paramOpacity = create_vpenv( LWVP_FLOAT, fieldNames[AGB_opacity], parentGroup, &inst->opacity );
	inst->paramBaseColor = create_vpenv( LWVP_COLOR + LWVPF_VECTOR_PARMNAMES, fieldNames[AGB_baseColor], parentGroup, inst->baseColor );
	inst->paramGoosColor = create_vpenv( LWVP_COLOR + LWVPF_VECTOR_PARMNAMES, fieldNames[AGB_goosColor], parentGroup, inst->goosColor );
	inst->paramBumpAmplitude = create_vpenv( LWVP_FLOAT, fieldNames[AGB_bumpamplitude], parentGroup, &inst->bumpAmplitude );
	inst->paramBaseSpecular = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseSpecular], parentGroup, &inst->baseSpecular );
	inst->paramGoosSpecular = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosSpecular], parentGroup, &inst->goosSpecular );
	inst->paramBaseGlossiness = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseGlossiness], parentGroup, &inst->baseGlossiness );
	inst->paramGoosGlossiness = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosGlossiness], parentGroup, &inst->goosGlossiness );
	inst->paramBaseLuminosity = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseLuminosity], parentGroup, &inst->baseLuminosity );
	inst->paramGoosLuminosity = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosLuminosity], parentGroup, &inst->goosLuminosity );
	inst->paramBaseReflection = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseReflection], parentGroup, &inst->baseReflection );
	inst->paramGoosReflection = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosReflection], parentGroup, &inst->goosReflection );
	inst->paramBaseTransparency = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseTransparency], parentGroup, &inst->baseTransparency );
	inst->paramGoosTransparency = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosTransparency], parentGroup, &inst->goosTransparency );
	inst->paramBaseRefractionIndex = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseRefractionIndex], parentGroup, &inst->baseRefractionIndex );
	inst->paramGoosRefractionIndex = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosRefractionIndex], parentGroup, &inst->goosRefractionIndex );
	inst->paramBaseTranslucency = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseTranslucency], parentGroup, &inst->baseTranslucency );
	inst->paramGoosTranslucency = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosTranslucency], parentGroup, &inst->goosTranslucency );
	inst->paramBaseDiffuse = create_vpenv( LWVP_FLOAT, fieldNames[AGB_baseDiffuse], parentGroup, &inst->baseDiffuse );
	inst->paramGoosDiffuse = create_vpenv( LWVP_FLOAT, fieldNames[AGB_goosDiffuse], parentGroup, &inst->goosDiffuse );


	return inst;
}

void CommonDestroy(AnythingGoos_p inst)
{
	if( inst->panel )
	{
		xpanf->destroy( inst->panel );
		inst->panel = NULL;
	}
	if ( inst->threadGroupId ) 
	{
		mtutil->groupDestroy( inst->threadGroupId );
		inst->threadGroupId = NULL;
	}

}

 void destroyParm ( LWVParmID* parm )
 {
	if (*parm)
	{
		vparmfuncs->destroy(*parm);
		*parm = NULL;
	}
 }

void dirtyEdgeList(AnythingGoos_p inst)
{
	EdgeList_p nextEdgeList = inst->edgeList;
	while (nextEdgeList)
	{
		EdgeList_p currentEdgeList = nextEdgeList;
		if (currentEdgeList->isFilled)
			currentEdgeList->isDirty = true;
		nextEdgeList = currentEdgeList->next;
	}
 }

void clearEdgeList(AnythingGoos_p inst)
{
	EdgeList_p nextEdgeList = inst->edgeList;
	while (nextEdgeList)
	{
		EdgeList_p currentEdgeList = nextEdgeList;
		nextEdgeList = currentEdgeList->next;
		CleanupEdgeList(currentEdgeList);
		free(currentEdgeList);
	}
	inst->edgeList = NULL;
 }

/***************************************************************
*
*  Destroy
*
*  Destroy our instance data.
*  There is no need to destroy all the inputs and outputs,
*  because that will be taken care of automatically by the
*  Node Editor.
*
***************************************************************/
XCALL_( static void )
Destroy( LWInstance lwInst)
{
	AnythingGoos_p inst = lwInst;

	clearEdgeList(inst);
	CommonDestroy(inst);

	destroyParm(&inst->paramNoise);
	destroyParm(&inst->paramDistance);
	destroyParm(&inst->paramOpacity);
	destroyParm(&inst->paramBaseColor);
	destroyParm(&inst->paramGoosColor);
	destroyParm(&inst->paramBumpAmplitude);
	destroyParm(&inst->paramBaseSpecular);
	destroyParm(&inst->paramGoosSpecular);
	destroyParm(&inst->paramBaseGlossiness);
	destroyParm(&inst->paramGoosGlossiness);
	destroyParm(&inst->paramBaseLuminosity);
	destroyParm(&inst->paramGoosLuminosity);
	destroyParm(&inst->paramBaseReflection);
	destroyParm(&inst->paramGoosReflection);
	destroyParm(&inst->paramBaseTransparency);
	destroyParm(&inst->paramGoosTransparency);
	destroyParm(&inst->paramBaseRefractionIndex);
	destroyParm(&inst->paramGoosRefractionIndex);
	destroyParm(&inst->paramBaseTranslucency);
	destroyParm(&inst->paramGoosTranslucency);
	destroyParm(&inst->paramBaseDiffuse);
	destroyParm(&inst->paramGoosDiffuse);
	destroyParm(&inst->paramEdgeAngle);
	//destroyParm(&inst->paramFalloffAngle);
	free( inst );
}

/***************************************************************
*
*  Copy
*
*  Copy the parameter values to a new copy of our instance.
*
***************************************************************/
XCALL_( static LWError )
Copy( LWInstance lwInstTo, LWInstance lwInstFrom )
{
	AnythingGoos_p to = lwInstTo;
	AnythingGoos_p from = lwInstFrom;

	if( !to || !from )
		return NULL;

	//clear out those thing unique to an instance
	clearEdgeList(to);
	CommonDestroy(to);

	//copy basic attributes
	VCPY( to->baseColor, from->baseColor );
	VCPY( to->goosColor, from->goosColor);
	to->noise = from->noise;
	to->perimeterEdges = from->perimeterEdges;
	to->innerEdges = from->innerEdges;
	to->outerEdges = from->outerEdges;
	to->edgeAngle = from->edgeAngle;
	to->falloff = from->falloff;
	to->distance = from->distance;
	to->opacity = from->opacity;
	to->blend = from->blend;
	to->invert = from->invert;
	to->includeAlphaBlend = from->includeAlphaBlend;
	//to->falloffAngle = from->falloffAngle;
	
	//copy variant parameters
	vparmfuncs->copy( to->paramNoise, from->paramNoise );
	vparmfuncs->copy( to->paramDistance, from->paramDistance );
	vparmfuncs->copy( to->paramOpacity, from->paramOpacity );
	vparmfuncs->copy( to->paramBaseColor, from->paramBaseColor );
	vparmfuncs->copy( to->paramGoosColor, from->paramGoosColor );
	vparmfuncs->copy( to->paramBumpAmplitude, from->paramBumpAmplitude );
	vparmfuncs->copy( to->paramBaseSpecular, from->paramBaseSpecular );
	vparmfuncs->copy( to->paramGoosSpecular, from->paramGoosSpecular );
	vparmfuncs->copy( to->paramBaseGlossiness, from->paramBaseGlossiness );
	vparmfuncs->copy( to->paramGoosGlossiness, from->paramGoosGlossiness );
	vparmfuncs->copy( to->paramBaseLuminosity, from->paramBaseLuminosity );
	vparmfuncs->copy( to->paramGoosLuminosity, from->paramGoosLuminosity );
	vparmfuncs->copy( to->paramBaseReflection, from->paramBaseReflection );
	vparmfuncs->copy( to->paramGoosReflection, from->paramGoosReflection );
	vparmfuncs->copy( to->paramBaseTransparency, from->paramBaseTransparency );
	vparmfuncs->copy( to->paramGoosTransparency, from->paramGoosTransparency );
	vparmfuncs->copy( to->paramBaseRefractionIndex, from->paramBaseRefractionIndex );
	vparmfuncs->copy( to->paramGoosRefractionIndex, from->paramGoosRefractionIndex );
	vparmfuncs->copy( to->paramBaseTranslucency, from->paramBaseTranslucency );
	vparmfuncs->copy( to->paramGoosTranslucency, from->paramGoosTranslucency );
	vparmfuncs->copy( to->paramBaseDiffuse, from->paramBaseDiffuse );
	vparmfuncs->copy( to->paramGoosDiffuse, from->paramGoosDiffuse );
	vparmfuncs->copy( to->paramEdgeAngle, from->paramEdgeAngle );
	//vparmfuncs->copy( to->paramFalloffAngle, from->paramFalloffAngle );

	//get a new thread group
	if ( mtutil )
		to->threadGroupId = mtutil->groupCreate(1);

	return NULL;
}

/***************************************************************
*
*  Load
*
*  Load our parameter values.
*
***************************************************************/
XCALL_( static LWError )
Load(  LWInstance lwInst, const LWLoadState *load )
{
	AnythingGoos_p inst = lwInst;

	LWID    id;
	LWError err;

    if ( (err = vparmfuncs->load( inst->paramNoise, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseColor, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosColor, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramOpacity, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBumpAmplitude, load ))) return err;


	// Find the data blocks
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_EDGEANGLE)
	{
		float   fval;
		LWLOAD_FP( load, &fval, 1 );
		inst->edgeAngle = fval;
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_DIST)
	{
		float   fval;
		LWLOAD_FP( load, &fval, 1 );
		inst->distance = fval;
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_PERIMETER)
	{
		LWLOAD_I4(load, &inst->perimeterEdges, 1);
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_INNER)
	{
		LWLOAD_I4(load, &inst->innerEdges, 1);
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_OUTER)
	{
		LWLOAD_I4(load, &inst->outerEdges, 1);
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_INVERT)
	{
		LWLOAD_I4(load, &inst->invert, 1);
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_FALLOFF)
	{
		LWLOAD_I4(load, &inst->falloff, 1);
		LWLOAD_END( load );
	}
	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_BLEND)
	{
		LWLOAD_I4(load, (int*)&inst->blend, 1);
		LWLOAD_END( load );
	}

    if ( (err = vparmfuncs->load( inst->paramBaseSpecular, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosSpecular, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseGlossiness, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosGlossiness, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseLuminosity, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosLuminosity, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseReflection, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosReflection, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseTransparency, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosTransparency, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseRefractionIndex, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosRefractionIndex, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseTranslucency, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosTranslucency, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramBaseDiffuse, load ))) return err;
    if ( (err = vparmfuncs->load( inst->paramGoosDiffuse, load ))) return err;

	id = LWLOAD_FIND( load, AG_Block );
	if (id == ID_AG_INCLUDE_ALPHA_BLEND)
	{
		LWLOAD_I4(load, &inst->includeAlphaBlend, 1);
		LWLOAD_END( load );
	}

    if ( (err = vparmfuncs->load( inst->paramDistance, load ))) 
	{
		vparmfuncs->setVal( inst->paramDistance, &inst->distance );
	}
    if ( (err = vparmfuncs->load( inst->paramEdgeAngle, load )))
	{
		vparmfuncs->setVal( inst->paramEdgeAngle, &inst->edgeAngle );
	}
	//if ( err = vparmfuncs->load( inst->paramFalloffAngle, load ))
	//{
	//	vparmfuncs->setVal( inst->paramFalloffAngle, &inst->falloffAngle );
	//}
	get_vpvalues( inst, 0.0 );

	return NULL;
}

/***************************************************************
*
*  Save
*
*  Save our parameter values.
*
***************************************************************/
XCALL_( static LWError )
Save( LWInstance lwInst, const LWSaveState *save )
{
	AnythingGoos_p inst = lwInst;

	LWError err;
	
    if ( (err = vparmfuncs->save( inst->paramNoise, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseColor, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosColor, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramOpacity, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBumpAmplitude, save ))) return err;

	LWSAVE_BEGIN( save, &AG_Block[ AGB_edgeangle ], 1 );
	{
		float   fval = (float)inst->edgeAngle;
		LWSAVE_FP( save, &fval, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_distance ], 1 );
	{
		float   fval = (float)inst->distance;
		LWSAVE_FP( save, &fval, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_perimeter ], 1 );
	{
		LWSAVE_I4( save, &inst->perimeterEdges, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_inner ], 1 );
	{
		LWSAVE_I4( save, &inst->innerEdges, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_outer ], 1 );
	{
		LWSAVE_I4( save, &inst->outerEdges, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_invert ], 1 );
	{
		LWSAVE_I4( save, &inst->invert, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_falloff ], 1 );
	{
		LWSAVE_I4( save, &inst->falloff, 1 );
	}
	LWSAVE_END( save );

	LWSAVE_BEGIN( save, &AG_Block[ AGB_blend ], 1 );
	{
		LWSAVE_I4( save, (const int*)&inst->blend, 1 );
	}
	LWSAVE_END( save );

    if ( (err = vparmfuncs->save( inst->paramBaseSpecular, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosSpecular, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseGlossiness, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosGlossiness, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseLuminosity, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosLuminosity, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseReflection, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosReflection, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseTransparency, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosTransparency, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseRefractionIndex, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosRefractionIndex, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseTranslucency, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosTranslucency, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramBaseDiffuse, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramGoosDiffuse, save ))) return err;

	LWSAVE_BEGIN( save, &AG_Block[ AGB_includeAlphaBlend ], 1 );
	{
		LWSAVE_I4( save, (const int*)&inst->includeAlphaBlend, 1 );
	}
	LWSAVE_END( save );

    if ( (err = vparmfuncs->save( inst->paramDistance, save ))) return err;
    if ( (err = vparmfuncs->save( inst->paramEdgeAngle, save ))) return err;
	//if ( err = vparmfuncs->save( inst->paramFalloffAngle, save )) return err;
	return NULL;
}

/***************************************************************
*
*  Init
*
*  Initialize render data here. Mode determines if init is
*  called for a previewer or the renderer. In which case you
*  might want to initialize data differently.
*  This shading model does not need to initialize data.
*
***************************************************************/
XCALL_( static LWError )
Init( LWInstance lwInst, int mode )
{
	//AnythingGoos_p inst = lwInst;
	//run into occasions where the render doesn't call cleanup
	//and this new run thinks we've already built the cache
	//clearEdgeList(inst);

	return NULL;
}

/***************************************************************
*
*  Cleanup
*
*  Cleanup render data here.
*  If there was data allocated in init, this is where you free
*  the data. This function is called when the renderer or
*  the previewer has finished.
*
***************************************************************/
XCALL_( static void )
Cleanup( LWInstance lwInst)
{
	/*AnythingGoos_p inst = lwInst;
		
	CleanupEdgeList (inst->edgeList);
	if (inst->edgeList)
	{
		free (inst->edgeList);
		inst->edgeList = NULL;
	}*/
}

/***************************************************************
*
*  NewTime
*
*  NewTime is called when a new pass starts.
*  This is where you setup your time dependant data.
*
***************************************************************/
XCALL_( static LWError )
NewTime( LWInstance lwInst, LWFrame f, LWTime t )
{
	double d[ 3 ];
	AnythingGoos_p inst = lwInst;

	vparmfuncs->getVal( inst->paramEdgeAngle, t, NULL, d );
	if (inst->edgeAngle != d[ 0 ])
	{
		dirtyEdgeList(inst);
	}
	get_vpvalues(inst, t);
	return NULL;
}


size_t scanPoints( void *voidWorkingMesh, LWPntID id )
{
	WorkingMesh_p workingMesh = voidWorkingMesh;
	workingMesh->points[ workingMesh->pointCount ].id = id;
	++workingMesh->pointCount;

	return 0;
}

size_t scanPolys(  void *voidWorkingMesh, LWPolID id )
{
	WorkingMesh_p workingMesh = voidWorkingMesh;
	workingMesh->polygons[ workingMesh->polygonCount ].id = id;
	workingMesh->polygons[ workingMesh->polygonCount ].vertices = NULL;
	++workingMesh->polygonCount;

	return 0;
}

/*======================================================================
findVert()

Binary search the point array and return the array index for the given
point ID.
====================================================================== */

int findVert( WorkingMesh_p odb, LWPntID id )
{
	int lt = 0, rt = odb->pointCount - 1, x;

	while ( rt >= lt ) {
		x = ( lt + rt ) / 2;
		if ( id < odb->points[ x ].id ) rt = x - 1; else lt = x + 1;
		if ( id == odb->points[ x ].id ) return x;
	}
	return -1;
}

/*
======================================================================
getPolyNormals()

Calculate the polygon normals.  By convention, LW's polygon normals
are based on the first, second and last points in the vertex list.
The normal is the cross product of two vectors formed from these
points.  It's undefined for one- and two-point polygons.
====================================================================== */

void getPolygonNormals( WorkingMesh_p odb )
{
	unsigned int j, k;
	LWFVector p1, p2, pn, v1, v2;

	for ( j = 0; j < odb->polygonCount; j++ ) {
		if ( odb->polygons[ j ].vertexCount < 3 ) continue;
		for ( k = 0; k < 3; k++ ) {
			p1[ k ] = odb->points[ odb->polygons[ j ].vertices[ 0 ] ].vertex[ k ];
			p2[ k ] = odb->points[ odb->polygons[ j ].vertices[ 1 ] ].vertex[ k ];
			pn[ k ] = odb->points[ odb->polygons[ j ].vertices[
				odb->polygons[ j ].vertexCount - 1 ] ].vertex[ k ];
		}

		for ( k = 0; k < 3; k++ ) {
			v1[ k ] = p2[ k ] - p1[ k ];
			v2[ k ] = pn[ k ] - p1[ k ];
		}

		cross( v1, v2, odb->polygons[ j ].normal );
		normalize( odb->polygons[ j ].normal );
	}
}


static void getVertexForEachPoint(LWMeshInfoID meshInfo, WorkingMesh_p workingMesh)
{
	unsigned int vertexIndex;

	for (vertexIndex = 0; vertexIndex < workingMesh->pointCount; vertexIndex++) {
		meshInfo->pntBasePos( meshInfo, workingMesh->points[vertexIndex].id, workingMesh->points[vertexIndex].vertex );
	}
}
static bool getPolygonPoints(LWMeshInfoID meshInfo, WorkingMesh_p workingMesh)
{

	unsigned int polygonIndex, vertexIndex;

	workingMesh->rawEdgeCount = 0;
	for (polygonIndex = 0; polygonIndex < workingMesh->polygonCount; polygonIndex++ ) {
		workingMesh->polygons[ polygonIndex ].vertexCount
			= meshInfo->polSize( meshInfo, workingMesh->polygons[ polygonIndex ].id );
		
		//for our edge list we only care about polys with 3 or more points
		if (workingMesh->polygons[polygonIndex].vertexCount >= 3) {
			workingMesh->rawEdgeCount += workingMesh->polygons[ polygonIndex ].vertexCount;
		}

		workingMesh->polygons[ polygonIndex ].vertices
			= calloc( workingMesh->polygons[ polygonIndex ].vertexCount, sizeof( unsigned int ));
		if (!workingMesh->polygons[ polygonIndex ].vertices)
		{
			return false;
		}

		for (vertexIndex = 0; vertexIndex < workingMesh->polygons[ polygonIndex ].vertexCount; vertexIndex++ ) {
			LWPntID pointId = meshInfo->polVertex( meshInfo, workingMesh->polygons[ polygonIndex ].id, vertexIndex );
			workingMesh->polygons[ polygonIndex ].vertices[vertexIndex] = findVert( workingMesh, pointId );
		}
	}

	return true;
}

void addRawEdge(WorkingMesh_p workingMesh, WorkingRawEdge_p rawEdges, unsigned int polygonIndex
				, unsigned int vertexIndex1, unsigned int vertexIndex2, unsigned int rawEdgeIndex) {
	unsigned int p1, p2;

	p1 = workingMesh->polygons[polygonIndex].vertices[vertexIndex1];
	p2 = workingMesh->polygons[polygonIndex].vertices[vertexIndex2];

	rawEdges[rawEdgeIndex].polygon = polygonIndex;
	if (p1 < p2) {
		rawEdges[rawEdgeIndex].vertices[0] = p1;
		rawEdges[rawEdgeIndex].vertices[1] = p2;
	}
	else {
		rawEdges[rawEdgeIndex].vertices[0] = p2;
		rawEdges[rawEdgeIndex].vertices[1] = p1;
	}

}

bool getWorkingEdgeList(AnythingGoos_p inst, EdgeList_p edgeList, WorkingMesh_p workingMesh) {

	unsigned int polygonIndex, edgeIndex, rawEdgeIndex = 0, workingPointIndex;
	WorkingRawEdge_p rawEdges = NULL;
	char *drawEdge = NULL;
	char *keepPoint = NULL;
	unsigned int *newPointIndex = NULL;

	getPolygonNormals(workingMesh);

	//build our raw list of edges
	rawEdges = (WorkingRawEdge_p) calloc(workingMesh->rawEdgeCount, sizeof(WorkingRawEdge));
	if (!rawEdges)
	{
		return false;
	}

	for (polygonIndex = 0; polygonIndex < workingMesh->polygonCount; polygonIndex++ ) {
		if (workingMesh->polygons[polygonIndex].vertexCount >= 3) {
			for (edgeIndex = 0; edgeIndex < workingMesh->polygons[polygonIndex].vertexCount - 1; edgeIndex++) {
				addRawEdge(workingMesh, rawEdges, polygonIndex, edgeIndex, edgeIndex + 1, rawEdgeIndex);
				rawEdgeIndex++;
			}//end edgeindex
			//do last and first edge
			addRawEdge(workingMesh, rawEdges, polygonIndex, 0, workingMesh->polygons[polygonIndex].vertexCount - 1, rawEdgeIndex);
			rawEdgeIndex++;
		}//end at least 3 sides

	}//end polygons

	//sort them so they are grouped
	qsort(rawEdges, workingMesh->rawEdgeCount, sizeof(WorkingRawEdge), WorkingRawEdgeCompare);

	//now spin through and examine the groups to see
	//if they need to be drawn
	drawEdge = (char*) calloc(workingMesh->rawEdgeCount, sizeof(char));
	if (!drawEdge)
	{
		free(rawEdges);
		return false;
	}
	
	//TODO:compare more than just first against others
	rawEdgeIndex = 0;
	while (rawEdgeIndex < workingMesh->rawEdgeCount) {
		unsigned int edgeOffset = 1;
		unsigned int edgePoint1 = rawEdges[rawEdgeIndex].vertices[0]
			, edgePoint2 = rawEdges[rawEdgeIndex].vertices[1];

		//assume we don't draw it
		drawEdge[rawEdgeIndex] = 0;
		
		//compare it against other polys that share the edge
		while (rawEdgeIndex + edgeOffset < workingMesh->rawEdgeCount
			&& rawEdges[rawEdgeIndex].vertices[0] == rawEdges[rawEdgeIndex + edgeOffset].vertices[0]
			&& rawEdges[rawEdgeIndex].vertices[1] == rawEdges[rawEdgeIndex + edgeOffset].vertices[1]) {
			
			double edgeCos, sign, edgeAngle;

			unsigned int polygonIndex1 = rawEdges[rawEdgeIndex].polygon
				, polygonIndex2 = rawEdges[rawEdgeIndex + edgeOffset].polygon;

			WorkingPolygon_p polygon1 = &workingMesh->polygons[polygonIndex1]
				, polygon2 = &workingMesh->polygons[polygonIndex2];

			unsigned int polygon1p1 = polygon1->vertices[0]
				, polygon1p2 = polygon1->vertices[1]
				, polygon1p3 = polygon1->vertices[2];

			LWFVector *polyNormal1 = &polygon1->normal
				, *polyNormal2 = &polygon2->normal;


			LWFVector planeVector;
			
			edgeCos = dot(*polyNormal1, *polyNormal2);

			if (polygon1p1 != edgePoint1 && polygon1p1 != edgePoint2)
			{
				//pt1 not on edge
				planeVector[0] = workingMesh->points[polygon1p1].vertex[0] - workingMesh->points[edgePoint1].vertex[0];
				planeVector[1] = workingMesh->points[polygon1p1].vertex[1] - workingMesh->points[edgePoint1].vertex[1];
				planeVector[2] = workingMesh->points[polygon1p1].vertex[2] - workingMesh->points[edgePoint1].vertex[2];

			}
			else if (polygon1p2 != edgePoint1 && polygon1p2 != edgePoint2)
			{
				//pt2 not on edge
				planeVector[0] = workingMesh->points[polygon1p2].vertex[0] - workingMesh->points[edgePoint1].vertex[0];
				planeVector[1] = workingMesh->points[polygon1p2].vertex[1] - workingMesh->points[edgePoint1].vertex[1];
				planeVector[2] = workingMesh->points[polygon1p2].vertex[2] - workingMesh->points[edgePoint1].vertex[2];

			}
			else
			{
				//pt3 not on edge
				planeVector[0] = workingMesh->points[polygon1p3].vertex[0] - workingMesh->points[edgePoint1].vertex[0];
				planeVector[1] = workingMesh->points[polygon1p3].vertex[1] - workingMesh->points[edgePoint1].vertex[1];
				planeVector[2] = workingMesh->points[polygon1p3].vertex[2] - workingMesh->points[edgePoint1].vertex[2];

			}

			sign = dot(*polyNormal2, planeVector);

			if (sign < 0)
			{
				edgeAngle = acos(edgeCos);
			}
			else
			{
				edgeAngle = -acos(edgeCos);
			}

			if ((edgeAngle > inst->edgeAngle) && inst->outerEdges)
			{
				drawEdge[rawEdgeIndex] = 1;
			}
			else
			if ((edgeAngle < -inst->edgeAngle) && inst->innerEdges)
			{
				drawEdge[rawEdgeIndex] = 1;
			}


			//set the dupe's draw edge = false
			drawEdge[rawEdgeIndex + edgeOffset] = 0;
			
			edgeOffset++;

		}
		if (edgeOffset == 1) {
			//nothing matched, this must be a perimeter edge
			drawEdge[rawEdgeIndex] = inst->perimeterEdges;
		}
		//just make sure that this edge isn't zero length
		if (drawEdge[rawEdgeIndex] 
			&& workingMesh->points[edgePoint1].vertex[0] == workingMesh->points[edgePoint2].vertex[0]
			&& workingMesh->points[edgePoint1].vertex[1] == workingMesh->points[edgePoint2].vertex[1]
			&& workingMesh->points[edgePoint1].vertex[2] == workingMesh->points[edgePoint2].vertex[2]) {

			drawEdge[rawEdgeIndex] = 0;
		}
		rawEdgeIndex += edgeOffset;

	}//end raw edges

	//don't need polygons anymore free that memory
	FreePolygonsWorkingMesh(workingMesh);

	//copy the edges
	edgeList->edgeCount = 0;

	for (rawEdgeIndex = 0; rawEdgeIndex < workingMesh->rawEdgeCount; rawEdgeIndex++) {
		if (drawEdge[rawEdgeIndex] == 1) {
			edgeList->edgeCount++;
		}
	}

	edgeList->edges = calloc(edgeList->edgeCount, sizeof(Edge));
	if (!edgeList->edges)
	{
		free(drawEdge);
		free(rawEdges);
		return false;
	}

	edgeIndex = 0;

	for (rawEdgeIndex = 0; rawEdgeIndex < workingMesh->rawEdgeCount; rawEdgeIndex++) {
		if (drawEdge[rawEdgeIndex] == 1) {
			edgeList->edges[edgeIndex].vertices[0] = 	rawEdges[rawEdgeIndex].vertices[0];
			edgeList->edges[edgeIndex].vertices[1] = 	rawEdges[rawEdgeIndex].vertices[1];
			edgeIndex++;
		}
	}

	//don't need edge data anymore
	free(drawEdge);
	drawEdge = NULL;

	free(rawEdges);
	rawEdges = NULL;

	//copy the points
	keepPoint = (char*) calloc(workingMesh->pointCount, sizeof(char));
	newPointIndex = (unsigned int*) calloc(workingMesh->pointCount, sizeof(unsigned int));
	if (!newPointIndex || !keepPoint)
	{
		if (newPointIndex)
			free(newPointIndex);
		if (keepPoint)
			free(keepPoint);
		return false;
	}
	
	//assume we don't keep it
	for (workingPointIndex = 0; workingPointIndex < workingMesh->pointCount; workingPointIndex++) {
		keepPoint[workingPointIndex] = 0;
	}

	//spin through the edges and see which points we need to keep
	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++) {
		keepPoint[edgeList->edges[edgeIndex].vertices[0]] = 1;
		keepPoint[edgeList->edges[edgeIndex].vertices[1]] = 1;
	}

	//build a new index for the existing points
	edgeList->pointCount = 0;
	for (workingPointIndex = 0; workingPointIndex < workingMesh->pointCount; workingPointIndex++) {
		if (keepPoint[workingPointIndex] == 1) {
			newPointIndex[workingPointIndex] = edgeList->pointCount;
			edgeList->pointCount++;
		}
	}

	//copy over the ones we want to keep
	edgeList->points = calloc(edgeList->pointCount, sizeof(LWFVector));
	for (workingPointIndex = 0; workingPointIndex < workingMesh->pointCount; workingPointIndex++) {
		if (keepPoint[workingPointIndex] == 1) {
			edgeList->points[newPointIndex[workingPointIndex]][0] = 
				workingMesh->points[workingPointIndex].vertex[0];
			edgeList->points[newPointIndex[workingPointIndex]][1] = 
				workingMesh->points[workingPointIndex].vertex[1];
			edgeList->points[newPointIndex[workingPointIndex]][2] = 
				workingMesh->points[workingPointIndex].vertex[2];
		}	
	}
	
	//free the points and keepPoints, don't need them anymore
	free(keepPoint);
	keepPoint = NULL;

	FreePointsWorkingMesh(workingMesh);

	//revalue our edge point index values
	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++) {
		edgeList->edges[edgeIndex].vertices[0] = newPointIndex[edgeList->edges[edgeIndex].vertices[0]];
		edgeList->edges[edgeIndex].vertices[1] = newPointIndex[edgeList->edges[edgeIndex].vertices[1]];
	}

	//free newPointIndex
	free(newPointIndex);
	newPointIndex = NULL;

	//now precalc our one over magnitude and p1 - p2
	edgeList->oneOverLineMagnitude = (float*)calloc(edgeList->edgeCount, sizeof(float));
	edgeList->p2minusp1 = (LWFVector*)calloc(edgeList->edgeCount, sizeof(LWFVector));
	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++) {
		unsigned int p1 = edgeList->edges[edgeIndex].vertices[0]
			, p2 = edgeList->edges[edgeIndex].vertices[1];

		edgeList->p2minusp1[edgeIndex][0] = edgeList->points[p2][0] - edgeList->points[p1][0];
		edgeList->p2minusp1[edgeIndex][1] = edgeList->points[p2][1] - edgeList->points[p1][1];
		edgeList->p2minusp1[edgeIndex][2] = edgeList->points[p2][2] - edgeList->points[p1][2];

		edgeList->oneOverLineMagnitude[edgeIndex] = dot(edgeList->p2minusp1[edgeIndex], edgeList->p2minusp1[edgeIndex]);

		edgeList->oneOverLineMagnitude[edgeIndex] = 1.0f / edgeList->oneOverLineMagnitude[edgeIndex];
	}
	return true;
}

double localEvaluate(AnythingGoos_p inst, EdgeList_p edgeList, LWNodalAccess *na, int threadIndex, int cachePosition)
{
	double result = 0;

	if (na->oPos[0] != edgeList->lastPos[threadIndex][0]
		|| na->oPos[1] != edgeList->lastPos[threadIndex][1]
		|| na->oPos[2] != edgeList->lastPos[threadIndex][2] )
	{

		double distance = FPOSINF;
		double distanceToUse = inst->distance;
		double noise = inst->noise;
		double opacity = inst->opacity;
		double includeAlpha = 0;
		int invert = inst->invert;

		inputfuncs->evaluate(inst->inputDistance, na, &distanceToUse);
		inputfuncs->evaluate(inst->inputInvert, na, &invert);

		//if we have an edge list to work with
		if (edgeList->isFilled) {
			//find the closest edge
			distance = getDistanceToEdge(edgeList, na->oPos, threadIndex);
		}
		//if we don't have objinfo, assume we're in the node editor and do
		//a simple evaluation
		else {
			double tempDistance[3] = {DPOSINF, DPOSINF, DPOSINF};
			if (inst->innerEdges)
				tempDistance[0] = fabs(na->oPos[0]);
			if (inst->outerEdges)
				tempDistance[1] = fabs(na->oPos[1]);
			if (inst->perimeterEdges)
				tempDistance[2] = fabs(na->oPos[2]);
			distance = tempDistance[0];
			if (tempDistance[1] < distance)
				distance = tempDistance[1];
			if (tempDistance[2] < distance)
				distance = tempDistance[2];
		}


		inputfuncs->evaluate(inst->inputNoise, na, &noise);
		distanceToUse *= noise;

		//do falloff 
		if (distance <= distanceToUse) 
		{
			switch (inst->falloff) {
				case AG_FalloffAbsolute:
					result = 1.0;
					break;
				case AG_FalloffRadial:
					result = cos(distance / distanceToUse * M_PI_2);
					break;
				case AG_FalloffLinear:
					result = (distanceToUse - distance) / distanceToUse;
					break;
				case AG_FalloffInvRadial:
					result = cos(M_PI_2 + distance / distanceToUse * M_PI_2) + 1.0f;
					break;
			}
		}//end < distance

		if (invert)
		{
			result = (1.0 - result);
		}

		inputfuncs->evaluate(inst->inputOpacity, na, &opacity);
		result *= opacity;

		switch (inst->includeAlphaBlend)
		{
		case AG_BlendAdditive:
		case AG_BlendSubtractive:
		case AG_BlendMax:
			includeAlpha = 0;
			break;
		case AG_BlendMultiple:
		case AG_BlendMin:
			includeAlpha = 1.0;
			break;
		}

		inputfuncs->evaluate(inst->inputIncludeAlpha, na, &includeAlpha);
		switch (inst->includeAlphaBlend)
		{
		case AG_BlendAdditive:
			result += includeAlpha;
			break;
		case AG_BlendSubtractive:
			result -= includeAlpha;
			break;
		case AG_BlendMultiple:
			result *= includeAlpha;
			break;
		case AG_BlendMax:
			if (includeAlpha > result)
				result = includeAlpha;
			break;
		case AG_BlendMin:
			if (includeAlpha < result)
				result = includeAlpha;
			break;
		}
		//clamp to 0 to 1
		if (result > 1.0)
			result = 1.0;
		if (result < 0)
			result = 0;

		if (cachePosition)
		{
			VCPY(edgeList->lastPos[threadIndex], na->oPos);
			edgeList->lastValue[threadIndex] = result;
		}

	}
	else
	{
		result = edgeList->lastValue[threadIndex];
	}
	return result;
}

void SimpleValueMix(AnythingGoos_p inst, EdgeList_p edgeList, LWNodalAccess *na, int threadIndex
					, double instBaseValue, double instGoosValue 
					, NodeInputID baseInput, NodeInputID goosInput
					, NodeValue Value)
{
	double result = localEvaluate(inst, edgeList, na, threadIndex, 1);
	double baseValue = instBaseValue;
	double goosValue = instGoosValue;
	inputfuncs->evaluate(baseInput, na, &baseValue);
	inputfuncs->evaluate(goosInput, na, &goosValue);

	result = result * goosValue + (1.0f - result) * baseValue;
	outputfuncs->setValue(Value, &result);
}

EdgeList_p getEdgeList(AnythingGoos_p inst, LWNodalAccess *na)
{
	EdgeList_p edgeList = inst->edgeList;
	while (edgeList != NULL)
	{
		if (na->objID == edgeList->objID && !edgeList->isDirty)
			return edgeList;
		edgeList = edgeList->next;
	}
	return NULL;
}

EdgeList_p getOrCreateEdgeList(AnythingGoos_p inst, LWNodalAccess *na)
{
	LWMeshInfoID meshInfo = NULL;

	EdgeList_p edgeList = inst->edgeList;
	while (edgeList != NULL)
	{
		if (na->objID == edgeList->objID && !edgeList->isDirty)
		{
			return edgeList;
		}
		edgeList = edgeList->next;
	}

	edgeList = calloc(1, sizeof(EdgeList));
	InitEdgeList(edgeList);

	//get mesh
	if (objinfo) {
		meshInfo = objinfo->meshInfo(na->objID, 1);
	}
	if (meshInfo) {
		WorkingMesh workingMesh;

		InitWorkingMesh(&workingMesh);
		
		workingMesh.points = calloc( meshInfo->numPoints( meshInfo ), sizeof( WorkingPoint));
		if (!workingMesh.points)
		{
			CleanupWorkingMesh(&workingMesh);
			CleanupEdgeList(edgeList);
			return NULL;
		}

		meshInfo->scanPoints( meshInfo, scanPoints, &workingMesh );

		qsort(workingMesh.points, workingMesh.pointCount, sizeof(WorkingPoint), WorkingPointCompare);

		workingMesh.polygons = calloc(meshInfo->numPolygons( meshInfo), sizeof( WorkingPolygon));
		if (!workingMesh.polygons)
		{
			CleanupWorkingMesh(&workingMesh);
			CleanupEdgeList(edgeList);
			free (edgeList);
			return NULL;
		}

		meshInfo->scanPolys( meshInfo, scanPolys, &workingMesh);

		if (!getPolygonPoints(meshInfo, &workingMesh))
		{
			CleanupWorkingMesh(&workingMesh);
			CleanupEdgeList(edgeList);
			free (edgeList);
			return NULL;
		}

		getVertexForEachPoint(meshInfo, &workingMesh);

		if (!getWorkingEdgeList(inst, edgeList, &workingMesh))
		{
			CleanupWorkingMesh(&workingMesh);
			CleanupEdgeList(edgeList);
			free (edgeList);
			return NULL;
		}


		if (!fillNodeTree(edgeList))
		{
			CleanupWorkingMesh(&workingMesh);
			CleanupEdgeList(edgeList);
			free (edgeList);
			return NULL;
		}

		edgeList->isFilled = true;
	}//got mesh info

	edgeList->objID = na->objID;

	if (inst->edgeList == NULL)
	{
		inst->edgeList = edgeList;
	}
	else
	{
		EdgeList_p currentEdgeList = inst->edgeList;
		while (currentEdgeList->next)
		{
			currentEdgeList = currentEdgeList->next;
		}
		currentEdgeList->next = edgeList;

	}
	return edgeList;

}

/***************************************************************
*
*  Evaluate
*
*  The main evaluation function.
*
***************************************************************/
XCALL_( static void )
Evaluate( LWInstance lwInst, LWNodalAccess *na, NodeOutputID out, NodeValue Value )
{
	AnythingGoos_p inst = lwInst;
	EdgeList_p edgeList = NULL;

	int threadIndex;
	
	threadIndex = mtutil->threadGetIndex( );
	if (threadIndex < 0)
	{
		threadIndex = 0;
	}

	//first check out of the mutex
	edgeList = getEdgeList(inst, na);

	if (edgeList == NULL) {

		//if not then we'll start up a mutex,
		//check again and then build if need be
		if (mtutil)
			mtutil->groupLockMutex( inst->threadGroupId, 0 );
				
		edgeList = getOrCreateEdgeList(inst, na);

		if (mtutil)
			mtutil->groupUnlockMutex( inst->threadGroupId, 0 );

		if (!edgeList)
			return;
	}//needed to build cache out of mutex

	if (out == inst->outputColor)
	{
		LWDVector final_color, baseColor, goosColor;
		double result = localEvaluate(inst, edgeList, na, threadIndex, 1);
		BlendingMode blend = inst->blend;

		VCPY( baseColor, inst->baseColor );
		inputfuncs->evaluate( inst->inputBaseColor, na, baseColor );
		VCPY( goosColor, inst->goosColor );
		inputfuncs->evaluate( inst->inputGoosColor, na, goosColor );

		inputfuncs->evaluate(inst->inputBlend, na, &blend);

		nodeutilfuncs->Blend(final_color, goosColor, baseColor,  result, blend);

		// Set the output color value.
		outputfuncs->setValue( Value, final_color );
	}
	else if (out == inst->outputAlpha)
	{
		double result = localEvaluate(inst, edgeList, na, threadIndex, 1);
		outputfuncs->setValue(Value, &result);
	}
	else if (out == inst->outputSpecular)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseSpecular, inst->goosSpecular
			, inst->inputBaseSpecular, inst->inputGoosSpecular
			, Value);
	}
	else if (out == inst->outputGlossiness)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseGlossiness, inst->goosGlossiness
			, inst->inputBaseGlossiness, inst->inputGoosGlossiness
			, Value);
	}
	else if (out == inst->outputLuminosity)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseLuminosity, inst->goosLuminosity
			, inst->inputBaseLuminosity, inst->inputGoosLuminosity
			, Value);
	}
	else if (out == inst->outputReflection)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseReflection, inst->goosReflection
			, inst->inputBaseReflection, inst->inputGoosReflection
			, Value);
	}
	else if (out == inst->outputTransparency)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseTransparency, inst->goosTransparency
			, inst->inputBaseTransparency, inst->inputGoosTransparency
			, Value);
	}
	else if (out == inst->outputRefractionIndex)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseRefractionIndex, inst->goosRefractionIndex
			, inst->inputBaseRefractionIndex, inst->inputGoosRefractionIndex
			, Value);
	}
	else if (out == inst->outputTranslucency)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseTranslucency, inst->goosTranslucency
			, inst->inputBaseTranslucency, inst->inputGoosTranslucency
			, Value);
	}
	else if (out == inst->outputDiffuse)
	{
		SimpleValueMix(inst, edgeList, na, threadIndex
			, inst->baseDiffuse, inst->goosDiffuse
			, inst->inputBaseDiffuse, inst->inputGoosDiffuse
			, Value);
	}
	else if (out == inst->outputBump)
	{
		double result = localEvaluate(inst, edgeList, na, threadIndex, 1);
		double bumpAmplitude = inst->bumpAmplitude;
		double spotSize = na->spotSize;

		LWDVector bump = {0, 0, 0}, oPosBackup, baseBump = {0, 0, 0}, goosBump = {0, 0, 0}, inputBump;

		if (isnan(spotSize))
		{
			spotSize = .0001f;
		}

		inputfuncs->evaluate(inst->inputBaseBump, na, baseBump);
		inputfuncs->evaluate(inst->inputGoosBump, na, goosBump);

		nodeutilfuncs->Blend(inputBump, goosBump, baseBump,  result, Blend_Normal);

		inputfuncs->evaluate(inst->inputBumpAmplitude, na, &bumpAmplitude);

		if (bumpAmplitude != 0)
		{
			VCPY(oPosBackup, na->oPos);

			na->oPos[0] += spotSize;
			bump[0] = localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[0] = oPosBackup[0];

			na->oPos[1] += spotSize;
			bump[1] = localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[1] = oPosBackup[1];

			na->oPos[2] += spotSize;
			bump[2] = localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[2] = oPosBackup[2];

			na->oPos[0] -= spotSize;
			bump[0] = -localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[0] = oPosBackup[0];

			na->oPos[1] -= spotSize;
			bump[1] = -localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[1] = oPosBackup[1];

			na->oPos[2] -= spotSize;
			bump[2] = -localEvaluate(inst, edgeList, na, threadIndex, 0);
			na->oPos[2] = oPosBackup[2];


			VSCL(bump, bumpAmplitude);
		}
		VADD(bump, inputBump);
		
		outputfuncs->setValue(Value, bump);
	}
	//if my edgelist has been dirtied while I was working, clean up what we can
	if (edgeList->isDirty)
	{
		if (edgeList->nodeDistance[threadIndex] != NULL)
		{
			free(edgeList->nodeDistance[threadIndex]);
			edgeList->nodeDistance[threadIndex] = NULL;
		}
	}	
}

/***************************************************************
*
*  The user interface get callback.
*
***************************************************************/
static void *AG_Get( void *voidInst, unsigned int vid )
{
	AnythingGoos_p inst = voidInst;

	/*  Return NULL to disable the control,
	if there is something connected to the corresponding input. */

	switch( vid )
	{
	case AG_noise:
		if( inputfuncs->check( inst->inputNoise ) )
			return NULL;
		return inst->paramNoise;

	case AG_baseColor:
		if( inputfuncs->check( inst->inputBaseColor ) )
			return NULL;
		return inst->paramBaseColor;

	case AG_goosColor:
		if( inputfuncs->check( inst->inputGoosColor ) )
			return NULL;
		return inst->paramGoosColor;

	case AG_opacity:
		if( inputfuncs->check( inst->inputOpacity ) )
			return NULL;
		return inst->paramOpacity;

	//case AG_falloffangle:
	//	if( inputfuncs->check( inst->inputFalloffAngle ) )
	//		return NULL;
	//	return inst->paramFalloffAngle;

	case AG_distance:
		if( inputfuncs->check( inst->inputDistance ) )
			return NULL;
		return inst->paramDistance;

	case AG_bumpamplitude:
		if( inputfuncs->check( inst->inputBumpAmplitude ) )
			return NULL;
		return inst->paramBumpAmplitude;

	case AG_baseSpecular:
		if( inputfuncs->check( inst->inputBaseSpecular ) )
			return NULL;
		return inst->paramBaseSpecular;

	case AG_goosSpecular:
		if( inputfuncs->check( inst->inputGoosSpecular ) )
			return NULL;
		return inst->paramGoosSpecular;

	case AG_baseGlossiness:
		if( inputfuncs->check( inst->inputBaseGlossiness ) )
			return NULL;
		return inst->paramBaseGlossiness;

	case AG_goosGlossiness:
		if( inputfuncs->check( inst->inputGoosGlossiness ) )
			return NULL;
		return inst->paramGoosGlossiness;

	case AG_baseLuminosity:
		if( inputfuncs->check( inst->inputBaseLuminosity ) )
			return NULL;
		return inst->paramBaseLuminosity;

	case AG_goosLuminosity:
		if( inputfuncs->check( inst->inputGoosLuminosity ) )
			return NULL;
		return inst->paramGoosLuminosity;

	case AG_baseReflection:
		if( inputfuncs->check( inst->inputBaseReflection ) )
			return NULL;
		return inst->paramBaseReflection;

	case AG_goosReflection:
		if( inputfuncs->check( inst->inputGoosReflection ) )
			return NULL;
		return inst->paramGoosReflection;

	case AG_baseTransparency:
		if( inputfuncs->check( inst->inputBaseTransparency ) )
			return NULL;
		return inst->paramBaseTransparency;

	case AG_goosTransparency:
		if( inputfuncs->check( inst->inputGoosTransparency ) )
			return NULL;
		return inst->paramGoosTransparency;

	case AG_baseRefractionIndex:
		if( inputfuncs->check( inst->inputBaseRefractionIndex ) )
			return NULL;
		return inst->paramBaseRefractionIndex;

	case AG_goosRefractionIndex:
		if( inputfuncs->check( inst->inputGoosRefractionIndex ) )
			return NULL;
		return inst->paramGoosRefractionIndex;

	case AG_baseTranslucency:
		if( inputfuncs->check( inst->inputBaseTranslucency ) )
			return NULL;
		return inst->paramBaseTranslucency;

	case AG_goosTranslucency:
		if( inputfuncs->check( inst->inputGoosTranslucency ) )
			return NULL;
		return inst->paramGoosTranslucency;

	case AG_baseDiffuse:
		if( inputfuncs->check( inst->inputBaseDiffuse ) )
			return NULL;
		return inst->paramBaseDiffuse;

	case AG_goosDiffuse:
		if( inputfuncs->check( inst->inputGoosDiffuse ) )
			return NULL;
		return inst->paramGoosDiffuse;
	case AG_perimeter:
		return &inst->perimeterEdges;

	case AG_inner:
		return &inst->innerEdges;

	case AG_outer:
		return &inst->outerEdges;

	case AG_invert:
		if( inputfuncs->check( inst->inputInvert ) )
			return NULL;
		return &inst->invert;

	case AG_edgeangle:
		return inst->paramEdgeAngle;

	case AG_falloff:
		return &inst->falloff;

	case AG_blend:
		if( inputfuncs->check( inst->inputBlend ) )
			return NULL;
		return &inst->blend;

	case AG_includeAlphaBlend:
		return &inst->includeAlphaBlend;

	case AG_innerpreview:
		{
			static char text[] = "Previews on X-axis.";

			return text;
		}

	case AG_outerpreview:
		{
			static char text[] = "Previews on Y-axis.";

			return text;
		}

	case AG_perimeterpreview:
		{
			static char text[] = "Previews on the Z-axis.";

			return text;
		}

	default:
		return NULL;
	}
}

/***************************************************************
*
*  The user interface set callback.
*
***************************************************************/
static LWXPRefreshCode AG_Set( void *voidInst, unsigned int vid, void *value )
{
	AnythingGoos_p inst = voidInst;

	switch( vid )
	{
	case AG_baseColor:
	case AG_goosColor:
	case AG_noise:
	case AG_opacity:
	case AG_bumpamplitude:
	case AG_baseSpecular:
	case AG_goosSpecular:
	case AG_baseGlossiness:
	case AG_goosGlossiness:
	case AG_baseLuminosity:
	case AG_goosLuminosity:
	case AG_baseReflection:
	case AG_goosReflection:
	case AG_baseTransparency:
	case AG_goosTransparency:
	case AG_baseRefractionIndex:
	case AG_goosRefractionIndex:
	case AG_baseTranslucency:
	case AG_goosTranslucency:
	case AG_baseDiffuse:
	case AG_goosDiffuse:
	case AG_distance:
	case AG_edgeangle:
	//case AG_falloffangle:
		break;

	case AG_perimeter:
		inst->perimeterEdges = *(int*)value;
		break;

	case AG_inner:
		inst->innerEdges = *(int*)value;
		break;

	case AG_outer:
		inst->outerEdges = *(int*)value;
		break;

	case AG_invert:
		inst->invert = *(int*)value;
		break;

	case AG_falloff:
		inst->falloff = *(int*)value;
		break;

	case AG_includeAlphaBlend:
		inst->includeAlphaBlend = *(int*)value;
		break;

	case AG_blend:
		inst->blend = *(int*)value;
		break;


	default:
		return LWXPRC_NONE;
	}
	return LWXPRC_DFLT;
}

/***************************************************************
*
*  The user interface change notification callback.
*
***************************************************************/
static void UIChangeNotify( LWXPanelID xpanel, unsigned int cid, unsigned int vid, int event )
{
	AnythingGoos_p	inst;

	if( !xpanel )
		return;

	inst = (AnythingGoos_p)xpanf->getData( xpanel, 0 );

	if( !inst )
		return;

	/*  If the user is tracking a control, we don't want to update the entire node flow.
	We just need to update this node, to make the previewer update faster. */
	if( event == LWXPEVENT_TRACK )
	{
		nodefuncs->UpdateNodePreview( inst->node );
		return;
	}
	// The value is set, so update the entire node flow.
	else if( event == LWXPEVENT_VALUE )
		lwupdate( LWNODE_HCLASS, inst );
}

/***************************************************************
*
*  The user interface control descriptions.
*
***************************************************************/
static LWXPanelControl AG_control[] = {
	{ AG_baseColor,		FIELD_NAME_BASE_COLOR,		"color-env"     },
	{ AG_goosColor,		FIELD_NAME_GOOS_COLOR,		"color-env"     },
	{ AG_blend,			FIELD_NAME_BLEND,			"iPopChoice"     },
	{ AG_opacity,		FIELD_NAME_OPACITY,			"percent-env"     },
	{ AG_invert,		FIELD_NAME_INVERT,			"iBoolean"    },
	{ AG_noise,			FIELD_NAME_NOISE,			"percent-env"     },
	{ AG_falloff,		FIELD_NAME_FALLOFF,			"iPopChoice"     },
	{ AG_distance,		FIELD_NAME_DISTANCE,		"distance-env"    },
	{ AG_edgeangle,		FIELD_NAME_EDGE_ANGLE,		"angle-env"    },
	//{ AG_falloffangle,		FIELD_NAME_FALLOFF_ANGLE,"angle-env"    },
	{ AG_includeAlphaBlend,		FIELD_NAME_INCLUDE_ALPHA_BLEND,			"iPopChoice"     },
	{ AG_bumpamplitude,	FIELD_NAME_BUMP_AMPLITUDE,	"percent-env"    },
	{ AG_innerpreview,	"",							"sInfo"    },
	{ AG_inner,			FIELD_NAME_INNER_EDGES,		"iBoolean"    },
	{ AG_outerpreview,	"",							"sInfo"    },
	{ AG_outer,			FIELD_NAME_OUTER_EDGES,		"iBoolean"    },
	{ AG_perimeterpreview,"",						"sInfo"    },
	{ AG_perimeter,		FIELD_NAME_PERIMETER_EDGES,  "iBoolean"    },
	{ AG_baseLuminosity,FIELD_NAME_BASE_LUMINOSITY,	"percent-env"    },
	{ AG_goosLuminosity,FIELD_NAME_GOOS_LUMINOSITY,	"percent-env"    },
	{ AG_baseDiffuse,FIELD_NAME_BASE_DIFFUSE,	"percent-env"    },
	{ AG_goosDiffuse,FIELD_NAME_GOOS_DIFFUSE,	"percent-env"    },
	{ AG_baseSpecular,	FIELD_NAME_BASE_SPECULAR,	"percent-env"    },
	{ AG_goosSpecular,	FIELD_NAME_GOOS_SPECULAR,	"percent-env"    },
	{ AG_baseGlossiness,FIELD_NAME_BASE_GLOSSINESS,	"percent-env"    },
	{ AG_goosGlossiness,FIELD_NAME_GOOS_GLOSSINESS,	"percent-env"    },
	{ AG_baseReflection,FIELD_NAME_BASE_REFLECTION,	"percent-env"    },
	{ AG_goosReflection,FIELD_NAME_GOOS_REFLECTION,	"percent-env"    },
	{ AG_baseTransparency,FIELD_NAME_BASE_TRANSPARENCY,	"percent-env"    },
	{ AG_goosTransparency,FIELD_NAME_GOOS_TRANSPARENCY,	"percent-env"    },
	{ AG_baseRefractionIndex,FIELD_NAME_BASE_REFRACTIONINDEX,	"float-env"    },
	{ AG_goosRefractionIndex,FIELD_NAME_GOOS_REFRACTIONINDEX,	"float-env"    },
	{ AG_baseTranslucency,FIELD_NAME_BASE_TRANSLUCENCY,	"percent-env"    },
	{ AG_goosTranslucency,FIELD_NAME_GOOS_TRANSLUCENCY,	"percent-env"    },
	{ 0 }
};

static LWXPanelDataDesc AG_data[] = {
	{ AG_baseColor,		FIELD_NAME_BASE_COLOR,		"color-env"   },
	{ AG_goosColor,		FIELD_NAME_GOOS_COLOR,		"color-env"   },
	{ AG_blend,			FIELD_NAME_BLEND,			"integer"     },
	{ AG_opacity,		FIELD_NAME_OPACITY,			"float-env"     },
	{ AG_invert,		FIELD_NAME_INVERT,			"integer"    },
	{ AG_noise,			FIELD_NAME_NOISE,			"float-env"   },
	{ AG_falloff,		FIELD_NAME_FALLOFF,			"integer"     },
	{ AG_distance,      FIELD_NAME_DISTANCE,		"distance-env"   },
	{ AG_edgeangle,		FIELD_NAME_EDGE_ANGLE,		"angle-env"    },
	//{ AG_falloffangle,		FIELD_NAME_FALLOFF_ANGLE,"angle-env"    },
	{ AG_includeAlphaBlend,		FIELD_NAME_INCLUDE_ALPHA_BLEND,			"integer"     },
	{ AG_bumpamplitude,	FIELD_NAME_BUMP_AMPLITUDE,	"float-env"    },
	{ AG_innerpreview,	"",							"string"    },
	{ AG_inner,			FIELD_NAME_INNER_EDGES,		"integer"    },
	{ AG_outerpreview,	"",							"string"    },
	{ AG_outer,			FIELD_NAME_OUTER_EDGES,		"integer"    },
	{ AG_perimeterpreview,"",						"string"    },
	{ AG_perimeter,		FIELD_NAME_PERIMETER_EDGES,	"integer"   },
	{ AG_baseLuminosity,FIELD_NAME_BASE_LUMINOSITY,	"float-env"    },
	{ AG_goosLuminosity,FIELD_NAME_GOOS_LUMINOSITY,	"float-env"    },
	{ AG_baseDiffuse,FIELD_NAME_BASE_DIFFUSE,	"float-env"    },
	{ AG_goosDiffuse,FIELD_NAME_GOOS_DIFFUSE,	"float-env"    },
	{ AG_baseSpecular,	FIELD_NAME_BASE_SPECULAR,	"float-env"    },
	{ AG_goosSpecular,	FIELD_NAME_GOOS_SPECULAR,	"float-env"    },
	{ AG_baseGlossiness,FIELD_NAME_BASE_GLOSSINESS,	"float-env"    },
	{ AG_goosGlossiness,FIELD_NAME_GOOS_GLOSSINESS,	"float-env"    },
	{ AG_baseReflection,FIELD_NAME_BASE_REFLECTION,	"float-env"    },
	{ AG_goosReflection,FIELD_NAME_GOOS_REFLECTION,	"float-env"    },
	{ AG_baseTransparency,FIELD_NAME_BASE_TRANSPARENCY,	"float-env"    },
	{ AG_goosTransparency,FIELD_NAME_GOOS_TRANSPARENCY,	"float-env"    },
	{ AG_baseRefractionIndex,FIELD_NAME_BASE_REFRACTIONINDEX,	"float"    },
	{ AG_goosRefractionIndex,FIELD_NAME_GOOS_REFRACTIONINDEX,	"float"    },
	{ AG_baseTranslucency,FIELD_NAME_BASE_TRANSLUCENCY,	"float-env"    },
	{ AG_goosTranslucency,FIELD_NAME_GOOS_TRANSLUCENCY,	"float-env"    },
	{ 0 }
};

/***************************************************************
*
*  The user interface panel callback.
*
***************************************************************/
XCALL_( static LWXPanelID )
AG_Panel( AnythingGoos_p inst )
{
	if( !inst )
		return NULL;

	if( !inst->panel )
	{
		// We will use the name of the node as the the title for the panel.
		const char	*nname = nodefuncs->nodeName( inst->node );
		LWXPanelHint AG_notify_hint[] = {
			XpCHGNOTIFY( UIChangeNotify ),
			XpLABEL( 0, nname ),
			XpDIVADD( AG_invert ),
			XpDIVADD( AG_bumpamplitude ),
			XpDIVADD( AG_goosDiffuse ),
			XpDIVADD( AG_goosReflection ),
			XpDIVADD( AG_goosTranslucency ),
			XpDIVADD( AG_perimeter ),
			XpSTRLIST(AG_falloff, FallOffSelections),
			XpSTRLIST(AG_includeAlphaBlend, AlphaBlendSelections),
			XpSTRLIST(AG_blend, BlendSelections),
			XpMIN(AG_baseRefractionIndex, 1),
			XpMAX(AG_baseRefractionIndex, 3),
			XpMIN(AG_goosRefractionIndex, 1),
			XpMAX(AG_goosRefractionIndex, 3),
			XpEND
		};

		if( NULL != ( inst->panel = xpanf->create( LWXP_VIEW, AG_control ) ) )
		{
			xpanf->describe( inst->panel, AG_data, AG_Get, AG_Set );
			xpanf->hint( inst->panel, 0, AG_notify_hint );
			xpanf->viewInst( inst->panel, inst );
			xpanf->setData( inst->panel, 0, inst );
		}
		else
			return NULL;
	}
	return inst->panel;
}

/***************************************************************
*
*  Handler activation function.
*
***************************************************************/
XCALL_( static int )
Handler( int version, GlobalFunc *global, LWNodeHandler *local,
		void *serverData)
{
	if( version != LWNODECLASS_VERSION )
		return AFUNC_BADVERSION;

	inputfuncs  = global( LWNODEINPUTFUNCS_GLOBAL,  GFUSE_TRANSIENT );
	outputfuncs = global( LWNODEOUTPUTFUNCS_GLOBAL, GFUSE_TRANSIENT );
	nodefuncs   = global( LWNODEFUNCS_GLOBAL,       GFUSE_TRANSIENT );
	nodeutilfuncs=global( LWNODEUTILITYFUNCS_GLOBAL,GFUSE_TRANSIENT ); 
	iteminfo    = global( LWITEMINFO_GLOBAL,        GFUSE_TRANSIENT );
	objinfo     = global( LWOBJECTINFO_GLOBAL,      GFUSE_TRANSIENT );
	mtutil		= global( LWMTUTILFUNCS_GLOBAL,		GFUSE_TRANSIENT );
	envfuncs	= global( LWENVELOPEFUNCS_GLOBAL,	GFUSE_TRANSIENT );
	vparmfuncs  = global( LWVPARMFUNCS_GLOBAL,		GFUSE_TRANSIENT);
	
	if( !inputfuncs || !outputfuncs || !nodefuncs || !nodeutilfuncs || !envfuncs || !vparmfuncs)
		return AFUNC_BADGLOBAL;

	local->inst->priv       = global;
	local->inst->create     = Create;
	local->inst->destroy    = Destroy;
	local->inst->load       = Load;
	local->inst->save       = Save;
	local->inst->copy       = Copy;
	local->rend->init       = Init;
	local->rend->cleanup    = Cleanup;
	local->rend->newTime    = NewTime;
	local->evaluate         = Evaluate;


	return AFUNC_OK;
}

/***************************************************************
*
*  Interface activation function.
*
***************************************************************/
XCALL_(int)
Interface( int version, GlobalFunc *global, LWInterface *local, void *serverdata )
{
	if( local == (LWInterface*)NULL )
		return AFUNC_BADLOCAL;

	lwupdate  = global( LWINSTUPDATE_GLOBAL,        GFUSE_TRANSIENT );
	xpanf     = global( LWXPANELFUNCS_GLOBAL,       GFUSE_TRANSIENT );

	if( !lwupdate || !xpanf )
		return AFUNC_BADGLOBAL;

	local->panel    = AG_Panel( local->inst );
	local->options  = NULL;
	local->command  = NULL;

	return AFUNC_OK;
}
/*****************************************************************************
 *  
 *  Server tag.
 *
 *****************************************************************************/
static ServerTagInfo AnythingGoos_tag[] = { 
{ "Digital Carvers Guild",SRVTAG_NODEGROUP },
{ "", 0 }
};


/***************************************************************
*
*  The server record
*
***************************************************************/
ServerRecord ServerDesc[] = {
	{ LWNODE_HCLASS, "Anything Goos", (ActivateFunc*)Handler, AnythingGoos_tag},
	{ LWNODE_ICLASS, "Anything Goos", (ActivateFunc*)Interface, AnythingGoos_tag },
	{ NULL }
};

void main()
{
	
	
}
