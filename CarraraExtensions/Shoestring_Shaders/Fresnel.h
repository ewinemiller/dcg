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

#ifndef __Fresnel__
#define __Fresnel__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "FresnelDef.h"
#include "BasicShader.h"
//Alan
#include "ComMessages.h"

#include "MCColorRGBA.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "MCCountedPtrHelper.h"
#include "COMUtilities.h"
#include "BasicModifiers.h"
#include "I3DShRenderFeature.h"
#include "I3DShRenderable.h"
#include "I3DShTreeElement.h"
#include "I3DShScene.h"
//#include "BasicCOMImplementations.h"
#include "copyright.h"
#define fuzzydelta    	0.000001
#define floataccuracy 	0.000001
#define nsdel 			0.000001
#define lsdel 			0.000001
#define de    			0.000001
#define undefined -1
#define UnKnownf			4096.
#define simple false

#define REAL(z)     ((z).dat[0])
#define IMAG(z)     ((z).dat[1])

#define SET_COMPLEX(zp,x,y) do {(zp)->dat[0]=(x); (zp)->dat[1]=(y);} while(0)
#define SET_REAL(zp,x) do {(zp)->dat[0]=(x);} while(0)
#define SET_IMAG(zp,y) do {(zp)->dat[1]=(y);} while(0)

#define CoeffSize  1024
#define CoeffScale 1023.0  //Needs to be set to one less than CoeffSize

#define nD      	589.3   //Sodium D line, nm.
#define nF      	486.1   //Hydrogen F, nm.
#define nC      	656.3   //Hydrogen C, nm.
#define Wmin    	409.3   //min Spectral wavelength of Wavelength slider.
#define Wmax    	709.3   //max Spectral wavelength of Wavelength slider.
#define CIEmin  	380.0   //min Spectral wavelength of CIE data.
#define CIEmax  	825.0   //max Spectral wavelength of CIE data.
#define step    	0.1     //Integration step.
#define NCSt 0.0 	//No Correction start. Do correction in high luma colors.
#define BCSt 30.0 	//Blend Start
#define CSt  45.0 	//Corection start 
#define CEnd 100.0 	//End

#define aCorrection			    2.0*(-2.49)
#define bCorrection             2.0*(-19.42)

#define Num_Medium_Presets 31
#define Water 			0
#define Air 			1
#define Glass 			2
#define Diamond			3
#define Lead_Crystal	4
#define Quartz			5
#define Plastic			6
#define Silver			7
#define Gold			8
#define Aluminium		9
#define Copper			10
#define Bronze			11
#define Iron			12
#define Ferrite			13
#define Alcohol			14
#define Amber			15
#define Amethyst		16	
#define Emerald			17
#define Ice				18
#define Ruby			19
#define Salt			20
#define Topaz			21
#define Mylar			22
#define Nickel			23
#define Platinum		24
#define Titanium		25
#define Plexiglass		26
#define Obsidian		27
#define SeaWater		28
#define Freshwater		29
#define Dirtywater		30
#define Other_Value     31

	struct Medium_Preset {
		real32 	nr;
		real32 	ni;
		real32 	mur;
		real32 	disp;};
//Source www.luxpop.com for nr , ni 
//disp: Get the real value for refraction for the material at wavelengths nd=589.3 nf=486.1 and nc=656.3 
//at the temperature you think appropriate (I use 25 celcius)
//then disp=(nf-nc)/(nd-1) , this can be negative for some materials.
//dispersion is the refractive index at blue minus that at red all divided by that at yellow.



#define Num_Illumination_Presets 15
#define D30 1
#define D35 2
#define D40 3
#define D45 4
#define D50 5
#define D55 6
#define D60 7
#define D65 8
#define D70 9
#define D75 10
#define D80 11
#define D85 12
#define D90 13
#define D93 14
#define Other_Illuminant 15


#define KRB 1
#define KBG 2
#define GCY 3
#define KGR 4
#define RYM 5
#define BMC 6

#define epsilon 0.01 //For comparisons.
#define delta   1 //Precision for Solutions to Line/Surface Intersection search. 
#define UnKnown 1.00000 

#define Less 			0
#define LessEqual 		1
#define Greater 		2
#define GreaterEqual 	3
#define EqualTo 		4
#define UnequalTo		5

/* White point chromaticities. x,y,Y */

#define IlluminantC     0.3101,       0.3162	   ,1.0   	 ,0.0000,       0.0000	   ,0.0			/* For NTSC television */
#define IlluminantD65   0.312713,     0.329016	   ,1.0   	 ,0.0000,       0.0000	   ,0.0			/* 6500K */
#define IlluminantD50   0.345741,     0.358666	   ,1.0   	 ,0.0000,       0.0000	   ,0.0			/* 5000K */
#define IlluminantE 	0.33333333,   0.358562     ,1.0      ,0.0000,       0.0000	   ,0.0			/* CIE equal-energy illuminant */
#define MyWhitePoint    0.316000,     0.328000     ,1.0      ,0.0000,   	0.000	   ,0.0000		/* MyMonitor d65 */
#define IlluminantD93   0.2848,       0.2932       ,1.0      ,0.0000,       0.0000	   ,0.0			/* MyMonitor. -> D9300 */
#define Eovia           0.312636,     0.328931     ,1.0      ,0.0000,       0.0000	   ,0.0			/* Eovia Whitepoint */


#define GAMMA_REC709	0		/* Rec. 709 */
#define sRGB	        1		/* sRGB     */

#define V000 "000"
#define V001 "001"
#define V010 "010"
#define V011 "011"
#define V100 "100"
#define V101 "101"
#define V110 "110"
#define V111 "111"

struct complex    {
  real dat[2];
};

struct CubicRoots {
 complex Root1;
 complex Root2;
 complex Root3;
 boolean Root1Valid;
 boolean Root2Valid;
 boolean Root3Valid;
};
 


struct  Coeff  //Coefficient array.
  {
    real32 dat[CoeffSize];
  };
 

struct colourSystem {
    char   *name;     	    	    /* Colour system name */
    real32  xRed,   yRed,   YRed,	/* Red x, y , Y*/
           xGreen, yGreen, YGreen, 	/* Green x, y ,Y*/
           xBlue,  yBlue,  YBlue,  	/* Blue x, y ,Y*/
           xWhite, yWhite, YWhite,  /* White point x, y ,Y*/
           xBlack, yBlack, YBlack,  /* Black point x, y ,Y*/
	       gamma;   	    	    /* Gamma correction for system */
};


struct LabResidue {
real32 L,a, b;real32 t,u,v; int32 Plane;
};

struct LabParameter {
real32 t,u, v;int32 Plane;
real32 L,a,b;
};

struct LabVolume {
real32 tmin,tmax,umin, umax, vmin, vmax;
};

struct XYZResidue {
real32 X,Y, Z; real32 t,u,v,Plane;
};

struct XYZParameter {
real32 t,u, v;
int32 Plane;
};

struct ssRGBColor {
real32 R,G,B;
};


struct ParameterVolume {
					boolean Intersects;
					real32 		tmin;
					real32 		tmax;
					real32 		umin;
					real32 		umax;
					real32 		vmin;
					real32 		vmax;
					real32		tLtmin;
					real32		tLtmax;
					real32		tatmin;
					real32		tatmax;
					real32		tbtmin;
					real32		tbtmax;
					real32 		UVLuminvmin;
					real32 		UVauminvmin;
					real32 		UVbuminvmin;
					real32 		UVLuminvmax;
					real32 		UVauminvmax;
					real32 		UVbuminvmax;
					real32 		UVLumaxvmin;
					real32 		UVaumaxvmin;
					real32 		UVbumaxvmin;
					real32 		UVLumaxvmax;
					real32 		UVaumaxvmax;
					real32 		UVbumaxvmax;
					boolean     t;
					boolean     u;
					boolean     v;
					pointer	 	Index;
					int32 		Order;
					int32 		Plane;
};


struct PointParam {
real32 x,y,z,t,u,v;
};

struct ParameterVolumePartition {
ParameterVolume v000,v001,v010,v011,v100,v101,v110,v111;
};

struct XYZPoint {
real32 X,Y, Z;
};

struct XYZLine {
real32 c1X,c1Y,c1Z;
real32 c2X,c2Y,c2Z; 
};

struct XYZReferenceWhite {
real32 Xr,Yr,Zr;
};

struct XYZSpectralColor {
real32 SX,SY,SZ;
};

struct XYZGrayColor {
real32 GX,GY,GZ;
};

struct XYZPlane {
real32 c1X,c1Y,c1Z;
real32 c2X,c2Y,c2Z; 
real32 c3X,c3Y,c3Z; 
};

struct LabPoint {
real32 L,a, b;
};

struct LabLine { //First point defined by XYZ coordinates.
real32 c1X,c1Y,c1Z;
real32 c2L,c2a,c2b; 
};

struct XYZIntersection {
LabLine TLine;
XYZPlane UVPlane;
XYZReferenceWhite RefWhite;
};

struct LabPlane { //Defined by XYZ coordinates.
real32 c1X,c1Y,c1Z;
real32 c2X,c2Y,c2Z; 
real32 c3X,c3Y,c3Z; 
};

struct LabIntersection {
LabLine TLine;
LabPlane UVPlane;
XYZReferenceWhite RefWhite;
};

struct LabSpectralColor {
real32 SL,Sa,Sb;
};

struct LabGrayColor {
real32 GL,Ga,Gb;
};


struct LabColor {
real32 L,a,b;
};

struct XYZColor {
real32 X,Y,Z;
};

struct xyYColor {
real32 x;
real32 y;
real32 Y;
};

//Didn't use thes in the end as precedence issues made one need as many brackets as the function versions.
inline boolean operator== ( complex a , complex b ) //(a + b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  boolean r=false;
  if( ar==br && ai==bi ) { r=true;};
  return r;
}
inline boolean operator!= ( complex a , complex b ) //(a + b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  boolean r=true;
  if( ar==br && ai==bi ) { r=false;};
  return r;
}

inline complex operator+ ( complex a , complex b ) //(a + b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar + br, ai + bi);
  return z;
}
inline complex operator+ ( real32 a , complex b ) //(a + b), ,b complex, a real.
{ real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, a+br, bi);
  return z;
}
inline complex operator+ ( complex a , real32 b ) //(a + b), a complex, b real.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex z;
  SET_COMPLEX (&z, ar+b, ai);
  return z;
}

inline complex operator- ( complex a , complex b ) //(a - b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar - br, ai - bi);
  return z;
}
inline complex operator- ( real32 a , complex b ) //(a - b), b complex, a real.
{ real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, a-br, bi);
  return z;
}
inline complex operator- ( complex a , real32 b ) //(a - b), a complex, b real.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex z;
  SET_COMPLEX (&z, ar-b, ai);
  return z;
}
inline complex operator* ( int a , complex b ) //(a * b), a int32,b complex.
{ real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, a * br, a * bi);
  return z;
}
inline complex operator* ( complex a , int b ) //(a * b), a int32,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex z;
  SET_COMPLEX (&z, ar * b, ai * b);
  return z;
}

inline complex operator* ( complex a , complex b ) //(a * b), a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, ar * br - ai * bi, ar * bi + ai * br);
  return z;
}
inline complex operator* ( real32 a , complex b ) //(a * b), b complex a real.
{ real32 br = REAL (b), bi = IMAG (b);
  complex z;
  SET_COMPLEX (&z, a * br, a * bi);
  return z;
}
inline complex operator* ( complex a , real32 b ) //(a * b), a complex, b real.
{ real32 ar = REAL (a), ai = IMAG (a);
  complex z;
  SET_COMPLEX (&z, ar * b, ai * b );
  return z;
}

inline complex operator/ ( complex a, complex b)  //   (a/b),   a,b complex.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 br = REAL (b), bi = IMAG (b);
  real32 d = 1.0/(br*br+bi*bi);
  real32 znr = (ar * br + ai * bi) * d;
  real32 zni = (ai * br - ar * bi) * d;
  complex z;
  SET_COMPLEX (&z, znr, zni);
  return z;
}
inline complex operator/ ( real32 a, complex b)  //   (a/b),   b complex, a real.
{ real32 br = REAL (b), bi = IMAG (b);
  real32 d = 1.0/(br*br+bi*bi);
  real32 znr =  a*br;
  real32 zni = -a*bi;
  complex z;
  SET_COMPLEX (&z, znr*d, zni*d);
  return z;
}
inline complex operator/ ( complex a, real32 b)  //   (a/b),   a complex, b real.
{ real32 ar = REAL (a), ai = IMAG (a);
  real32 d = 1.0/b;
  real32 znr = ar;
  real32 zni = ai;
  complex z;
  SET_COMPLEX (&z, znr*d, zni*d);
  return z;
}

inline complex operator- ( complex a )            //    (-a),   a complex.
{ complex z;
  SET_COMPLEX (&z, -REAL (a), -IMAG (a));
  return z;
}
inline complex operator+ ( complex a )            //    (-a),   a complex.
{ complex z;
  SET_COMPLEX (&z, REAL (a), IMAG (a));
  return z;
}


// Define the FresnelShader CLSID ( see the FresnelDef.h file to get R_CLSID_Fresnel value )

extern const MCGUID CLSID_Fresnel;

struct FresnelShaderPublicData
{
	TMCString255 Object_Name;
	int32		Mode;				// smooth mix or shader influenced
	TMCCountedPtr<IShParameterComponent> fMixShade;	
    real32 		n1r;
    real32 		n1i;
    real32 		n2r;
    real32 		n2i;
    real32 		mu1r;
    real32 		mu2r;
    real32 		Pper;
    int32 		TRMode;
    real32 		n2rslider;
    real32 		n2islider;
    real32 		mu2rslider;
    real32 		lambdaslider;
    real32 		disp;
    real32 		n2rd;
    real32 		n2id;
	int32 		Preset_n1;
	int32 		Preset_n2;
	int32 		Preset_illum;
	boolean  Color_Object;
	TMCColorRGBA Spectral_Color;
    real32 		Spectral_Red;
    real32 		Spectral_Green;
    real32 		Spectral_Blue;
	TMCColorRGBA illuminant_Color;
	real32        illuminant;
	real32        xWhite;
	real32        yWhite;

};

struct FresnelShaderShadowData
{
	TMCString255 Object_Name;
	int32		Mode;				// smooth mix or shader influenced
	TMCCountedPtr<IShParameterComponent> fMixShade;	
    real32 		n1r;
    real32 		n1i;
    real32 		n2r;
    real32 		n2i;
    real32 		mu1r;
    real32 		mu2r;
    real32 		Pper;
    int32 		TRMode;
    real32 		n2rslider;
    real32 		n2islider;
    real32 		mu2rslider;
    real32 		lambdaslider;
    real32 		disp;
    real32 		n2rd;
    real32 		n2id;
	int32 		Preset_n1;
	int32 		Preset_n2;
	int32 		Preset_illum;
	boolean  		Color_Object;
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Spectral_Color;
	TMCColorRGBA illuminant_Color;
#elif (VERSIONNUMBER < 0x040000)
	TMCColorRGB Spectral_Color;
	TMCColorRGB illuminant_Color;
#endif
	real32        illuminant;
	real32        xWhite;
	real32        yWhite;

};
struct FresnelShaderPrivateData
{
	real32 		ref_len;
	real32 		angle;
	real32 		weight[7];
	real32 		sine_z;
	TVector3 	ref_vec;
	real32 		rot_matrix[3][3];
	real32 		cos_rot;
	real32 		sin_rot;
	uint32		Imp_Out;
	real32 		luma;
	real32 		last_luma;
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA Color;
#elif (VERSIONNUMBER < 0x040000)
	TMCColorRGB Color;
#endif
	Coeff 		Rparc;
    Coeff 		Rperc;
    Coeff 		Tparc;
    Coeff 		Tperc;
    real32 		n1r_internal;
    real32 		n1i_internal;
    real32 		n2r_internal;
    real32 		n2i_internal;
    real32 		mu1r_internal;
    real32 		mu2r_internal;
    real32 		Ppar;
	real32 		min_RGB;              //Minimum value for rgb component in a Spectrum. 
	real32 		max_RGB;              //Maximum value for rgb component in a Spectrum.
	int32 		partitioncount;

	real32 illum_Preset_data[Num_Illumination_Presets] ;


	colourSystem Source_ColorSystem;
	colourSystem Destination_ColorSystem;
	colourSystem Monitor_ColorSystem;
	colourSystem CIE_ColorSystem;
	colourSystem Color_System;
	colourSystem Adobe_ColorSystem;
	colourSystem Eovia_ColorSystem;
		
	boolean 	 BlackBody;
	real32    GLf;
    boolean 	 grayscale;
	boolean 	 first_pass;
	boolean 	 SolutionFound;
	boolean	 SolutionAbort; 
	ssRGBColor SpectralRGB;
	LabColor SpectralLab;
	LabColor WhitePoint;
	ssRGBColor WhitePointRGB;

	colourSystem    AdobeRGB   ;
	colourSystem    NTSCsystem ;
	colourSystem    SMPTEsystem;
	colourSystem    CIEsystem  ;
	colourSystem    MyMonitor  ;
	colourSystem    AppleRGB   ;
	colourSystem    Source     ;
	colourSystem    Destination;
	boolean disableKGR;
	};

class Fresnel : public TBasicShader
{
public :
	FresnelShaderPublicData	FresnelPublicData;

	Fresnel();

	STANDARD_RELEASE;

	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(FresnelShaderPublicData);}

#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x030000)
	virtual real 		MCCOMAPI    GetColor		    (TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x040000)
	virtual real 		MCCOMAPI    GetColor		    (TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual boolean		MCCOMAPI    WantsTransform	();  

#endif

	virtual MCCOMErr		MCCOMAPI	HandleEvent(MessageID message, IMFResponder* source, void* data);

protected:

	TMCCountedPtr<I3DShShader> fMixShader;

	enum
	{
		kSmooth_mode	= IDTYPE('S', 'M', 'M', 'X'), //IDs from .rsr
		kShader_mode	= IDTYPE('S', 'H', 'M', 'X')
	};

	enum
	{
		kUseTransparency = IDTYPE('T', 'S', 'e', 'l'),
		kUseReflection   = IDTYPE('R', 'S', 'e', 'l')
	};

private :
	FresnelShaderShadowData 	FresnelShadowData;
	FresnelShaderPrivateData	FresnelPrivateData;

#if (VERSIONNUMBER <= 0x030000)
	TMCColorRGB hold;
#elif (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA hold;
#endif

	real32 		Re ( complex x );
    real32 		Im ( complex x );
    real32 		ndispersion ( real32 lambda , real32 nt, real32 dispnt );
	complex 	cadd ( complex x , complex y );
	complex 	csub ( complex x , complex y );
	complex 	cmul ( complex x , complex y );
	complex 	cneg ( complex x );
	complex 	sinc ( complex x );
	complex 	cosc ( complex x );
	complex 	arcsinc ( complex x );
	complex 	csqr ( complex x );
	complex 	cdiv ( complex x, complex y);
	complex 	cabs ( complex x );
	complex 	cabsq ( complex x );
	complex 	cosfc ( complex n, complex d, complex a);
	complex 	n1 ( complex thetai );
	complex 	n2 ( complex thetai );
	complex 	mu1 ( complex thetai );
	complex 	mu2 ( complex thetai );
	complex 	theta ( complex thetai );
	complex 	thetat ( complex thetai );
	complex 	Eperr ( complex thetai );
	complex 	Eperi ( complex thetai );
	complex 	Epert ( complex thetai );
	complex 	Eparr ( complex thetai );
	complex 	Epari ( complex thetai );
	complex 	Epart ( complex thetai );
	complex 	rper ( complex thetai );
	complex 	rpar ( complex thetai );
	complex 	tper ( complex thetai );
	complex 	tpar ( complex thetai );
	complex 	TArea ( complex thetai );
	complex 	Rper ( complex thetai );
	complex 	Tper ( complex thetai );
	complex 	Rpar ( complex thetai );
	complex 	Tpar ( complex thetai );
	inline real32 R ( complex thetai );
	inline real32 T ( complex thetai );
	inline real32 Fresnel_filter ( real32 angle);
    void 		Chromatic_Adaption_xyY_XYZ (struct colourSystem WScs,struct colourSystem WDcs,real32 xS, real32 yS, real32 YS, real32 *XD, real32 *YD,real32 *ZD);
    void        xyY_to_RGB (struct colourSystem WScs,struct colourSystem WDcs,real32 x, real32 y, real32 Y, real32 *R, real32 *G,real32 *B);
	XYZColor 	RGB_to_XYZ (struct colourSystem WScs,struct colourSystem WDcs, real32 R, real32 G,real32 B);
    real32       spectral_spd (real32 lambda,  real32 temperature);
    void        lambda_to_xyzXYZ (real32 *x, real32 *y, real32 *z, real32 *X, real32 *Y,real32 *Z,real32 lambda,real32 temperature);
    int32         constrain_rgb (real32 *r, real32 *g, real32 *b);
	void        norm_rgb (real32 *r, real32 *g, real32 *b);
    void 		gamma_correct (struct colourSystem cs, real32 *c);
    void        gamma_correct_rgb (struct colourSystem cs, real32 *r, real32 *g, real32 *b);
    colourSystem 	whitepoint (struct colourSystem cs, real32 temperature);
	void        	Calc_Spectral_Color (struct colourSystem WScs,struct colourSystem WDcs, real32 *R, real32 *G,real32 *B, real32 *L, real32 *a, real32 *b, real32 lambda, real32 temperature, real32 GLf,int32 Subdivides);
	int32         	XYZtoCorColorTemp(real32 X,real32 Y, real32 Z, real32 *temp);
	LabParameter 	SolveDumbXYZ ( ParameterVolume V , LabIntersection R ,real32 GLf,int32 Subdivides);
	ParameterVolumePartition 	PartitionVolume (ParameterVolume V,real32 Subdivides);
	ParameterVolume 			ContainsIntersection (ParameterVolume V, LabIntersection tuvObject);	
	LabColor 					XYZ2Lab ( XYZColor XYZ , XYZReferenceWhite Ref );
	XYZColor 					Lab2XYZ ( LabColor Lab , XYZReferenceWhite Ref );
	LabParameter 				Lab_Gamut_Project (struct colourSystem WDcs, XYZSpectralColor Color ,real32 GLf,real32 Ga,real32 Gb,int32 Subdivides);
	XYZColor 					xyY2XYZ ( xyYColor Color );
	inline real32 				fxXYZ ( real32 x );
	inline real32 				fyXYZ ( real32 x );
	inline real32 				fzXYZ ( real32 x );
	inline complex 				SetComplex ( real32 a , real32 b);
	CubicRoots 					SolveCubic ( real32 ar,real32 ai,real32 br,real32 bi,real32 cr,real32 ci,real32 dr,real32 di);
	PointParam 					LinePlaneIntersection (XYZPoint xx1,XYZPoint xx2,XYZPoint xx3,XYZPoint xx4,XYZPoint xx5);
	int32 round(real32 input);
	float x1,y1,x2,y2;
};                 
#endif // __Fresnel__
